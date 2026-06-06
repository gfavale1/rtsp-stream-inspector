# TP-Link Tapo C200 Setup Notes

## 1. Purpose

This document records safe local testing notes for TP-Link Tapo C200-style RTSP streams. It is a local-device aid, not a guarantee for all Tapo firmware versions or configurations.

## 2. Privacy and credential warning

Do not commit:

- real usernames;
- real passwords;
- private camera IP addresses;
- real RTSP URLs;
- Authorization headers;
- Basic Auth tokens.

Use placeholders in documentation and issue reports.

## 3. Local testing assumptions

A local Tapo camera may expose RTSP paths after RTSP access is enabled in the vendor application. Exact setup steps and stream paths can vary by firmware version and account configuration.

The analyzer currently focuses on RTSP over TCP interleaved transport.

## 4. Example sanitized RTSP URL

Use placeholder examples only:

```text
rtsp://user:password@camera.local:554/stream1
rtsp://user:password@camera.local:554/stream2
```

Replace these locally during private testing. Do not commit the replaced values.

## 5. Expected stream behavior

A typical local Tapo RTSP stream may provide:

- H.264 video;
- Basic Auth;
- RTSP DESCRIBE/SETUP/PLAY flow;
- RTP over RTSP/TCP interleaved when requested;
- RTCP depending on camera behavior and capture duration.

Some captures may not include SPS/PPS or RTCP packets if the capture is too short.

## 6. Known limitations

- Device-specific RTSP paths may differ.
- Digest Auth is not currently implemented.
- UDP transport is future work.
- Vendor firmware changes may affect stream behavior.
- This document does not cover cloud access, account security, or vendor app internals.

## 7. Troubleshooting

If the analyzer cannot connect:

- verify that RTSP is enabled for the camera;
- verify that the stream path is correct;
- verify that the camera and host are on the same network;
- check firewall rules;
- increase `--timeout-ms`;
- try a longer capture if RTCP or SPS/PPS is missing;
- confirm whether the camera requires Digest Auth, which is future work.

See [troubleshooting.md](troubleshooting.md) for general troubleshooting.

## 8. What not to commit

Never commit:

- local RTSP URLs;
- screenshots with real endpoints;
- camera usernames or passwords;
- Authorization headers;
- generated reports that include private environment details.
