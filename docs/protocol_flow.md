# Protocol Flow

This document summarizes the implemented RTSP/RTP/RTCP/H.264 analysis flow.

## RTSP handshake

The analyzer connects to the RTSP server over TCP and performs:

1. `OPTIONS`
2. `DESCRIBE`
3. optional Basic Auth retry after `401 Unauthorized`
4. SDP parsing
5. `SETUP`
6. `PLAY`
7. media capture
8. `TEARDOWN`

## DESCRIBE and Basic Auth retry

If the server responds to `DESCRIBE` with `401 Unauthorized`, the analyzer can retry with Basic Auth when credentials are available from the RTSP URL. Authorization material is redacted from logs and reports.

## SDP extraction

The SDP response is used to extract:

- video track;
- H.264 payload type;
- RTP clock rate;
- control URI.

The selected control URI is used for `SETUP`.

## SETUP TCP interleaved

The analyzer requests RTP/RTCP over the RTSP TCP connection:

```text
Transport: RTP/AVP/TCP;unicast;interleaved=0-1
```

This uses channel 0 for RTP and channel 1 for RTCP.

## PLAY and interleaved media frames

After `PLAY`, the analyzer reads interleaved frames from the TCP connection. Each interleaved frame header is 4 bytes:

- `$` marker;
- channel identifier;
- 16-bit big-endian payload length.

RTP frames are parsed by `RtpParser`; RTCP frames are parsed by `RtcpParser`.

## H.264 inspection

The H.264 analyzer inspects RTP payload structure at NAL-unit level and counts SPS, PPS, SEI, IDR, non-IDR, STAP-A, FU-A, FU-A starts/ends, and unknown NAL units.

It does not decode video frames.

## TEARDOWN

The analyzer attempts RTSP `TEARDOWN` at the end of the capture. The result is recorded in reports and findings.

## Limitations

- UDP RTP/RTCP transport is future work.
- Digest Auth is future work.
- RTSPS/TLS is future work.
- Full H.264 FU-A reassembly is future work.
