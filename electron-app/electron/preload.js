const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("rtspInspector", {
  runAnalysis: (options) => ipcRenderer.invoke("run-analysis", options),
  stopAnalysis: () => ipcRenderer.invoke("stop-analysis"),
  openPath: (path) => ipcRenderer.invoke("open-path", path),
  showInFolder: (path) => ipcRenderer.invoke("show-in-folder", path),
  deletePath: (path) => ipcRenderer.invoke("delete-path", path),
  onAnalysisLog: (callback) => {
    const listener = (_event, chunk) => callback(chunk);
    ipcRenderer.on("analysis-log", listener);
    return () => ipcRenderer.removeListener("analysis-log", listener);
  },
  removeAnalysisLogListener: (listener) => {
    if (listener) listener();
  },
});
