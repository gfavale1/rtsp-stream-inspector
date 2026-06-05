import { useEffect, useMemo, useState } from "react";
import ConnectionForm from "./components/ConnectionForm.jsx";
import SummaryCards from "./components/SummaryCards.jsx";
import RtpStatsPanel from "./components/RtpStatsPanel.jsx";
import H264StatsPanel from "./components/H264StatsPanel.jsx";
import StreamMetricsPanel from "./components/StreamMetricsPanel.jsx";
import RtpQualityPanel from "./components/RtpQualityPanel.jsx";
import FindingsPanel from "./components/FindingsPanel.jsx";
import ReportsPanel from "./components/ReportsPanel.jsx";
import LiveLogPanel from "./components/LiveLogPanel.jsx";
import StatusPill from "./components/StatusPill.jsx";
import Tabs from "./components/Tabs.jsx";
import EmptyState from "./components/EmptyState.jsx";
import { lastNonEmptyLine } from "./utils/format.js";

const initialForm = {
  rtspUrl: "",
  binaryPath: "../build/rtsp-inspector",
  timeoutMs: 5000,
  frames: 1000,
  packetLogLimit: 0,
  generateMarkdown: true
};

function countFindings(findings = []) {
  return findings.reduce(
    (acc, finding) => {
      if (finding.severity === "critical") acc.critical += 1;
      else if (finding.severity === "warning") acc.warning += 1;
      else if (finding.severity === "ok") acc.ok += 1;
      return acc;
    },
    { ok: 0, warning: 0, critical: 0 }
  );
}

export default function App() {
  const [form, setForm] = useState(initialForm);
  const [status, setStatus] = useState("idle");
  const [logs, setLogs] = useState([]);
  const [report, setReport] = useState(null);
  const [reportPaths, setReportPaths] = useState(null);
  const [error, setError] = useState("");
  const [activeTab, setActiveTab] = useState("overview");

  const isRunning = status === "running";
  const lastLine = useMemo(() => lastNonEmptyLine(logs), [logs]);
  const findingCounts = useMemo(() => countFindings(report?.findings), [report]);
  const importantFindingsCount = findingCounts.warning + findingCounts.critical;

  useEffect(() => {
    const removeListener = window.rtspInspector.onAnalysisLog((chunk) => {
      setLogs((previous) => [...previous, chunk]);
    });

    return () => {
      window.rtspInspector.removeAnalysisLogListener(removeListener);
    };
  }, []);

  async function handleStartAnalysis() {
    setStatus("running");
    setError("");
    setReport(null);
    setReportPaths(null);
    setLogs([]);
    setActiveTab("overview");

    try {
      const result = await window.rtspInspector.runAnalysis(form);
      setReport(result.report);
      setReportPaths(result.paths);
      setStatus("success");
    } catch (err) {
      const message = err?.message ?? "Analysis failed.";

      if (message.toLowerCase().includes("stopped")) {
        setStatus("stopped");
        setError("");
      } else {
        setStatus("error");
        setError(message);
      }
    }
  }

  async function handleStopAnalysis() {
    try {
      await window.rtspInspector.stopAnalysis();
      setStatus("stopped");
    } catch (err) {
      setStatus("error");
      setError(err?.message ?? "Unable to stop analysis.");
    }
  }

  function clearLogs() {
    setLogs([]);
  }

  function renderTab() {
    if (!report && activeTab !== "logs") {
      return <EmptyState />;
    }

    switch (activeTab) {
      case "overview":
        return (
          <div className="overview-stack">
            <SummaryCards report={report} />
            <div className="overview-columns">
              <FindingsPanel findings={report?.findings} compact />
              <ReportsPanel
                paths={reportPaths}
                metadata={report?.metadata}
                configuration={report?.configuration}
                compact
              />
            </div>
          </div>
        );

      case "rtp":
        return <RtpStatsPanel rtp={report?.rtp} />;

      case "h264":
        return <H264StatsPanel h264={report?.h264} />;

      case "quality":
        return (
          <div className="details-grid">
            <StreamMetricsPanel metrics={report?.stream_metrics} />
            <RtpQualityPanel quality={report?.rtp_quality} />
          </div>
        );

      case "findings":
        return <FindingsPanel findings={report?.findings} />;

      case "reports":
        return (
          <ReportsPanel
            paths={reportPaths}
            metadata={report?.metadata}
            configuration={report?.configuration}
          />
        );

      case "logs":
        return <LiveLogPanel logs={logs} isRunning={isRunning} onClear={clearLogs} />;

      default:
        return null;
    }
  }

  return (
    <div className="app-shell">
      <ConnectionForm
        form={form}
        setForm={setForm}
        status={status}
        isRunning={isRunning}
        onStart={handleStartAnalysis}
        onStop={handleStopAnalysis}
        lastLogLine={lastLine}
      />

      <main className="main-content">
        <header className="topbar">
          <div>
            <h1>Stream Inspector</h1>
            <p>Analyze RTSP/RTP/H.264 streams.</p>
          </div>

          <div className="topbar-side">
            <StatusPill status={status} />
            <span>{report?.metadata?.command ?? "analyze"}</span>
            <span>{report?.metadata?.generated_at_utc ?? "Not generated"}</span>
          </div>
        </header>

        {error && (
          <div className="error-banner">
            <strong>Analysis error</strong>
            <span>{error}</span>
          </div>
        )}

        <Tabs activeTab={activeTab} onChange={setActiveTab} findingsCount={importantFindingsCount} />

        <section className="tab-content">{renderTab()}</section>
      </main>
    </div>
  );
}
