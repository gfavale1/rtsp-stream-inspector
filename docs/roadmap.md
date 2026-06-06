# Roadmap

This roadmap separates implemented capabilities from planned and optional work.

## 1. Implemented

- RTSP TCP interleaved support.
- RTSP `OPTIONS`, `DESCRIBE`, `SETUP`, `PLAY`, and `TEARDOWN`.
- Basic Auth support.
- SDP extraction for H.264 video track metadata.
- RTP parser and RTP statistics.
- RTP sequence wrap-around handling.
- RTCP parser and RTCP statistics.
- RTCP Sender Report, Receiver Report, SDES, BYE, and compound packet support.
- H.264 NAL-level inspection.
- SPS, PPS, SEI, IDR, non-IDR, STAP-A, FU-A, and unknown NAL counters.
- Jitter and inter-arrival timing metrics.
- Stream metrics: bitrate, packet rate, payload bitrate, and average packet sizes.
- Findings engine.
- JSON and Markdown reports.
- Electron dashboard UI.
- Live backend logs in Electron.
- Report open/show/copy actions.
- Catch2 tests.
- Deterministic malformed/fuzz-style tests.
- GitHub Actions C++ CI.
- GitHub Actions Electron CI.

## 2. Near-term

- Documentation polish and screenshots.
- Add `docs/assets/ui-dashboard.png` for README display.
- Security grep script or CI check for accidental credentials.
- AddressSanitizer and UndefinedBehaviorSanitizer CI jobs.
- RTCP display improvements in the Electron dashboard if needed.
- RTCP comparison and consistency improvements.
- More malformed/fuzz-style test cases for edge protocol inputs.

## 3. Mid-term

- Digest Auth support.
- H264RtpDepacketizer with FU-A reassembly.
- Lightweight probe mode improvements.
- Electron packaging.
- Report schema versioning.
- More structured report validation.
- Better vendor-specific RTSP compatibility documentation.

## 4. Long-term / optional

- RTP/RTCP over UDP.
- RTSPS/TLS support.
- Native Node-API or structured IPC alternative.
- Real fuzzing harness with libFuzzer/AFL-style tools.
- Historical report comparison.
- Advanced stream health scoring.
- Multi-stream comparison.
- Optional PCAP export or import workflows.

Optional items should not be presented as implemented until they exist in code and tests.
