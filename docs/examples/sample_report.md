# Sample RTSP Stream Inspector Report

This is a sanitized documentation example. It does not contain real camera credentials, Authorization headers, Basic tokens, or private RTSP URLs.

## Source Summary

- Host: `camera.local`
- Documentation IP: `192.0.2.10`
- Port: `554`
- Path: `/stream1`
- Sanitized URL: `rtsp://camera.local:554/stream1`
- Transport: `RTP/AVP/TCP;unicast;interleaved=0-1`

## Video Summary

- Codec: H.264
- Payload type: 96
- RTP clock rate: 90000
- Control URI: `trackID=1`

## RTP Statistics

- Packets received: 1000
- Packets lost: 0
- Out-of-order packets: 0
- SSRC: 123456
- Total payload bytes: 524288

## RTCP Statistics

- RTCP frames received: 12
- RTCP packets parsed: 12
- Malformed RTCP packets: 0
- Sender Reports: 4
- Receiver Reports: 0
- SDES packets: 4
- BYE packets: 0

## H.264 Counters

- SPS: 1
- PPS: 1
- SEI: 0
- IDR slices: 8
- Non-IDR slices: 240
- FU-A packets: 42
- FU-A starts: 21
- FU-A ends: 21
- Unknown NAL units: 0

## Stream Metrics

- Capture duration: 10.0 s
- RTP bitrate: 4.2 Mbps
- H.264 payload bitrate: 4.0 Mbps
- Packets per second: 100.0
- Average RTP packet size: 650.0 bytes
- Average H.264 payload size: 620.0 bytes

## Quality Metrics

- Jitter: 1.2 ms
- Average inter-arrival gap: 10.0 ms
- Maximum inter-arrival gap: 38.0 ms

## Findings

- OK: No RTP packet loss detected.
- OK: RTCP traffic was observed.
- WARNING: RTSP traffic is not encrypted; credentials and media metadata may be exposed on the network.

## TEARDOWN Status

- TEARDOWN successful: yes
