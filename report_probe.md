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

- RTP frames received: 946
- RTCP frames received: 54
- Total interleaved payload bytes: 1046296

## RTP Statistics

- Packets received: 946
- Packets lost estimated: 0
- Out-of-order packets: 0
- Loss rate: 0.000
- Total RTP bytes: 1043920
- Total RTP payload bytes: 1032568
- First sequence number: 58686
- Last sequence number: 59631
- Last RTP timestamp: 2577690
- Payload type: 96
- SSRC: 1612688084

## H.264 NAL Statistics

- NAL units seen: 946
- SPS: 15
- PPS: 15
- SEI: 0
- IDR slices: 299
- Non-IDR slices: 617
- STAP-A packets: 0
- FU-A packets: 713
- FU-A starts: 218
- FU-A ends: 217
- Unknown NAL units: 0

## Stream Metrics

- Capture duration: 28.677 s
- RTP bitrate: 0.291 Mbps
- H264 payload bitrate: 0.288 Mbps
- RTP packets/sec: 32.989 pps
- Average RTP packet size: 1103.510 bytes
- Average H264 payload size: 1091.510 bytes

## RTP Quality Metrics

- Packets observed for jitter: 946
- RTP jitter: 1.610 ms
- Average inter-arrival gap: 30.279 ms
- Max inter-arrival gap: 524.134 ms

## Findings

- [OK] No RTP packet loss detected.
- [OK] No out-of-order RTP packets detected.
- [OK] No unknown H.264 NAL units detected.
- [OK] SPS and PPS NAL units were observed.
- [WARN] FU-A fragmentation start/end counters are not balanced. The capture may have started or stopped in the middle of a fragmented frame.
- [OK] RTP jitter is within the basic threshold.
- [WARN] A large RTP inter-arrival gap was detected.
- [OK] RTSP TEARDOWN completed successfully.
- [WARN] RTSP traffic is not encrypted; credentials and media metadata may be exposed on the network.
- [WARN] If Basic authentication is used over plain RTSP, credentials are only Base64-encoded and not encrypted.

## RTSP Session Cleanup

- TEARDOWN success: yes
