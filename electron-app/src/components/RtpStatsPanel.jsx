import SectionCard from "./SectionCard.jsx";
import { MetricRow } from "./shared.jsx";
import { formatBytes, formatInteger, formatPercent } from "../utils/format.js";

export default function RtpStatsPanel({ rtp }) {
  return (
    <SectionCard title="RTP" description="Packet counters and sequence tracking.">
      <div className="metric-list">
        <MetricRow label="Packets received" value={formatInteger(rtp?.packets_received)} />
        <MetricRow label="Packets lost" value={formatInteger(rtp?.packets_lost)} />
        <MetricRow label="Loss rate" value={formatPercent(rtp?.loss_rate)} />
        <MetricRow label="Out-of-order packets" value={formatInteger(rtp?.out_of_order_packets)} />
        <MetricRow label="Total RTP bytes" value={formatBytes(rtp?.total_rtp_bytes)} />
        <MetricRow label="Payload bytes" value={formatBytes(rtp?.total_payload_bytes)} />
        <MetricRow label="First sequence" value={formatInteger(rtp?.first_sequence_number)} />
        <MetricRow label="Last sequence" value={formatInteger(rtp?.last_sequence_number)} />
        <MetricRow label="Last timestamp" value={formatInteger(rtp?.last_timestamp)} />
        <MetricRow label="Payload type" value={formatInteger(rtp?.payload_type)} />
        <MetricRow label="SSRC" value={formatInteger(rtp?.ssrc)} />
      </div>
    </SectionCard>
  );
}
