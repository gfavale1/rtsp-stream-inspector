import SectionCard from "./SectionCard.jsx";
import { MetricRow } from "./shared.jsx";
import { formatBytes, formatMbps, formatNumber } from "../utils/format.js";

export default function StreamMetricsPanel({ metrics }) {
  return (
    <SectionCard title="Stream metrics" description="Throughput and packet rate.">
      <div className="metric-list">
        <MetricRow label="Capture duration" value={`${formatNumber(metrics?.capture_duration_seconds, 2)} s`} />
        <MetricRow label="RTP bitrate" value={formatMbps(metrics?.rtp_bitrate_mbps)} />
        <MetricRow label="H.264 bitrate" value={formatMbps(metrics?.h264_payload_bitrate_mbps)} />
        <MetricRow label="RTP packets/sec" value={`${formatNumber(metrics?.rtp_packets_per_second, 1)} pps`} />
        <MetricRow label="Average RTP packet size" value={formatBytes(metrics?.average_rtp_packet_size)} />
        <MetricRow label="Average H.264 payload size" value={formatBytes(metrics?.average_h264_payload_size)} />
      </div>
    </SectionCard>
  );
}
