# Troubleshooting

## 1. Native analyzer binary not found

Likely cause:

- Electron is pointing to the wrong native binary path.
- The C++ backend has not been built yet.

What to check:

- run `./scripts/build.sh`;
- verify that `build/rtsp-inspector` exists;
- update the binary path in the Electron settings panel.

## 2. RTSP 401 Unauthorized

Likely cause:

- the camera requires authentication;
- the username or password is incorrect;
- credentials were omitted from the local RTSP URL.

What to check:

- verify credentials locally;
- use placeholders in documentation and commits only;
- confirm that the camera supports Basic Auth if using current analyzer support.

## 3. Camera requires Digest Auth

Digest Auth is not implemented yet. Cameras that require Digest Auth may fail during DESCRIBE or authentication retry.

Current workaround:

- test with a camera or local RTSP source that supports Basic Auth or no authentication;
- document Digest-only devices as unsupported for now.

## 4. DESCRIBE or PLAY timeout

Likely causes:

- wrong RTSP path;
- camera unreachable;
- firewall or NAT issue;
- camera is slow to respond;
- stream is not enabled.

What to check:

- increase `--timeout-ms`;
- verify the path in the vendor app or documentation;
- verify TCP connectivity to the RTSP port;
- test from the same network segment.

## 5. No RTCP observed

No RTCP may be normal for some devices or short captures.

What to check:

- increase frame count or capture duration;
- verify that the stream emits RTCP;
- compare with a local MediaMTX/FFmpeg test stream;
- remember that RTCP timing depends on server behavior.

The analyzer reports missing RTCP as a finding, but this does not automatically mean the stream is broken.

## 6. Missing SPS/PPS during short capture

Some streams send SPS/PPS only periodically or around keyframes.

What to check:

- increase capture duration;
- use a stream with more frequent IDR frames for testing;
- compare H.264 counters across multiple runs.

A missing SPS/PPS warning can be expected for very short captures.

## 7. Report not generated

Likely causes:

- output directory is not writable;
- analysis failed before report writing;
- invalid output path;
- native analyzer path is incorrect.

What to check:

- inspect live logs;
- verify output directory permissions;
- run the CLI manually with sanitized local values;
- check whether the process exited with an error.

## 8. Electron cannot open report path

Likely causes:

- the report file was deleted or moved;
- the path is not accessible from the current OS context;
- WSL/Windows path translation is involved.

What to check:

- use "show in folder" first;
- verify path existence;
- keep output directories on the same side of the OS boundary when possible.

## 9. WSL, firewall, or network routing issues

When using WSL with Windows-hosted RTSP test servers, `localhost` may refer to the WSL environment rather than the Windows host.

What to check:

- use the Windows vEthernet address from `ipconfig`;
- allow the RTSP server through Windows Firewall;
- test TCP connectivity with `nc -vz <host> 8554`;
- keep MediaMTX and FFmpeg running while analyzing.

UDP transport is future work and may be more sensitive to NAT/firewall behavior.

## 10. Do not commit credentials

Never commit:

- real RTSP URLs;
- camera usernames;
- camera passwords;
- Authorization headers;
- Basic Auth tokens;
- generated reports from private devices unless reviewed and sanitized.
