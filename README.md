# RTSP Stream Inspector

[![C++ CI](https://github.com/gfavale1/rtsp-stream-inspector/actions/workflows/ci.yml/badge.svg)](https://github.com/gfavale1/rtsp-stream-inspector/actions/workflows/ci.yml)
[![Electron CI](https://github.com/gfavale1/rtsp-stream-inspector/actions/workflows/electron-ci.yml/badge.svg)](https://github.com/gfavale1/rtsp-stream-inspector/actions/workflows/electron-ci.yml)

RTSP Stream Inspector is a native C++20 and Electron desktop tool for inspecting RTSP/RTP/RTCP/H.264 camera streams without decoding video frames. It focuses on protocol visibility, stream metrics, security findings, and structured reporting for IP camera and CCTV diagnostics.

The project combines a native analyzer with a React/Electron desktop UI. The backend inspects RTSP control-plane traffic, RTP/RTCP interleaved media frames, and H.264 NAL-unit structure, then produces sanitized JSON and Markdown reports consumed by the desktop dashboard.

## Screenshots

Screenshots can be added under `docs/assets/` as:

- `ui-dashboard.png`
- `ui-h264.png`
- `ui-quality.png`
- `ui-reports.png`

See [docs/assets/README.md](docs/assets/README.md) for screenshot handling rules. Do not include real RTSP URLs, credentials, private camera IPs, or tokens in screenshots.

## Why this project exists

RTSP cameras often expose streams through weak or unencrypted configurations. General-purpose tools such as Wireshark are powerful, but broad and not optimized for a focused camera-stream inspection workflow.

RTSP Stream Inspector provides a narrower layer for RTSP/RTP/RTCP/H.264 streams: it extracts stream structure, computes metrics, highlights security and quality findings, and writes reports that are easy to inspect from a desktop UI or from the command line.

## Features

### RTSP control plane

- `OPTIONS`, `DESCRIBE`, `SETUP`, `PLAY`, and `TEARDOWN`.
- Credential-aware RTSP URL parsing.
- Request URI generation without inline credentials.
- `401 Unauthorized` handling with Basic Auth retry.
- Session ID and CSeq handling.
- SDP extraction for video track, H.264 payload type, RTP clock rate, and control URI.
- TCP interleaved setup with `Transport: RTP/AVP/TCP;unicast;interleaved=0-1`.

### RTP analysis

- RTP over RTSP/TCP interleaved media frames.
- RTP channel 0 handling.
- RTP parser and aggregate statistics.
- Packet count, packet loss estimation, and out-of-order detection.
- Sequence number wrap-around handling.
- Payload type, SSRC, RTP timestamps, bitrate, and packet rate.

### RTCP analysis

- RTCP channel 1 handling.
- RTCP header parsing.
- Sender Reports, Receiver Reports, SDES, and BYE packets.
- Compound RTCP packet parsing.
- Malformed RTCP packet counters.
- Sender packet/octet counters and report-block fields where available.

### H.264 inspection

- NAL-level inspection without pixel decoding.
- SPS, PPS, SEI, IDR, and non-IDR counters.
- STAP-A and FU-A detection.
- FU-A start/end counters and consistency checks.
- Unknown NAL-unit counters.

### Quality metrics

- RFC 3550-style interarrival jitter estimate.
- Jitter in milliseconds.
- Average and maximum RTP inter-arrival gaps.
- Capture duration, RTP bitrate, and H.264 payload bitrate.
- Packets per second.
- Average RTP packet size and average H.264 payload size.

### Security and quality findings

- Unencrypted RTSP warning.
- Basic Auth over plain RTSP warning.
- Packet loss and out-of-order packet findings.
- Missing SPS/PPS and unknown H.264 NAL-unit findings.
- FU-A start/end imbalance finding.
- High jitter and large inter-arrival gap findings.
- RTCP observed/missing, Sender Report observed/missing, and malformed RTCP findings.
- RTSP TEARDOWN success/failure finding.

### Reports

- Sanitized JSON report used as the backend/frontend contract.
- Human-readable Markdown report.
- Timestamped report paths from the Electron UI.
- No usernames, passwords, raw Authorization headers, or Basic tokens in reports.
- Sanitized examples in [docs/examples/sample_report.json](docs/examples/sample_report.json) and [docs/examples/sample_report.md](docs/examples/sample_report.md).

### Electron UI

- React/Vite desktop dashboard.
- Start and stop analysis controls.
- Live backend logs.
- RTSP URL, timeout, frame count, and packet log limit configuration.
- Native analyzer binary path and output directory selection.
- RTP, RTCP, H.264, and quality metrics panels.
- Findings and reports panels.
- Open report, show report in folder, and copy report path actions.
- Non-sensitive settings persistence. RTSP URLs and credentials are intentionally not persisted.

### Testing and CI

- Catch2 unit tests executed with `ctest`.
- RTP, RTCP, H.264, jitter, stream metrics, anomaly detector, and report writer tests.
- Deterministic malformed/fuzz-style parser tests with synthetic byte buffers.
- Sanitization/security regression tests.
- GitHub Actions C++ CI and Electron CI.
- CI does not require live cameras or real credentials.

## Architecture summary

The desktop architecture keeps a simple and explicit boundary between the UI and the native analyzer:

```text
Electron / React UI
  -> Electron main process
  -> child_process.spawn
  -> native C++ analyzer
  -> RTSP/RTP/RTCP/H.264 inspection
  -> JSON/Markdown reports
  -> Electron dashboard
```

Electron launches the native analyzer as a controlled child process through `child_process.spawn`. The native process writes structured reports; the Electron side reads the JSON report and renders the dashboard.

See [docs/architecture.md](docs/architecture.md) for the complete architecture and protocol pipeline.

## Quick start

### Requirements

- C++20 compiler.
- CMake.
- Node.js and npm for the Electron frontend.
- A local/private RTSP camera or local RTSP test stream for live testing.

### Build the C++ backend

```bash
./scripts/build.sh
```

If the build script is not available, use CMake directly:

```bash
cmake -S . -B build
cmake --build build
```

### Run the CLI analyzer

Use sanitized placeholders in examples and documentation:

```bash
./build/rtsp-inspector analyze \
  --url "rtsp://user:password@camera.local:554/stream1" \
  --frames 1000 \
  --timeout-ms 5000 \
  --packet-log-limit 20 \
  --output report.json \
  --markdown report.md
```

Do not commit real RTSP URLs, camera usernames, passwords, or Authorization headers.

### Run the Electron UI

```bash
cd electron-app
npm install
npm run dev
```

### Build the Electron frontend

```bash
cd electron-app
npm run build
```

## Reports

The JSON report is the contract between the C++ backend and the Electron UI. The Markdown report is intended for human review and sharing. Both outputs are sanitized and must not contain credentials or raw Authorization headers.

See [docs/report_format.md](docs/report_format.md) for the report schema and examples.

## Testing

Build and run the C++ tests:

```bash
./scripts/build.sh
ctest --test-dir build --output-on-failure
```

Validate the Electron build:

```bash
cd electron-app
npm run build
```

The test suite is deterministic, uses synthetic byte buffers, and does not require a live camera. Live RTSP testing is manual and must use local/private credentials only.

See [docs/testing.md](docs/testing.md) for the full testing strategy.

## Documentation

| Document | Description |
|---|---|
| [docs/README.md](docs/README.md) | Documentation index |
| [docs/architecture.md](docs/architecture.md) | System architecture, process boundary, protocol pipeline, reports |
| [docs/protocol_flow.md](docs/protocol_flow.md) | RTSP handshake, SDP extraction, TCP interleaved RTP/RTCP flow |
| [docs/report_format.md](docs/report_format.md) | JSON and Markdown report contract |
| [docs/examples/sample_report.json](docs/examples/sample_report.json) | Sanitized JSON report example |
| [docs/examples/sample_report.md](docs/examples/sample_report.md) | Sanitized Markdown report example |
| [docs/security_notes.md](docs/security_notes.md) | Credential handling, redaction, and security model |
| [docs/testing.md](docs/testing.md) | Catch2, malformed tests, CI, and local validation |
| [docs/troubleshooting.md](docs/troubleshooting.md) | Common setup and runtime issues |
| [docs/limitations.md](docs/limitations.md) | Current implementation limits and scope boundaries |
| [docs/roadmap.md](docs/roadmap.md) | Implemented, near-term, mid-term, and optional future work |
| [docs/electron_ui.md](docs/electron_ui.md) | Electron UI runtime model and report-driven dashboard |
| [docs/rtsp_handshake.md](docs/rtsp_handshake.md) | RTSP control-plane reference |
| [docs/rtp_packet_structure.md](docs/rtp_packet_structure.md) | RTP and TCP interleaved framing reference |
| [docs/h264_nal_units.md](docs/h264_nal_units.md) | H.264 NAL-unit inspection reference |
| [docs/metrics.md](docs/metrics.md) | Stream metrics and quality metric definitions |
| [docs/tested_devices.md](docs/tested_devices.md) | Manual tested-device notes |
| [docs/tapo_c200_setup.md](docs/tapo_c200_setup.md) | TP-Link Tapo C200 setup notes, if used locally |
| [docs/assets/README.md](docs/assets/README.md) | Screenshot and documentation asset rules |

## Security notes

- Authorization headers are redacted.
- JSON and Markdown reports must not contain usernames, passwords, raw Authorization headers, or Basic tokens.
- Electron persists only non-sensitive settings; RTSP URLs and credentials are not persisted.
- Basic Auth is Base64 encoding, not encryption.
- Plain RTSP can expose credentials and stream metadata to network observers.
- The tool warns about unencrypted RTSP and Basic Auth over plain RTSP.

See [docs/security_notes.md](docs/security_notes.md) for the complete security model.

