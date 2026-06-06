import { useState } from "react";
import StatusPill from "./StatusPill.jsx";

export default function ConnectionForm({
  form,
  setForm,
  status,
  isRunning,
  onStart,
  onStop,
  lastLogLine
}) {
  const [advancedOpen, setAdvancedOpen] = useState(false);
  const [showUrl, setShowUrl] = useState(false);
  const [pickerError, setPickerError] = useState("");

  function updateField(field, value) {
    setForm((previous) => ({ ...previous, [field]: value }));
  }

  function submit(event) {
    event.preventDefault();
    onStart();
  }

  async function browseBinary() {
    setPickerError("");

    try {
      const result = await window.rtspInspector.selectBinaryPath();

      if (!result.canceled && result.path) {
        updateField("binaryPath", result.path);
      }
    } catch (error) {
      setPickerError(error?.message ?? "Unable to select binary.");
    }
  }

  async function selectOutputDirectory() {
    setPickerError("");

    try {
      const result = await window.rtspInspector.selectOutputDirectory();

      if (!result.canceled && result.path) {
        updateField("outputDirectory", result.path);
      }
    } catch (error) {
      setPickerError(error?.message ?? "Unable to select output directory.");
    }
  }

  return (
    <aside className="sidebar">
      <div className="brand">
        <div className="brand-mark">R</div>
        <div>
          <h1>RTSP Inspector</h1>
          <p>Stream diagnostics</p>
        </div>
      </div>

      <div className="compact-status">
        <StatusPill status={status} />
        <span>{lastLogLine}</span>
      </div>

      <form className="connection-form" onSubmit={submit}>
        <label className="field">
          <span>RTSP URL</span>
          <div className="url-row">
            <input
              type={showUrl ? "text" : "password"}
              placeholder="rtsp://host:554/stream"
              value={form.rtspUrl}
              onChange={(event) => updateField("rtspUrl", event.target.value)}
              disabled={isRunning}
              required
            />
            <button
              type="button"
              className="text-button"
              onClick={() => setShowUrl((value) => !value)}
              disabled={isRunning}
            >
              {showUrl ? "Hide" : "Show"}
            </button>
          </div>
          <small>Credentials are not shown in logs or reports.</small>
        </label>

        <div className="field-row">
          <label className="field">
            <span>Timeout</span>
            <input
              type="number"
              min="1"
              value={form.timeoutMs}
              onChange={(event) => updateField("timeoutMs", Number(event.target.value))}
              disabled={isRunning}
            />
          </label>

          <label className="field">
            <span>Frames</span>
            <input
              type="number"
              min="1"
              value={form.frames}
              onChange={(event) => updateField("frames", Number(event.target.value))}
              disabled={isRunning}
            />
          </label>
        </div>

        <label className="field">
          <span>Packet log limit</span>
          <input
            type="number"
            min="0"
            value={form.packetLogLimit}
            onChange={(event) => updateField("packetLogLimit", Number(event.target.value))}
            disabled={isRunning}
          />
        </label>

        <button
          type="button"
          className="advanced-link"
          onClick={() => setAdvancedOpen((value) => !value)}
        >
          <span>Advanced</span>
          <span>{advancedOpen ? "Hide" : "Show"}</span>
        </button>

        {advancedOpen && (
          <div className="advanced-panel">
            <label className="field">
              <span>Binary path</span>
              <div className="path-row">
                <input
                  type="text"
                  value={form.binaryPath}
                  onChange={(event) => updateField("binaryPath", event.target.value)}
                  disabled={isRunning}
                  placeholder="../build/rtsp-inspector"
                />
                <button type="button" className="small-button" disabled={isRunning} onClick={browseBinary}>
                  Browse
                </button>
              </div>
            </label>

            <label className="field">
              <span>Output directory</span>
              <div className="path-row">
                <input
                  type="text"
                  value={form.outputDirectory}
                  onChange={(event) => updateField("outputDirectory", event.target.value)}
                  disabled={isRunning}
                  placeholder="Temporary directory"
                />
                <button
                  type="button"
                  className="small-button"
                  disabled={isRunning}
                  onClick={selectOutputDirectory}
                >
                  Select
                </button>
              </div>
              <small>Leave empty to use a temporary folder.</small>
            </label>

            <label className="checkbox-row">
              <input
                type="checkbox"
                checked={form.generateMarkdown}
                onChange={(event) => updateField("generateMarkdown", event.target.checked)}
                disabled={isRunning}
              />
              Generate Markdown report
            </label>

            {pickerError && <p className="inline-error">{pickerError}</p>}
          </div>
        )}

        <div className="form-actions">
          <button type="submit" className="primary-button" disabled={isRunning}>
            {isRunning ? "Running" : "Start analysis"}
          </button>

          <button
            type="button"
            className={isRunning ? "stop-button visible" : "stop-button"}
            disabled={!isRunning}
            onClick={onStop}
          >
            Stop
          </button>
        </div>
      </form>
    </aside>
  );
}
