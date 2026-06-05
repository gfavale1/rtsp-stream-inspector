import { useEffect, useRef } from "react";
import SectionCard from "./SectionCard.jsx";

export default function LiveLogPanel({ logs, isRunning, onClear }) {
  const logRef = useRef(null);

  useEffect(() => {
    if (logRef.current) {
      logRef.current.scrollTop = logRef.current.scrollHeight;
    }
  }, [logs]);

  return (
    <SectionCard
      title="Logs"
      description={isRunning ? "Analyzer output is updating live." : "Analyzer output from the last run."}
      actions={
        <button type="button" className="small-button" onClick={onClear}>
          Clear
        </button>
      }
    >
      <pre ref={logRef} className="log-panel">
        {logs.length === 0 ? "No logs yet." : logs.join("")}
      </pre>
    </SectionCard>
  );
}
