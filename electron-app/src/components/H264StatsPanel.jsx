import SectionCard from "./SectionCard.jsx";
import { MetricRow } from "./shared.jsx";
import { formatInteger } from "../utils/format.js";

export default function H264StatsPanel({ h264 }) {
  return (
    <SectionCard title="H.264" description="NAL unit distribution.">
      <div className="metric-list two-columns">
        <MetricRow label="NAL units" value={formatInteger(h264?.nal_units_seen)} />
        <MetricRow label="SPS" value={formatInteger(h264?.sps)} />
        <MetricRow label="PPS" value={formatInteger(h264?.pps)} />
        <MetricRow label="SEI" value={formatInteger(h264?.sei)} />
        <MetricRow label="IDR slices" value={formatInteger(h264?.idr_slices)} />
        <MetricRow label="Non-IDR slices" value={formatInteger(h264?.non_idr_slices)} />
        <MetricRow label="STAP-A packets" value={formatInteger(h264?.stap_a_packets)} />
        <MetricRow label="FU-A packets" value={formatInteger(h264?.fu_a_packets)} />
        <MetricRow label="FU-A starts" value={formatInteger(h264?.fu_a_starts)} />
        <MetricRow label="FU-A ends" value={formatInteger(h264?.fu_a_ends)} />
        <MetricRow label="Unknown NAL units" value={formatInteger(h264?.unknown_nal_units)} />
      </div>
    </SectionCard>
  );
}
