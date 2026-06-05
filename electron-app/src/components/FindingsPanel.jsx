import SectionCard from "./SectionCard.jsx";

function groupedFindings(findings = []) {
  return {
    critical: findings.filter((finding) => finding.severity === "critical"),
    warning: findings.filter((finding) => finding.severity === "warning"),
    ok: findings.filter((finding) => finding.severity === "ok")
  };
}

function labelFor(severity) {
  if (severity === "critical") return "CRIT";
  if (severity === "warning") return "WARN";
  if (severity === "ok") return "OK";
  return "INFO";
}

function FindingItem({ finding }) {
  return (
    <article className="finding-item">
      <span className={`severity severity-${finding.severity}`}>{labelFor(finding.severity)}</span>
      <div>
        <strong>{finding.code}</strong>
        <p>{finding.message}</p>
      </div>
    </article>
  );
}

function FindingGroup({ title, findings }) {
  if (!findings.length) return null;

  return (
    <div className="finding-group">
      <h3>{title}</h3>
      <div className="finding-list">
        {findings.map((finding, index) => (
          <FindingItem key={`${finding.code}-${index}`} finding={finding} />
        ))}
      </div>
    </div>
  );
}

export default function FindingsPanel({ findings = [], compact = false }) {
  const grouped = groupedFindings(findings);
  const important = [...grouped.critical, ...grouped.warning];

  if (compact) {
    return (
      <SectionCard title="Stream health">
        <div className="health-summary">
          <span>{grouped.ok.length} passed</span>
          <span>{grouped.warning.length} warnings</span>
          <span>{grouped.critical.length} critical</span>
        </div>

        {important.length === 0 ? (
          <p className="quiet-success">No warnings detected.</p>
        ) : (
          <div className="finding-list">
            {important.slice(0, 4).map((finding, index) => (
              <FindingItem key={`${finding.code}-${index}`} finding={finding} />
            ))}
          </div>
        )}
      </SectionCard>
    );
  }

  return (
    <SectionCard title="Findings" description="Grouped by severity.">
      <FindingGroup title="Critical" findings={grouped.critical} />
      <FindingGroup title="Warnings" findings={grouped.warning} />
      <FindingGroup title="Passed checks" findings={grouped.ok} />

      {findings.length === 0 && <p className="empty-copy">No findings available.</p>}
    </SectionCard>
  );
}
