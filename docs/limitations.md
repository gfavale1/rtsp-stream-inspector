# Limitations

This document records current project boundaries honestly. Implemented features should not be confused with planned work.

## 1. Transport limitations

The primary supported and tested media transport is RTP over RTSP/TCP interleaved.

The TCP interleaved transport header is:

```text
Transport: RTP/AVP/TCP;unicast;interleaved=0-1
```

RTP/RTCP over UDP is not currently a primary implemented path and should be treated as future work unless explicitly added and tested.

## 2. Authentication limitations

Basic Auth is supported.

Digest Auth is not implemented yet and should be documented as future work. Cameras that require Digest Auth may fail during the RTSP handshake.

## 3. Video limitations

The tool performs H.264 NAL-level inspection only. It does not:

- decode pixels;
- display video frames;
- act as a video player;
- act as an NVR;
- reconstruct complete H.264 frames from FU-A fragments unless a depacketizer is added later.

FU-A counters and consistency checks are implemented. Full FU-A reassembly is future work.

## 4. Tool scope

RTSP Stream Inspector is not:

- Wireshark;
- a full NVR;
- a complete vulnerability scanner;
- a camera management system;
- a video playback application.

It is a focused RTSP/RTP/RTCP/H.264 inspection and reporting tool.

## 5. Electron integration limitations

The current Electron integration uses a process + report file architecture:

```text
Electron -> child_process.spawn -> C++ analyzer -> JSON/Markdown reports -> Electron dashboard
```

This is intentional, simple, debuggable, and appropriate for the current project. Native Node-API integration or structured IPC may be explored later, but it is not required for the current design.

## 6. Environment limitations

Live behavior depends on camera and network conditions:

- camera vendors expose different RTSP paths;
- authentication schemes differ across vendors;
- some cameras may not emit RTCP regularly;
- some streams may omit SPS/PPS during short captures;
- WSL, NAT, firewall, or host routing may affect local testing;
- future UDP support may be more sensitive to OS/network configuration than TCP interleaving.

## 7. Security limitations

The tool warns about insecure transport and authentication patterns, but it does not replace a full security audit. It does not currently provide:

- RTSPS/TLS support;
- Digest Auth;
- credential rotation checks;
- device firmware analysis;
- exploit detection;
- network-wide discovery.
