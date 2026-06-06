# RTSP Handshake

## 1. Overview

This document describes the implemented RTSP control-plane flow used by RTSP Stream Inspector. The analyzer communicates with the camera over TCP, negotiates the stream, requests TCP interleaved transport, captures RTP/RTCP frames, and then attempts TEARDOWN.

Implemented commands:

- `OPTIONS`;
- `DESCRIBE`;
- `SETUP`;
- `PLAY`;
- `TEARDOWN`.

## 2. URL parsing and sanitization

The analyzer accepts an RTSP URL from the CLI or Electron UI. URLs may contain inline credentials, for example:

```text
rtsp://user:password@camera.local:554/stream1
```

Credentials are sensitive. Request URIs and generated reports must not preserve `user:password@` userinfo. Documentation examples use placeholders only.

## 3. TCP connection

The analyzer opens a TCP connection to the RTSP server, usually on port 554 unless another port is specified in the URL.

The primary supported media transport is RTP/RTCP over RTSP/TCP interleaved.

## 4. OPTIONS

`OPTIONS` is used to check basic RTSP server responsiveness and supported methods. A normal response is `200 OK`.

## 5. DESCRIBE

`DESCRIBE` requests the SDP description for the stream. The SDP body is later used to identify the video media track and H.264 payload information.

## 6. 401 Unauthorized and Basic Auth retry

If `DESCRIBE` returns `401 Unauthorized`, the analyzer can retry using Basic Auth when credentials are available from the RTSP URL.

Authorization headers are redacted before they appear in logs or reports. Basic Auth is Base64 encoding, not encryption. When used over plain RTSP, credentials can be exposed to network observers.

Digest Auth is not implemented yet and is documented as future work.

## 7. SDP extraction

The SDP parser extracts:

- video track;
- H.264 payload type;
- RTP clock rate;
- control URI.

The selected control URI is used to build the `SETUP` request.

## 8. SETUP with TCP interleaved transport

The analyzer requests RTP/RTCP over the RTSP TCP connection with:

```text
Transport: RTP/AVP/TCP;unicast;interleaved=0-1
```

This maps:

- channel 0 to RTP;
- channel 1 to RTCP.

`client_port` is not used for TCP interleaved transport. It belongs to RTP/RTCP over UDP.

## 9. PLAY

`PLAY` starts media delivery. After a successful response, the server begins sending interleaved RTP/RTCP frames over the same TCP connection.

## 10. Interleaved media capture

After `PLAY`, the analyzer reads interleaved frame headers and dispatches payloads according to channel:

- channel 0: `RtpParser`, `RtpStats`, `JitterEstimator`, and `H264Analyzer`;
- channel 1: `RtcpParser` and `RtcpStats`.

## 11. TEARDOWN

At the end of analysis, the analyzer attempts RTSP `TEARDOWN`. The result is recorded in the report and findings.

## 12. Security considerations

Important security behavior:

- Authorization headers are redacted;
- reports must not contain raw credentials;
- Electron does not persist RTSP URLs or credentials;
- unencrypted RTSP is reported as a security finding;
- Basic Auth over plain RTSP is reported as a warning.

## 13. Current limitations

Current RTSP limitations:

- Digest Auth is future work;
- RTSPS/TLS is future work;
- RTP/RTCP over UDP is future work;
- camera-specific path discovery is not automatic;
- live behavior depends on vendor firmware and configuration.

## 14. Future work

Possible future improvements:

- Digest Auth;
- RTSPS/TLS;
- UDP transport;
- more vendor compatibility notes;
- optional lightweight discovery/probe helpers;
- more structured RTSP response diagnostics.
