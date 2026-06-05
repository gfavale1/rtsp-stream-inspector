import SectionCard from "./SectionCard.jsx";
import { MetricRow } from "./shared.jsx";

async function copyText(text) {
  if (!text) return;
  await navigator.clipboard.writeText(text);
}

function ReportRow({ label, path }) {
  return (
    <div className="report-row">
      <div>
        <strong>{label}</strong>
        <code>{path ?? "—"}</code>
      </div>
      <button type="button" className="small-button" disabled={!path} onClick={() => copyText(path)}>
        Copy
      </button>
    </div>
  );
}

export default function ReportsPanel({ paths, metadata, configuration, compact = false }) {
  if (compact) {
    return (
      <SectionCard title="Reports">
        <div className="compact-meta">
          <MetricRow label="Command" value={metadata?.command ?? "analyze"} />
          <MetricRow label="Generated" value={metadata?.generated_at_utc ?? "—"} />
        </div>
        <ReportRow label="JSON" path={paths?.json} />
        <ReportRow label="Markdown" path={paths?.markdown} />
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
        <ReportRow label="JSON report" path={paths?.json} />
        <ReportRow label="Markdown report" path={paths?.markdown} />
      </div>
    </SectionCard>
  );
}
