export default function StatusPill({ status = "idle" }) {
  const label = {
    idle: "Idle",
    running: "Running",
    success: "Completed",
    error: "Error",
    stopped: "Stopped"
  }[status] ?? status;

  return <span className={`status-pill status-${status}`}>{label}</span>;
}
