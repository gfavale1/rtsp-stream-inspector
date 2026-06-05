const { contextBridge, ipcRenderer } = require("electron");

contextBridge.exposeInMainWorld("rtspInspector", {
  runAnalysis: (options) => ipcRenderer.invoke("run-analysis", options),
  stopAnalysis: () => ipcRenderer.invoke("stop-analysis"),

  onAnalysisLog: (callback) => {
    const listener = (_event, chunk) => callback(chunk);
    ipcRenderer.on("analysis-log", listener);
    return () => ipcRenderer.removeListener("analysis-log", listener);
  },

  removeAnalysisLogListener: (listener) => {
    if (listener) {
      listener();
    }
  }
});
