import { useState } from "react";
import SectionCard from "./SectionCard.jsx";
import { MetricRow } from "./shared.jsx";

async function copyText(text) {
  if (!text) return;
  await navigator.clipboard.writeText(text);
}

function ReportRow({ label, path, onStatus }) {
  async function handleCopy() {
    try {
      await copyText(path);
      onStatus(`${label} path copied.`);
    } catch (error) {
      onStatus(error?.message ?? `Unable to copy ${label} path.`);
    }
  }

  async function handleOpen() {
    try {
      const result = await window.rtspInspector.openPath(path);

      if (!result.ok) {
        onStatus(result.error || `Unable to open ${label}.`);
        return;
      }

      onStatus(`${label} opened.`);
    } catch (error) {
      onStatus(error?.message ?? `Unable to open ${label}.`);
    }
  }

  async function handleShowInFolder() {
    try {
      const result = await window.rtspInspector.showInFolder(path);

      if (!result.ok) {
        onStatus(result.error || `Unable to show ${label} in folder.`);
        return;
      }

      onStatus(`${label} revealed in folder.`);
    } catch (error) {
      onStatus(error?.message ?? `Unable to show ${label} in folder.`);
    }
  }

  const generated = Boolean(path);

  return (
    <div className="report-row">
      <div>
        <strong>{label}</strong>
        <code>{generated ? path : "Not generated"}</code>
      </div>
      <div className="report-actions">
        <button type="button" className="small-button" disabled={!generated} onClick={handleCopy}>
          Copy
        </button>
        <button type="button" className="small-button" disabled={!generated} onClick={handleOpen}>
          Open
        </button>
        <button type="button" className="small-button" disabled={!generated} onClick={handleShowInFolder}>
          Show
        </button>
      </div>
    </div>
  );
}

export default function ReportsPanel({ paths, metadata, configuration, compact = false }) {
  const [statusMessage, setStatusMessage] = useState("");

  if (compact) {
    return (
      <SectionCard title="Reports">
        <div className="compact-meta">
          <MetricRow label="Command" value={metadata?.command ?? "analyze"} />
          <MetricRow label="Generated" value={metadata?.generated_at_utc ?? "—"} />
        </div>

        <div className="report-list">
          <ReportRow label="JSON" path={paths?.json} onStatus={setStatusMessage} />
          <ReportRow label="Markdown" path={paths?.markdown} onStatus={setStatusMessage} />
        </div>

        {statusMessage && <p className="inline-status">{statusMessage}</p>}
      </SectionCard>
    );
  }

  return (
    <SectionCard title="Reports" description="Generated files for this analysis.">
      <div className="metadata-grid">
        <MetricRow label="Command" value={metadata?.command ?? "—"} />
        <MetricRow label="Generated" value={metadata?.generated_at_utc ?? "—"} />
        <MetricRow label="Frames" value={configuration?.frames_requested ?? "—"} />
        <MetricRow label="Packet log" value={configuration?.packet_log_limit ?? "—"} />
        <MetricRow
          label="Timeout"
          value={configuration?.timeout_ms ? `${configuration.timeout_ms} ms` : "—"}
        />
      </div>

      <div className="report-list">
        <ReportRow label="JSON report" path={paths?.json} onStatus={setStatusMessage} />
        <ReportRow label="Markdown report" path={paths?.markdown} onStatus={setStatusMessage} />
      </div>

      {statusMessage && <p className="inline-status">{statusMessage}</p>}
    </SectionCard>
  );
}
