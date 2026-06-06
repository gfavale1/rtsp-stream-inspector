# RTP Packet Structure

## 1. Overview

RTSP Stream Inspector parses RTP packets transported over RTSP/TCP interleaved framing. RTP packets carry H.264 payloads that are then passed to the H.264 NAL-level analyzer.

The primary supported and tested transport is RTP over RTSP/TCP interleaved.

## 2. RTSP interleaved framing

After RTSP `PLAY`, the server can multiplex RTP and RTCP frames over the same TCP connection used for RTSP control messages.

The interleaved frame header is 4 bytes:

- byte 0: `$` marker, `0x24`;
- byte 1: channel identifier;
- byte 2-3: 16-bit big-endian payload length.

Only the first byte is the magic marker. The remaining bytes identify the channel and payload length.

## 3. RTP channel mapping

The current TCP interleaved setup uses:

```text
Transport: RTP/AVP/TCP;unicast;interleaved=0-1
```

The implemented channel convention is:

- channel 0: RTP;
- channel 1: RTCP.

`client_port` is not used for TCP interleaved transport; it belongs to RTP/RTCP over UDP.

## 4. RTP fixed header

The RTP fixed header is 12 bytes before optional CSRC and extension data:

```text
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT      |       sequence number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           synchronization source identifier (SSRC)            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

The parser validates RTP version 2 and extracts the fixed header fields.

## 5. Fields parsed by the tool

The analyzer uses RTP fields for transport and stream metrics:

- version;
- marker bit;
- payload type;
- sequence number;
- timestamp;
- SSRC;
- payload bytes.

Optional CSRC and extension-related malformed cases are covered by deterministic tests to ensure truncated inputs fail safely.

## 6. Sequence tracking

`RtpStats` tracks sequence progression to estimate:

- packets received;
- packet loss;
- out-of-order packets;
- first and last sequence number;
- sequence wrap-around.

Sequence wrap-around is expected when the 16-bit RTP sequence number moves from `65535` to `0`.

## 7. Timestamp and jitter usage

RTP timestamps are used by the jitter estimator together with local packet arrival timing. The estimator computes an RFC 3550-style interarrival jitter value and exposes it in milliseconds.

The timestamp is also useful for correlating RTP transport behavior with H.264 payload cadence.

## 8. Payload forwarding to H.264 analyzer

After RTP header parsing, the payload bytes are forwarded to the H.264 analyzer. The H.264 layer inspects NAL-unit structure and updates counters for SPS, PPS, IDR, non-IDR, STAP-A, FU-A, and unknown NAL units.

The payload is not decoded into pixels.

## 9. RTCP channel note

RTCP data is carried on channel 1 in the current TCP interleaved configuration. RTCP frames are passed to `RtcpParser` and `RtcpStats`, not to the RTP parser.

See [protocol_flow.md](protocol_flow.md) and [metrics.md](metrics.md) for the full media-plane pipeline.

## 10. Current limitations

Current RTP limitations:

- RTP over UDP is future work;
- TCP interleaved transport is the primary supported/tested path;
- packet loss is estimated from observed sequence gaps;
- full H.264 FU-A reassembly is future work;
- the analyzer does not decode or display video frames.
