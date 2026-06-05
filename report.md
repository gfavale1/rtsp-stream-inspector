# RTSP Stream Inspection Report

## Source

- Host: 192.168.0.242
- Port: 554
- Path: /stream2
- Transport: rtp_interleaved_tcp

## Video Track

- Codec: H264
- Payload type: 96
- Clock rate: 90000
- Control: track1

## Interleaved Capture

- RTP frames received: 940
- RTCP frames received: 60
- Total interleaved payload bytes: 906224

## RTP Statistics

- Packets received: 940
- Packets lost estimated: 0
- Out-of-order packets: 0
- Loss rate: 0.000
- Total RTP bytes: 903584
- Total RTP payload bytes: 892304
- First sequence number: 57689
- Last sequence number: 58628
- Last RTP timestamp: 2869200
- Payload type: 96
- SSRC: 477236326

## H.264 NAL Statistics

- NAL units seen: 940
- SPS: 16
- PPS: 16
- SEI: 0
- IDR slices: 432
- Non-IDR slices: 476
- STAP-A packets: 0
- FU-A packets: 464
- FU-A starts: 32
- FU-A ends: 32
- Unknown NAL units: 0

## Stream Metrics

- Capture duration: 31.914 s
- RTP bitrate: 0.227 Mbps
- H264 payload bitrate: 0.224 Mbps
- RTP packets/sec: 29.454 pps
- Average RTP packet size: 961.260 bytes
- Average H264 payload size: 949.260 bytes

## Findings

- [OK] No RTP packet loss detected.
- [OK] No out-of-order RTP packets detected.
- [OK] No unknown H.264 NAL units detected.
- [OK] SPS and PPS NAL units were observed.
- [OK] Observed FU-A fragmentation start/end counters are balanced.
- [OK] RTSP TEARDOWN completed successfully.
- [WARN] RTSP traffic is not encrypted; credentials and media metadata may be exposed on the network.
- [WARN] If Basic authentication is used over plain RTSP, credentials are only Base64-encoded and not encrypted.

## RTSP Session Cleanup

- TEARDOWN success: yes
