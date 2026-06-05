export function MetricRow({ label, value }) {
  return (
    <div className="metric-row">
      <span>{label}</span>
      <strong>{value ?? "—"}</strong>
    </div>
  );
}
