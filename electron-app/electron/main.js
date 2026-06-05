const { app, BrowserWindow, ipcMain, Menu } = require("electron");
const path = require("node:path");
const fs = require("node:fs/promises");
const os = require("node:os");
const { spawn } = require("node:child_process");

const isDev = !app.isPackaged;

let currentAnalysisProcess = null;
let currentStopRequested = false;

function createWindow() {
  Menu.setApplicationMenu(null);

  const window = new BrowserWindow({
    width: 1240,
    height: 820,
    minWidth: 980,
    minHeight: 680,
    title: "RTSP Stream Inspector",
    webPreferences: {
      preload: path.join(__dirname, "preload.js"),
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: false
    }
  });

  if (isDev) {
    window.loadURL("http://127.0.0.1:5173");
  } else {
    window.loadFile(path.join(__dirname, "..", "dist", "index.html"));
  }
}

function defaultBinaryPath() {
  const binaryName = process.platform === "win32" ? "rtsp-inspector.exe" : "rtsp-inspector";
  return path.resolve(__dirname, "..", "..", "build", binaryName);
}

function resolveBinaryPath(binaryPath) {
  if (!binaryPath || String(binaryPath).trim() === "") {
    return defaultBinaryPath();
  }

  const trimmed = String(binaryPath).trim();
  if (path.isAbsolute(trimmed)) {
    return trimmed;
  }

  return path.resolve(__dirname, "..", trimmed);
}

function sanitizeOptions(rawOptions) {
  return {
    rtspUrl: String(rawOptions?.rtspUrl ?? ""),
    timeoutMs: Number.parseInt(rawOptions?.timeoutMs ?? 5000, 10),
    frames: Number.parseInt(rawOptions?.frames ?? 1000, 10),
    packetLogLimit: Number.parseInt(rawOptions?.packetLogLimit ?? 0, 10),
    generateMarkdown: rawOptions?.generateMarkdown !== false,
    binaryPath: String(rawOptions?.binaryPath ?? "")
  };
}

function validateOptions(options) {
  if (!options.rtspUrl.trim()) {
    throw new Error("RTSP URL is required.");
  }

  if (!Number.isFinite(options.timeoutMs) || options.timeoutMs <= 0) {
    throw new Error("Timeout must be a positive number.");
  }

  if (!Number.isFinite(options.frames) || options.frames <= 0) {
    throw new Error("Frames must be a positive number.");
  }

  if (!Number.isFinite(options.packetLogLimit) || options.packetLogLimit < 0) {
    throw new Error("Packet log limit must be zero or a positive number.");
  }
}

function sendLog(event, message) {
  event.sender.send("analysis-log", String(message));
}

ipcMain.handle("run-analysis", async (event, rawOptions) => {
  if (currentAnalysisProcess) {
    throw new Error("An analysis is already running.");
  }

  const options = sanitizeOptions(rawOptions);
  validateOptions(options);

  const binary = resolveBinaryPath(options.binaryPath);
  const reportDir = await fs.mkdtemp(path.join(os.tmpdir(), "rtsp-inspector-"));
  const jsonPath = path.join(reportDir, "report.json");
  const markdownPath = path.join(reportDir, "report.md");

  const args = [
    "analyze",
    "--url", options.rtspUrl,
    "--timeout-ms", String(options.timeoutMs),
    "--frames", String(options.frames),
    "--packet-log-limit", String(options.packetLogLimit),
    "--output", jsonPath
  ];

  if (options.generateMarkdown) {
    args.push("--markdown", markdownPath);
  }

  currentStopRequested = false;

  sendLog(event, `Using analyzer: ${binary}\n`);
  sendLog(event, `Report directory: ${reportDir}\n`);
  sendLog(event, "Starting analysis...\n");

  return await new Promise((resolve, reject) => {
    const child = spawn(binary, args, {
      cwd: path.resolve(__dirname, "..", ".."),
      windowsHide: true
    });

    currentAnalysisProcess = child;

    child.stdout.on("data", (chunk) => {
      sendLog(event, chunk.toString());
    });

    child.stderr.on("data", (chunk) => {
      sendLog(event, chunk.toString());
    });

    child.on("error", (error) => {
      currentAnalysisProcess = null;
      reject(new Error(`Failed to start rtsp-inspector: ${error.message}`));
    });

    child.on("close", async (code) => {
      currentAnalysisProcess = null;

      if (currentStopRequested) {
        currentStopRequested = false;
        reject(new Error("Analysis stopped by user."));
        return;
      }

      if (code !== 0) {
        reject(new Error(`rtsp-inspector exited with code ${code}`));
        return;
      }

      try {
        const reportText = await fs.readFile(jsonPath, "utf8");
        const report = JSON.parse(reportText);

        resolve({
          report,
          paths: {
            json: jsonPath,
            markdown: options.generateMarkdown ? markdownPath : null,
            directory: reportDir
          }
        });
      } catch (error) {
        reject(new Error(`Analysis completed, but report.json could not be read: ${error.message}`));
      }
    });
  });
});

ipcMain.handle("stop-analysis", async () => {
  if (!currentAnalysisProcess) {
    return { stopped: false };
  }

  currentStopRequested = true;
  currentAnalysisProcess.kill("SIGTERM");

  return { stopped: true };
});

app.whenReady().then(() => {
  createWindow();

  app.on("activate", () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on("window-all-closed", () => {
  if (process.platform !== "darwin") {
    app.quit();
  }
});
