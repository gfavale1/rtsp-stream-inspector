# Report Format

## 1. Overview

The JSON report is the stable machine-readable contract between the native C++ backend and the Electron UI. The C++ analyzer produces the report after stream analysis. Electron reads the JSON file and renders dashboard panels for source information, video metadata, RTP/RTCP statistics, H.264 counters, quality metrics, findings, and report actions.

The Markdown report is a human-readable companion artifact generated from the same `AnalysisReport` data.

## 2. Report generation lifecycle

1. Electron starts the analyzer with CLI arguments.
2. The analyzer performs RTSP control-plane negotiation.
3. RTP/RTCP interleaved frames are captured and parsed.
4. RTP, RTCP, H.264, jitter, and stream metrics are collected.
5. `AnalysisReport` is populated.
6. `AnomalyDetector` appends findings.
7. `JsonReportWriter` writes the JSON report.
8. `MarkdownReportWriter` optionally writes the Markdown report.
9. Electron reads the JSON report and updates the dashboard.

## 3. JSON root objects

The report contains the following root objects:

- `metadata`
- `configuration`
- `source`
- `video`
- `interleaved`
- `rtp`
- `rtcp`
- `h264`
- `stream_metrics`
- `rtp_quality`
- `findings`
- `teardown_success`

## 4. Field descriptions

### `metadata`

Represents report-level metadata such as generation time and tool identity. Produced by the backend and consumed by Electron for status/header information.

### `configuration`

Represents analysis configuration such as frame count, timeout, packet log limit, and report options. Produced by the backend from CLI arguments. Electron may display selected values in settings or report views.

### `source`

Represents sanitized RTSP source information. It must not contain username/password userinfo or raw authorization data. Produced by URL parsing and RTSP setup logic. Electron may display host, port, path, or transport information.

### `video`

Represents SDP-derived video metadata such as codec, payload type, RTP clock rate, and control URI. Produced by SDP parsing. Electron displays these values in the stream identity area.

### `interleaved`

Represents RTP/RTCP interleaved frame counters and channel-level information. Produced by `InterleavedFrameReader` and `StreamAnalyzer`. Electron may use this for transport summary panels.

### `rtp`

Represents parsed RTP statistics: packets received, packet loss estimation, out-of-order packets, payload type, SSRC, timestamps, sequence number boundaries, and payload bytes. Produced by `RtpParser` and `RtpStats`. Electron displays this in RTP panels.

### `rtcp`

Represents parsed RTCP statistics: frames received, packets parsed, malformed packets, Sender Reports, Receiver Reports, SDES packets, BYE packets, application-defined packets, unknown packets, and selected last report values. Produced by `RtcpParser` and `RtcpStats`. Electron may display it inside RTP/RTCP diagnostics.

### `h264`

Represents H.264 NAL-level counters: SPS, PPS, SEI, IDR, non-IDR, STAP-A, FU-A, FU-A start/end, and unknown NAL units. Produced by `H264Analyzer`. Electron displays this in the H.264 inspection panel.

### `stream_metrics`

Represents derived stream metrics such as capture duration, RTP bitrate, H.264 payload bitrate, packets per second, average RTP packet size, and average H.264 payload size. Produced by `MetricsCollector`. Electron displays these in metrics panels.

### `rtp_quality`

Represents timing-oriented metrics such as jitter, average inter-arrival gap, maximum inter-arrival gap, and packets observed. Produced by `JitterEstimator`. Electron displays these in quality metrics panels.

### `findings`

Represents structured findings emitted by `AnomalyDetector`. Electron displays findings in triage and analytics views.

### `teardown_success`

Boolean indicating whether RTSP TEARDOWN completed successfully. Produced by RTSP session shutdown logic. Electron may display it as part of the analysis status.

## 5. Findings model

Each finding uses a compact structured model:

```json
{
  "severity": "warning",
  "code": "unencrypted_rtsp",
  "message": "RTSP traffic is not encrypted; credentials and media metadata may be exposed on the network."
}
```

Expected severity values are typically:

- `ok`
- `warning`
- `critical`

Consumers should not depend on finding order. They should filter or group by `severity` and stable `code` values.

## 6. Sanitization guarantees

Reports must never include:

- username;
- password;
- raw authorization header;
- Basic token;
- full RTSP URL with inline credentials.

The report may include sanitized source information such as host, port, stream path, transport, and a sanitized URI without userinfo.

## 7. Example sanitized JSON skeleton

```json
{
  "metadata": {
    "tool": "rtsp-stream-inspector",
    "generated_at_utc": "2026-01-01T00:00:00Z"
  },
  "configuration": {
    "frames": 1000,
    "timeout_ms": 5000,
    "packet_log_limit": 20,
    "markdown_enabled": true
  },
  "source": {
    "host": "camera.local",
    "port": 554,
    "path": "/stream1",
    "sanitized_url": "rtsp://camera.local:554/stream1",
    "transport": "RTP/AVP/TCP;unicast;interleaved=0-1"
  },
  "video": {
    "codec": "H264",
    "payload_type": 96,
    "rtp_clock_rate": 90000,
    "control_uri": "trackID=1"
  },
  "interleaved": {
    "rtp_frames_received": 1000,
    "rtcp_frames_received": 12
  },
  "rtp": {
    "packets_received": 1000,
    "packets_lost": 0,
    "out_of_order_packets": 0,
    "payload_type": 96,
    "ssrc": 123456,
    "first_sequence_number": 100,
    "last_sequence_number": 1099,
    "total_payload_bytes": 524288
  },
  "rtcp": {
    "frames_received": 12,
    "packets_parsed": 12,
    "malformed_packets": 0,
    "sender_reports": 4,
    "receiver_reports": 0,
    "source_description_packets": 4,
    "bye_packets": 0,
    "app_packets": 0,
    "unknown_packets": 0,
    "last_sender_ssrc": 123456,
    "last_rtp_timestamp_from_sr": 987654,
    "last_sender_packet_count": 1000,
    "last_sender_octet_count": 524288,
    "last_report_block_jitter": null,
    "last_fraction_lost": null,
    "last_cumulative_lost": null
  },
  "h264": {
    "sps": 1,
    "pps": 1,
    "sei": 0,
    "idr_slices": 8,
    "non_idr_slices": 240,
    "stap_a_packets": 0,
    "fu_a_packets": 42,
    "fu_a_starts": 21,
    "fu_a_ends": 21,
    "unknown_nal_units": 0
  },
  "stream_metrics": {
    "capture_duration_seconds": 10.0,
    "rtp_bitrate_mbps": 4.2,
    "h264_payload_bitrate_mbps": 4.0,
    "rtp_packets_per_second": 100.0,
    "average_rtp_packet_size": 650.0,
    "average_h264_payload_size": 620.0
  },
  "rtp_quality": {
    "packets_observed": 1000,
    "jitter_ms": 1.2,
    "average_interarrival_gap_ms": 10.0,
    "max_interarrival_gap_ms": 38.0
  },
  "findings": [
    {
      "severity": "ok",
      "code": "no_packet_loss",
      "message": "No RTP packet loss detected."
    }
  ],
  "teardown_success": true
}
```

The skeleton uses placeholders and documentation-only values. It must not be copied with real camera credentials.

## 8. Markdown report

The Markdown report is generated from the same internal analysis data. It is intended for human review and should include sections for RTSP/session details, RTP statistics, RTCP statistics, H.264 counters, stream metrics, RTP quality metrics, findings, and TEARDOWN status.

Like the JSON report, Markdown output must not contain credentials, raw authorization headers, or Basic tokens.

## 9. Versioning considerations

The report format is currently project-local. Future improvements may add an explicit schema version under `metadata` so Electron can handle old and new reports more safely.

Recommended compatibility rules:

- adding a new root object is allowed;
- adding fields to an existing object is allowed;
- removing or renaming fields should be avoided without schema versioning;
- Electron should tolerate missing optional fields;
- findings should be matched by stable `code`, not display text.


## 10. Sanitized sample reports

Sanitized examples are available in:

- [examples/sample_report.json](examples/sample_report.json)
- [examples/sample_report.md](examples/sample_report.md)

These files use documentation-only hostnames and reserved IP ranges. They must not be replaced with real camera credentials or private RTSP URLs.
