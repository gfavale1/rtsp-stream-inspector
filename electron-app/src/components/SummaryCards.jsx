import MetricCard from "./MetricCard.jsx";
import { formatInteger, formatMbps, formatMs, formatPercent } from "../utils/format.js";

function countFindings(findings = []) {
  return findings.reduce(
    (acc, finding) => {
      if (finding.severity === "critical") acc.critical += 1;
      else if (finding.severity === "warning") acc.warning += 1;
      else if (finding.severity === "ok") acc.ok += 1;
      return acc;
    },
    { ok: 0, warning: 0, critical: 0 }
  );
}

export default function SummaryCards({ report }) {
  const counts = countFindings(report?.findings);

  return (
    <div className="summary-grid">
      <MetricCard
        label="Packets"
        value={formatInteger(report?.rtp?.packets_received)}
        subtext={`${formatInteger(report?.interleaved?.rtp_frames_received)} RTP frames`}
      />
      <MetricCard
        label="Packet loss"
        value={formatInteger(report?.rtp?.packets_lost)}
        subtext={`${formatPercent(report?.rtp?.loss_rate)} loss rate`}
        tone={report?.rtp?.packets_lost > 0 ? "warning" : "ok"}
      />
      <MetricCard
        label="Bitrate"
        value={formatMbps(report?.stream_metrics?.rtp_bitrate_mbps)}
        subtext="RTP stream"
      />
      <MetricCard
        label="Jitter"
        value={formatMs(report?.rtp_quality?.jitter_ms)}
        subtext={`${formatMs(report?.rtp_quality?.max_interarrival_gap_ms)} max gap`}
        tone={report?.rtp_quality?.jitter_ms > 50 ? "warning" : "ok"}
      />
      <MetricCard
        label="Checks"
        value={`${counts.warning + counts.critical}`}
        subtext={`${counts.ok} passed · ${counts.warning} warnings`}
        tone={counts.critical > 0 ? "critical" : counts.warning > 0 ? "warning" : "ok"}
      />
    </div>
  );
}
