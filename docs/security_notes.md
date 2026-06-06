# Security Notes

## 1. Security model

RTSP Stream Inspector is a local inspection tool. It connects to a user-provided RTSP endpoint, performs protocol analysis, writes local reports, and displays results in an Electron UI.

It is not a vulnerability scanner, exploit framework, or replacement for a full network security audit. Its security value comes from focused stream diagnostics, redaction, findings, and parser robustness.

## 2. Threat assumptions

The tool assumes that:

- RTSP streams may be exposed by IoT/CCTV devices with weak transport security;
- RTSP URLs may contain inline credentials;
- Basic Auth may be used over plain RTSP;
- logs and reports may be shared or committed by mistake;
- parsers may receive malformed RTP, RTCP, H.264, SDP, or RTSP data;
- tests and CI must not depend on a live camera or real credentials.

## 3. Credential handling

RTSP URLs may contain inline userinfo. The backend must build request URIs without preserving `user:password@` in serialized output. Credentials are used only for authentication logic and must not appear in generated reports.

Documentation examples use placeholders only, such as:

```text
rtsp://user:password@camera.local:554/stream1
```

Do not commit real RTSP URLs or camera credentials.

## 4. Log redaction

Authorization headers are redacted before being printed or written to artifacts. The redacted form should expose the header name but not the credential material.

Redaction must be case-insensitive and robust across common authentication schemes. No raw token or response value should appear in logs.

## 5. Report sanitization

JSON and Markdown reports must not persist:

- username;
- password;
- raw authorization headers;
- Basic tokens;
- full RTSP URLs containing inline credentials.

Reports may include sanitized source fields such as host, port, path, control URI, transport, payload type, and codec metadata.

## 6. Electron persistence policy

Electron may persist non-sensitive settings, including:

- native analyzer binary path;
- output directory;
- timeout;
- frame count;
- packet log limit;
- Markdown generation preference.

Electron must not persist RTSP URLs or credentials. The RTSP endpoint is session-only.

## 7. Findings related to insecure transport

The analyzer emits findings for insecure conditions, including:

- unencrypted RTSP transport;
- Basic Auth over plain RTSP;
- missing or malformed RTCP observations;
- high jitter;
- large RTP inter-arrival gaps;
- packet loss and out-of-order packets;
- H.264 parameter set or FU-A consistency issues.

Basic Auth is Base64 encoding, not encryption. Plain RTSP can expose credentials and media metadata to network observers.

## 8. Malformed input robustness

The test suite includes deterministic malformed/fuzz-style parser tests. These tests use synthetic byte buffers and malformed strings to verify controlled failure behavior for:

- RTP packets;
- RTCP packets;
- H.264 payloads;
- SDP/RTSP parsing where testable;
- sanitization and report security behavior.

The goal is not full fuzzing coverage, but reproducible CI-friendly coverage for corrupted, truncated, inconsistent, or malformed inputs.

## 9. CI and test isolation

CI must not require:

- live cameras;
- network access to private devices;
- real usernames;
- real passwords;
- real RTSP URLs.

Parser and report tests use synthetic inputs only.

## 10. Current security limitations

Current limitations include:

- RTSPS/TLS is not implemented yet;
- Digest Auth is not implemented yet;
- RTP/RTCP over UDP is not the primary supported/tested transport;
- the tool is an inspector, not a full vulnerability scanner;
- it does not replace Wireshark or a complete security audit;
- it does not perform full video decoding or malware analysis of media content.

## 11. Future security improvements

Possible improvements:

- Digest Auth support;
- RTSPS/TLS support;
- UDP RTP/RTCP support;
- AddressSanitizer and UndefinedBehaviorSanitizer CI jobs;
- security grep CI check for accidental credentials;
- expanded malformed/fuzz-style test corpus;
- optional real fuzzing harness with libFuzzer/AFL-style tooling;
- stronger report schema versioning and validation.
