export const tabs = [
  { id: "overview", label: "Overview" },
  { id: "rtp", label: "RTP" },
  { id: "h264", label: "H.264" },
  { id: "quality", label: "Quality" },
  { id: "findings", label: "Findings" },
  { id: "reports", label: "Reports" },
  { id: "logs", label: "Logs" }
];

export default function Tabs({ activeTab, onChange, findingsCount = 0 }) {
  return (
    <nav className="tabs" aria-label="Sections">
      {tabs.map((tab) => (
        <button
          key={tab.id}
          type="button"
          className={activeTab === tab.id ? "tab active" : "tab"}
          onClick={() => onChange(tab.id)}
        >
          {tab.label}
          {tab.id === "findings" && findingsCount > 0 && (
            <span className="tab-count">{findingsCount}</span>
          )}
        </button>
      ))}
    </nav>
  );
}
