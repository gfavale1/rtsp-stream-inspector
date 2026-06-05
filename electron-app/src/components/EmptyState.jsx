import SectionCard from "./SectionCard.jsx";

export default function EmptyState() {
  return (
    <SectionCard title="No analysis yet" description="Run an analysis to view stream metrics and findings.">
      <ul className="empty-list">
        <li>RTP packet statistics</li>
        <li>H.264 NAL inspection</li>
        <li>Jitter and timing metrics</li>
        <li>Security findings</li>
      </ul>
    </SectionCard>
  );
}
