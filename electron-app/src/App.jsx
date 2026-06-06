import { useEffect, useMemo, useRef, useState } from "react";
import {
  Activity,
  AlertTriangle,
  BarChart3,
  CheckCircle2,
  CircleDot,
  Copy,
  FileJson,
  FileText,
  FolderOpen,
  Gauge,
  Hexagon,
  Eye,
  EyeOff,
  Info,
  Play,
  Radio,
  Settings,
  ShieldAlert,
  ShieldCheck,
  Square,
  Terminal,
  Video,
  Wifi,
  Zap,
} from "lucide-react";

const initialForm = {
  rtspUrl: "",
  binaryPath: "../build/rtsp-inspector",
  timeoutMs: 5000,
  frames: 1000,
  packetLogLimit: 0,
  generateMarkdown: true,
};

const tabs = [
  { id: "rtp", label: "RTP Stats" },
  { id: "h264", label: "H.264 Inspection" },
  { id: "quality", label: "Quality Metrics" },
];

function n(value, digits = 0) {
  if (value === undefined || value === null || Number.isNaN(Number(value))) return "—";
  return Number(value).toLocaleString(undefined, { maximumFractionDigits: digits });
}

function fixed(value, digits = 2, suffix = "") {
  if (value === undefined || value === null || Number.isNaN(Number(value))) return "—";
  return `${Number(value).toFixed(digits)}${suffix}`;
}

function severityStyle(severity = "ok") {
  const normalized = severity.toLowerCase();
  if (normalized === "critical") return "severity-critical";
  if (normalized === "warning") return "severity-warning";
  return "severity-ok";
}

function countFindings(findings = []) {
  return findings.reduce(
    (acc, finding) => {
      const severity = finding.severity?.toLowerCase();
      if (severity === "critical") acc.critical += 1;
      else if (severity === "warning") acc.warning += 1;
      else acc.ok += 1;
      return acc;
    },
    { ok: 0, warning: 0, critical: 0 }
  );
}

function lastNonEmptyLine(logs) {
  return logs
    .join("")
    .split(/\r?\n/)
    .map((line) => line.trim())
    .filter(Boolean)
    .at(-1);
}

function NavIcon({ icon: Icon, label, active, onClick }) {
  return (
    <button
      className={`dock-button ${active ? "active" : ""}`}
      title={label}
      aria-label={label}
      aria-pressed={active}
      onClick={onClick}
      type="button"
    >
      <Icon size={18} />
    </button>
  );
}

function StatusBadge({ status }) {
  const label = {
    idle: "Idle",
    running: "Running",
    success: "Completed",
    error: "Error",
    stopped: "Stopped",
  }[status] ?? status;
  return <span className={`status-badge status-${status}`}>{label}</span>;
}

function Field({ label, children, hint }) {
  return (
    <label className="field-shell">
      <span>{label}</span>
      {children}
      {hint ? <small>{hint}</small> : null}
    </label>
  );
}

function MetricValue({ value, unit }) {
  return (
    <strong className="metric-value">
      <span className="metric-number">{value}</span>
      {unit ? <span className="metric-unit">{unit}</span> : null}
    </strong>
  );
}

function MicroMetric({ label, value, unit, hint, tone = "neutral", muted = false }) {
  return (
    <div className={`micro-metric tone-${tone} ${muted ? "is-muted" : ""}`}>
      <span>{label}</span>
      <MetricValue value={value} unit={unit} />
      {hint ? <small>{hint}</small> : null}
    </div>
  );
}

function CounterTile({ label, value, unit, tone = "neutral" }) {
  return (
    <div className={`counter-tile tone-${tone}`}>
      <span>{label}</span>
      <MetricValue value={value} unit={unit} />
    </div>
  );
}

function ProgressRow({ label, value, tone = "accent" }) {
  const width = Math.max(0, Math.min(100, Number(value) || 0));
  return (
    <div className="progress-row">
      <div>
        <span>{label}</span>
        <strong>{fixed(width, 0, "%")}</strong>
      </div>
      <div className="progress-track">
        <div className={`progress-fill fill-${tone}`} style={{ width: `${width}%` }} />
      </div>
    </div>
  );
}

function FindingItem({ finding }) {
  const severity = finding.severity ?? "ok";
  return (
    <div className="finding-row">
      <span className={`severity-pill ${severityStyle(severity)}`}>{severity.toUpperCase()}</span>
      <div>
        <strong>{finding.code?.replaceAll("_", " ") ?? "Finding"}</strong>
        <p>{finding.message ?? finding.title ?? "No description available."}</p>
      </div>
    </div>
  );
}

function ReportsPanel({ reportPaths, compact = false, table = false, onDelete }) {
  const copyPath = async (path) => {
    if (!path) return;
    await navigator.clipboard?.writeText(path);
  };

  const openPath = async (path) => {
    if (!path) return;
    await window.rtspInspector?.openPath?.(path);
  };

  const showInFolder = async (path) => {
    if (!path) return;
    await window.rtspInspector?.showInFolder?.(path);
  };

  const deletePath = async (path) => {
    if (!path) return;
    await window.rtspInspector?.deletePath?.(path);
    onDelete?.(path);
  };

  if (!reportPaths) {
    return (
      <div className="report-empty">
        <FileText size={16} />
        <span>Reports will appear here after a completed analysis.</span>
      </div>
    );
  }

  const rows = [
    { label: "JSON report", type: "JSON", path: reportPaths.json, icon: FileJson },
    { label: "Markdown report", type: "Markdown", path: reportPaths.markdown, icon: FileText },
    { label: "Output directory", type: "Directory", path: reportPaths.directory, icon: FolderOpen, directory: true },
  ].filter((row) => row.path);

  if (table) {
    return (
      <div className="report-table-wrap">
        <table className="report-table">
          <thead>
            <tr>
              <th>Artifact</th>
              <th>Type</th>
              <th>Path</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((row) => (
              <tr key={row.label}>
                <td><span className="artifact-name"><row.icon size={15} />{row.label}</span></td>
                <td><span className="table-pill">{row.type}</span></td>
                <td><code>{row.path}</code></td>
                <td>
                  <div className="table-actions">
                    <button onClick={() => openPath(row.path)}>Open File</button>
                    <button onClick={() => copyPath(row.path)}>Copy Path</button>
                    <button onClick={() => deletePath(row.path)} disabled={row.directory}>Delete</button>
                  </div>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    );
  }

  return (
    <div className={`report-list ${compact ? "compact-reports" : ""}`}>
      {rows.map((row) => (
        <div className="report-row" key={row.label}>
          <row.icon size={16} />
          <div>
            <strong>{row.label}</strong>
            <code>{row.path}</code>
          </div>
          <div className="report-actions">
            <button className="ghost-icon" onClick={() => openPath(row.path)} title="Open">
              <FolderOpen size={14} />
            </button>
            <button className="ghost-icon" onClick={() => showInFolder(row.path)} title="Show in folder">
              <FileText size={14} />
            </button>
            <button className="ghost-icon" onClick={() => copyPath(row.path)} title="Copy path">
              <Copy size={14} />
            </button>
          </div>
        </div>
      ))}
    </div>
  );
}

function App() {
  const [form, setForm] = useState(initialForm);
  const [status, setStatus] = useState("idle");
  const [logs, setLogs] = useState([]);
  const [report, setReport] = useState(null);
  const [reportPaths, setReportPaths] = useState(null);
  const [error, setError] = useState("");
  const [activeTab, setActiveTab] = useState("rtp");
  const [activeSidebarTab, setActiveSidebarTab] = useState("connection");
  const [logsCollapsed, setLogsCollapsed] = useState(false);
  const [showEndpoint, setShowEndpoint] = useState(true);
  const logQueueRef = useRef([]);
  const logTimerRef = useRef(null);

  const isRunning = status === "running";

  const findings = report?.findings ?? [];
  const findingCounts = useMemo(() => countFindings(findings), [findings]);
  const lastLine = useMemo(() => lastNonEmptyLine(logs), [logs]);
  const logsText = useMemo(() => logs.join(""), [logs]);

  const hasSuccessfulPlay = useMemo(() => {
    const text = logsText.toLowerCase();
    return /rtsp\s+play.*(200|ok|completed|success)|play.*(200|ok|completed|success)|rtp stream started/.test(text);
  }, [logsText]);

  const hasRtpPackets =
    Number(report?.rtp?.packets_received ?? 0) > 0;

  const hasCompletedAnalysis =
    Boolean(report) && hasRtpPackets;

  const isStreamEstablished =
    hasSuccessfulPlay || hasRtpPackets;

  const canShowLiveMetrics =
    hasCompletedAnalysis;

  const hasAnalysisData =
    Boolean(report) || isRunning || isStreamEstablished;

  const healthScore = useMemo(() => {
    const loss = Number(report?.rtp?.loss_rate ?? 0) * 100;
    const jitter = Number(report?.rtp_quality?.jitter_ms ?? 0);
    const penalties = findingCounts.critical * 25 + findingCounts.warning * 8 + Math.min(loss * 4, 20) + Math.min(jitter, 10);
    return Math.max(0, Math.round(100 - penalties));
  }, [report, findingCounts]);

  useEffect(() => {
    if (!window.rtspInspector?.onAnalysisLog) return undefined;

    const pushProgressiveLog = (chunk) => {
      const text = String(chunk ?? "");
      const pieces = text.match(/[^\n]*\n|[^\n]+/g) ?? [text];
      logQueueRef.current.push(...pieces.filter(Boolean));

      if (logTimerRef.current) return;
      logTimerRef.current = window.setInterval(() => {
        const next = logQueueRef.current.shift();
        if (next) {
          setLogs((previous) => [...previous, next]);
          return;
        }
        window.clearInterval(logTimerRef.current);
        logTimerRef.current = null;
      }, 28);
    };

    const removeListener = window.rtspInspector.onAnalysisLog(pushProgressiveLog);
    return () => {
      window.rtspInspector.removeAnalysisLogListener?.(removeListener);
      if (logTimerRef.current) window.clearInterval(logTimerRef.current);
      logTimerRef.current = null;
      logQueueRef.current = [];
    };
  }, []);

  function updateForm(field, value) {
    setForm((previous) => ({ ...previous, [field]: value }));
  }

  async function handleStartAnalysis() {
    if (!window.rtspInspector?.runAnalysis) {
      setStatus("error");
      setError("Electron preload API is not available. Run the app inside Electron, not directly in the browser.");
      return;
    }

    setStatus("running");
    setError("");
    setReport(null);
    setReportPaths(null);
    if (logTimerRef.current) window.clearInterval(logTimerRef.current);
    logTimerRef.current = null;
    logQueueRef.current = [];
    setLogs([]);
    setActiveSidebarTab("connection");
    setLogsCollapsed(false);

    try {
      const result = await window.rtspInspector.runAnalysis(form);
      setReport(result.report);
      setReportPaths(result.paths);
      setStatus("success");
    } catch (err) {
      const message = err?.message ?? "Analysis failed.";
      if (message.toLowerCase().includes("stopped")) {
        setStatus("stopped");
      } else {
        setStatus("error");
        setError(message);
      }
    }
  }

  async function handleStopAnalysis() {
    try {
      await window.rtspInspector?.stopAnalysis?.();
      setStatus("stopped");
    } catch (err) {
      setStatus("error");
      setError(err?.message ?? "Unable to stop analysis.");
    }
  }

  function handleReportDeleted(path) {
    setReportPaths((previous) => {
      if (!previous) return previous;
      return Object.fromEntries(Object.entries(previous).map(([key, value]) => [key, value === path ? null : value]));
    });
  }

  const rtp = report?.rtp ?? {};
  const h264 = report?.h264 ?? {};
  const quality = report?.rtp_quality ?? {};
  const metrics = report?.stream_metrics ?? {};
  const source = report?.source ?? {};
  const video = report?.video ?? {};

  const actionableFindings = findings.filter((finding) => {
    const severity = finding.severity?.toLowerCase();
    return severity === "warning" || severity === "critical";
  });
  const warningFindingLabels = actionableFindings.map((finding) =>
    finding.title ?? finding.message ?? finding.code?.replaceAll("_", " ") ?? "Finding"
  );
  const findingsSummary = findingCounts.critical > 0
    ? `${findingCounts.critical} Critical: ${warningFindingLabels.join(", ")}`
    : findingCounts.warning > 0
      ? `${findingCounts.warning} Warnings: ${warningFindingLabels.join(", ")}`
      : report
        ? "No high-risk anomalies detected"
        : "No findings yet";

  const sidebarItems = [
    { id: "connection", label: "Connection & Triage", icon: Radio },
    { id: "analytics", label: "Deep Analytics", icon: Wifi },
    { id: "reports", label: "Report Manager", icon: FileText },
    { id: "settings", label: "Settings", icon: Settings },
  ];

  const MasterPanel = () => (
    <section className="master-panel">
      <header className="master-header">
        <div>
          <h1>RTSP Stream Inspector</h1>
          <p>Network video inspection console</p>
        </div>
        <StatusBadge status={status} />
      </header>

      <div className="connection-card">
        <Field label="RTSP endpoint" hint="The URL is intentionally not persisted.">
          <div className="endpoint-input-wrap">
            <input
              className="mono-input endpoint-input"
              value={form.rtspUrl}
              onChange={(event) => updateForm("rtspUrl", event.target.value)}
              placeholder="rtsp://user:pass@192.168.1.20:554/stream"
              type={showEndpoint ? "text" : "password"}
              disabled={isRunning}
            />
            <button
              className="endpoint-visibility"
              type="button"
              onClick={() => setShowEndpoint((value) => !value)}
              title={showEndpoint ? "Hide endpoint" : "Show endpoint"}
              disabled={isRunning}
            >
              {showEndpoint ? <EyeOff size={14} /> : <Eye size={14} />}
            </button>
          </div>
        </Field>

        <div className="action-grid">
          <button className="primary-action" onClick={handleStartAnalysis} disabled={isRunning}>
            <Play size={15} />
            Analyze
          </button>
          <button className="secondary-action danger" onClick={handleStopAnalysis} disabled={!isRunning}>
            <Square size={14} />
            Stop
          </button>
        </div>

        <div className="form-grid compact">
          <Field label="Timeout">
            <input value={form.timeoutMs} onChange={(e) => updateForm("timeoutMs", e.target.value)} disabled={isRunning} />
          </Field>
          <Field label="Frames">
            <input value={form.frames} onChange={(e) => updateForm("frames", e.target.value)} disabled={isRunning} />
          </Field>
          <Field label="Packet log limit">
            <input value={form.packetLogLimit} onChange={(e) => updateForm("packetLogLimit", e.target.value)} disabled={isRunning} />
          </Field>
          <label className="check-row">
            <input
              type="checkbox"
              checked={form.generateMarkdown}
              onChange={(e) => updateForm("generateMarkdown", e.target.checked)}
              disabled={isRunning}
            />
            Markdown
          </label>
        </div>

        <Field label="Analyzer binary">
          <input
            className="mono-input"
            value={form.binaryPath}
            onChange={(event) => updateForm("binaryPath", event.target.value)}
            disabled={isRunning}
          />
        </Field>
      </div>

      <div className="identity-card">
        <div className="panel-title-row">
          <h2>Stream Identity</h2>
          <Video size={15} />
        </div>
        <div className="identity-grid">
          <span>Codec</span><strong>{video.codec ?? "—"}</strong>
          <span>Payload</span><strong>{video.payload_type ?? rtp.payload_type ?? "—"}</strong>
          <span>Address</span><strong>{source.host ?? "—"}</strong>
          <span>Transport</span><strong>{source.transport ?? report?.configuration?.transport ?? "—"}</strong>
        </div>
      </div>

      <div className="findings-feed">
        <div className="panel-title-row sticky-title">
          <h2>Anomalies & Findings</h2>
          <span className="finding-count">{findingCounts.warning + findingCounts.critical}</span>
        </div>
        <div className="finding-list">
          {findings.length > 0 ? findings.map((finding, index) => <FindingItem finding={finding} key={`${finding.code}-${index}`} />) : (
            <div className="empty-feed">
              <Info size={15} />
              <span>Run an analysis to populate live findings.</span>
            </div>
          )}
        </div>
      </div>
    </section>
  );

  const LogConsole = ({ expanded = false }) => (
    <footer className={`log-console ${logsCollapsed ? "collapsed" : ""} ${expanded ? "expanded" : ""}`}>
      <button className="console-title" onClick={() => setLogsCollapsed((value) => !value)}>
        <Terminal size={15} />
        <span>Live Hex / Log Stream</span>
        <Activity size={14} className={isRunning ? "spin-soft" : ""} />
      </button>
      {!logsCollapsed && (
        <pre>{logs.length ? logs.join("") : "# waiting for backend output...\n# RTP packets and analyzer diagnostics will stream here."}</pre>
      )}
    </footer>
  );

  const AnalyticsTabs = () => (
    <>
      <div className="tab-strip">
        {tabs.map((tab) => (
          <button className={activeTab === tab.id ? "active" : ""} key={tab.id} onClick={() => setActiveTab(tab.id)}>
            {tab.label}
          </button>
        ))}
      </div>

      <section className="canvas-area view-transition">
        {activeTab === "rtp" && (
          <>
            <div className="metric-grid four">
              <MicroMetric label="Packets received" value={n(rtp.packets_received)} hint="RTP frames" />
              <MicroMetric label="Packet loss" value={fixed((rtp.loss_rate ?? 0) * 100, 2)} unit="%" tone={(rtp.loss_rate ?? 0) > 0.02 ? "warning" : "ok"} />
              <MicroMetric label="Out-of-order" value={n(rtp.out_of_order_packets)} tone={rtp.out_of_order_packets > 0 ? "warning" : "ok"} />
              <MicroMetric label="RTP bitrate" value={fixed(metrics.rtp_bitrate_mbps, 2)} unit="Mbps" />
            </div>

            <div className="split-grid">
              <div className="panel-card wide">
                <div className="panel-title-row">
                  <h2>Transport Continuity</h2>
                  <Gauge size={16} />
                </div>
                <ProgressRow label="Packet continuity" value={100 - (Number(rtp.loss_rate ?? 0) * 100)} tone="ok" />
                <ProgressRow label="Timing regularity" value={Math.max(0, 100 - Number(quality.jitter_ms ?? 0) * 6)} />
                <ProgressRow label="Payload efficiency" value={Math.min(100, Number(metrics.h264_payload_bitrate_mbps ?? 0) / Math.max(Number(metrics.rtp_bitrate_mbps ?? 1), 0.01) * 100)} />
              </div>
              <div className="panel-card mono-list">
                <div className="panel-title-row">
                  <h2>RTP Metadata</h2>
                  <CircleDot size={16} />
                </div>
                <span>First sequence <strong>{n(rtp.first_sequence_number)}</strong></span>
                <span>Last sequence <strong>{n(rtp.last_sequence_number)}</strong></span>
                <span>SSRC <strong>{n(rtp.ssrc)}</strong></span>
                <span>Payload bytes <strong>{n(rtp.total_payload_bytes)}</strong></span>
                <span>Packets / sec <strong>{fixed(metrics.rtp_packets_per_second, 2)}</strong></span>
              </div>
            </div>
          </>
        )}

        {activeTab === "h264" && (
          <>
            <div className="panel-card">
              <div className="panel-title-row">
                <h2>H.264 NAL Unit Counters</h2>
                <Hexagon size={16} />
              </div>
              <div className="counter-grid">
                <CounterTile label="SPS" value={n(h264.sps)} tone="ok" />
                <CounterTile label="PPS" value={n(h264.pps)} tone="ok" />
                <CounterTile label="IDR slices" value={n(h264.idr_slices)} />
                <CounterTile label="Non-IDR" value={n(h264.non_idr_slices)} />
                <CounterTile label="FU-A packets" value={n(h264.fu_a_packets)} />
                <CounterTile label="FU-A starts" value={n(h264.fu_a_starts)} />
                <CounterTile label="FU-A ends" value={n(h264.fu_a_ends)} />
                <CounterTile label="Unknown" value={n(h264.unknown_nal_units)} tone={h264.unknown_nal_units > 0 ? "critical" : "ok"} />
              </div>
            </div>
            <div className="panel-card nal-breakdown">
              <div className="panel-title-row">
                <h2>NAL Breakdown</h2>
                <BarChart3 size={16} />
              </div>
              <ProgressRow label="IDR slices" value={Math.min(100, Number(h264.idr_slices ?? 0) / Math.max(Number(h264.nal_units_seen ?? 1), 1) * 100)} tone="ok" />
              <ProgressRow label="Non-IDR slices" value={Math.min(100, Number(h264.non_idr_slices ?? 0) / Math.max(Number(h264.nal_units_seen ?? 1), 1) * 100)} />
              <ProgressRow label="FU-A fragments" value={Math.min(100, Number(h264.fu_a_packets ?? 0) / Math.max(Number(h264.nal_units_seen ?? 1), 1) * 100)} tone="warning" />
            </div>
            <div className="panel-card codec-strip">
              <span>NAL units seen <strong>{n(h264.nal_units_seen)}</strong></span>
              <span>SEI <strong>{n(h264.sei)}</strong></span>
              <span>STAP-A <strong>{n(h264.stap_a_packets)}</strong></span>
              <span>Avg H.264 payload <strong>{fixed(metrics.average_h264_payload_size, 1)} bytes</strong></span>
            </div>
          </>
        )}

        {activeTab === "quality" && (
          <>
            <div className="hero-quality-grid">
              <div className="quality-hero">
                <span>Jitter</span>
                <MetricValue value={fixed(quality.jitter_ms, 2)} unit="ms" />
                <small>RTP timestamp variance</small>
              </div>
              <div className="quality-hero">
                <span>Payload bitrate</span>
                <MetricValue value={fixed(metrics.h264_payload_bitrate_mbps, 3)} unit="Mbps" />
                <small>H.264 payload only</small>
              </div>
              <div className="quality-hero warning">
                <span>Max inter-arrival gap</span>
                <MetricValue value={fixed(quality.max_interarrival_gap_ms, 1)} unit="ms" />
                <small>Largest observed packet gap</small>
              </div>
            </div>
            <div className="split-grid">
              <div className="panel-card">
                <div className="panel-title-row">
                  <h2>Stream Health Metrics</h2>
                  <Zap size={16} />
                </div>
                <ProgressRow label="Jitter stability" value={Math.max(0, 100 - Number(quality.jitter_ms ?? 0) * 8)} tone="ok" />
                <ProgressRow label="Inter-arrival consistency" value={Math.max(0, 100 - Number(quality.average_interarrival_gap_ms ?? 0))} />
                <ProgressRow label="Capture completeness" value={report?.teardown_success ? 100 : 72} tone={report?.teardown_success ? "ok" : "warning"} />
              </div>
              <div className="panel-card mono-list">
                <span>Average gap <strong>{fixed(quality.average_interarrival_gap_ms, 2)} ms</strong></span>
                <span>Observed packets <strong>{n(quality.packets_observed)}</strong></span>
                <span>Capture duration <strong>{fixed(metrics.capture_duration_seconds, 2)} s</strong></span>
                <span>Avg RTP packet <strong>{fixed(metrics.average_rtp_packet_size, 1)} bytes</strong></span>
              </div>
            </div>
          </>
        )}
      </section>
    </>
  );

  const ConnectionTriageView = () => (
    <>
      <section className="connection-canvas view-transition">
        <div className="panel-card stream-summary-card">
          <div className="panel-title-row">
            <h2>Stream Health Summary</h2>
            <ShieldCheck size={16} />
          </div>
          <div className="summary-grid">
            <MicroMetric
              label="Health score"
              value={canShowLiveMetrics ? healthScore : "—"}
              unit={canShowLiveMetrics ? "%" : undefined}
              tone={canShowLiveMetrics ? (healthScore < 75 ? "warning" : "ok") : "neutral"}
              muted={!canShowLiveMetrics}
            />
            <MicroMetric
              label="Findings"
              value={canShowLiveMetrics ? findingCounts.warning + findingCounts.critical : 0}
              hint={canShowLiveMetrics ? findingsSummary : "Waiting for RTSP PLAY response"}
              tone={canShowLiveMetrics ? (findingCounts.critical ? "critical" : findingCounts.warning ? "warning" : "ok") : "neutral"}
              muted={!canShowLiveMetrics}
            />
            <MicroMetric
              label="Packets"
              value={canShowLiveMetrics ? n(rtp.packets_received) : 0}
              hint={canShowLiveMetrics ? "Observed RTP frames" : "Streaming not started"}
              muted={!canShowLiveMetrics}
            />
            <MicroMetric
              label="Max gap"
              value={canShowLiveMetrics ? fixed(quality.max_interarrival_gap_ms, 1) : "—"}
              unit={canShowLiveMetrics ? "ms" : undefined}
              tone={canShowLiveMetrics && Number(quality.max_interarrival_gap_ms ?? 0) > 100 ? "warning" : "neutral"}
              muted={!canShowLiveMetrics}
            />
          </div>
        </div>
        <LogConsole expanded />
      </section>
    </>
  );

  const DeepAnalyticsView = () => (
    <>
      {!hasAnalysisData ? (
        <section className="empty-state view-transition">
          <Wifi size={28} />
          <h2>No active stream.</h2>
          <p>Please connect via the Connection tab first.</p>
          <button className="primary-action" onClick={() => setActiveSidebarTab("connection")}>Go to Connection</button>
        </section>
      ) : (
        <>
          {actionableFindings.length > 0 ? (
            <div className={`deep-alert ${findingCounts.critical ? "critical" : "warning"}`}>
              <AlertTriangle size={15} />
              <strong>{findingsSummary}</strong>
            </div>
          ) : null}
          <AnalyticsTabs />
          <LogConsole />
        </>
      )}
    </>
  );

  const ReportManagerView = () => (
    <section className="utility-view view-transition">
      <div className="utility-card report-manager-card">
        <div className="utility-header">
          <div>
            <p className="eyebrow">Report Manager</p>
            <h2>Generated analysis artifacts</h2>
            <p>High-density artifact table for JSON and Markdown reports saved in the selected output directory.</p>
          </div>
          <FileText size={20} />
        </div>
        <ReportsPanel reportPaths={reportPaths} table onDelete={handleReportDeleted} />
      </div>
    </section>
  );

  const SettingsView = () => (
    <section className="utility-view view-transition">
      <div className="utility-card settings-card">
        <div className="utility-header">
          <div>
            <p className="eyebrow">Settings</p>
            <h2>Analyzer defaults</h2>
            <p>Persistable local configuration. The RTSP endpoint remains session-only.</p>
          </div>
          <Settings size={20} />
        </div>

        <div className="settings-grid">
          <Field label="Native C++ binary path">
            <input
              className="mono-input"
              value={form.binaryPath}
              onChange={(event) => updateForm("binaryPath", event.target.value)}
              disabled={isRunning}
            />
          </Field>
          <Field label="Default packet log limit">
            <input value={form.packetLogLimit} onChange={(e) => updateForm("packetLogLimit", e.target.value)} disabled={isRunning} />
          </Field>
          <Field label="Timeout threshold">
            <input value={form.timeoutMs} onChange={(e) => updateForm("timeoutMs", e.target.value)} disabled={isRunning} />
          </Field>
          <Field label="Frame count limit">
            <input value={form.frames} onChange={(e) => updateForm("frames", e.target.value)} disabled={isRunning} />
          </Field>
          <Field label="UI theme preference">
            <input value="Cyberpunk-Noir / Obsidian" readOnly />
          </Field>
          <label className="check-row settings-check">
            <input
              type="checkbox"
              checked={form.generateMarkdown}
              onChange={(e) => updateForm("generateMarkdown", e.target.checked)}
              disabled={isRunning}
            />
            Generate Markdown report by default
          </label>
        </div>
      </div>
    </section>
  );

  const renderWorkspace = () => {
    if (activeSidebarTab === "connection") return ConnectionTriageView();
    if (activeSidebarTab === "analytics") return DeepAnalyticsView();
    if (activeSidebarTab === "reports") return ReportManagerView();
    return SettingsView();
  };

  return (
    <div className={`desktop-shell ${activeSidebarTab === "connection" ? "with-master" : "no-master"}`}>
      <aside className="icon-dock">
        <div className="dock-brand" title="RTSP Stream Inspector">
          <Radio size={18} />
        </div>
        <nav>
          {sidebarItems.map((item) => (
            <NavIcon
              key={item.id}
              icon={item.icon}
              label={item.label}
              active={activeSidebarTab === item.id}
              onClick={() => setActiveSidebarTab(item.id)}
            />
          ))}
        </nav>
        <div className="daemon-status" title="C++ daemon status">
          <span className={isRunning ? "pulse-dot" : "static-dot"} />
          <small>C++</small>
        </div>
      </aside>

      {activeSidebarTab === "connection" ? MasterPanel() : null}

      <main className="analytics-panel">
        <header className="analytics-header">
          <div>
            <p className="eyebrow">Native C++ backend</p>
            <h2>{activeSidebarTab === "connection" ? "STREAM CONNECTION & TRIAGE" : activeSidebarTab === "analytics" ? "DEEP METRICS & DIAGNOSTICS" : activeSidebarTab === "reports" ? "GENERATED ARTIFACTS MANAGER" : "ANALYZER LOCAL CONFIGURATION"}</h2>
            <p>{report?.metadata?.generated_at_utc ?? lastLine ?? "Ready to inspect RTSP/RTP/H.264 streams."}</p>
          </div>
          <div className="header-actions">
            <div className="health-chip">
              <ShieldCheck size={15} />
              <span>Health</span>
              <strong>{canShowLiveMetrics ? `${healthScore}%` : "—"}</strong>
            </div>
            <ReportsPanel reportPaths={reportPaths} compact />
          </div>
        </header>

        {error ? (
          <div className="error-banner">
            <ShieldAlert size={16} />
            <div>
              <strong>Analysis error</strong>
              <p>{error}</p>
            </div>
          </div>
        ) : null}

        {renderWorkspace()}
      </main>
    </div>
  );
}

export default App;
