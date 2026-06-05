import SectionCard from "./SectionCard.jsx";
import { MetricRow } from "./shared.jsx";
import { formatInteger, formatMs, formatNumber } from "../utils/format.js";

export default function RtpQualityPanel({ quality }) {
  return (
    <SectionCard title="RTP quality" description="Jitter and inter-arrival timing.">
      <div className="metric-list">
        <MetricRow label="Packets observed" value={formatInteger(quality?.packets_observed)} />
        <MetricRow label="Jitter" value={formatMs(quality?.jitter_ms)} />
        <MetricRow label="Jitter seconds" value={`${formatNumber(quality?.jitter_seconds, 4)} s`} />
        <MetricRow label="Jitter timestamp units" value={formatNumber(quality?.jitter_timestamp_units, 2)} />
        <MetricRow label="Average gap" value={formatMs(quality?.average_interarrival_gap_ms)} />
        <MetricRow label="Max gap" value={formatMs(quality?.max_interarrival_gap_ms)} />
      </div>
    </SectionCard>
  );
}
