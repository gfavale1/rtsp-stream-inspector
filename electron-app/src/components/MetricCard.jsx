export default function MetricCard({ label, value, subtext, tone = "neutral" }) {
  return (
    <article className={`metric-card tone-${tone}`}>
      <span className="metric-label">{label}</span>
      <strong>{value}</strong>
      {subtext && <small>{subtext}</small>}
    </article>
  );
}
