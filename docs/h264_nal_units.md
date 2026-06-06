# H.264 NAL Unit Inspection

## 1. Overview

RTSP Stream Inspector inspects H.264 stream structure at NAL-unit level. The analyzer reads RTP payloads and classifies the H.264 NAL units that appear in the stream.

The tool does not decode pixels, reconstruct complete video frames, or display video. Its purpose is protocol and stream-structure inspection, not playback.

## 2. Current implementation scope

The current implementation tracks:

- SPS;
- PPS;
- SEI;
- IDR slices;
- non-IDR slices;
- STAP-A aggregation packets;
- FU-A fragmentation units;
- FU-A start and end counters;
- unknown or unsupported NAL-unit types.

FU-A start/end counters and consistency checks are implemented. Full FU-A frame reassembly is future work.

## 3. NAL unit header

For single NAL units, the first payload byte contains the H.264 NAL header:

```text
+---+----+-----+
| F | NRI| Type|
+---+----+-----+
 1b  2b   5b
```

The analyzer uses the low 5 bits to identify the NAL-unit type.

## 4. NAL unit types tracked by the tool

| NAL type | Meaning | Tracked |
|---:|---|---|
| 1 | Non-IDR slice | yes |
| 5 | IDR slice | yes |
| 6 | SEI | yes |
| 7 | SPS | yes |
| 8 | PPS | yes |
| 24 | STAP-A aggregation packet | yes |
| 28 | FU-A fragmentation unit | yes |

Other NAL-unit types may be counted as unknown when they are not part of the current inspection scope.

## 5. Single NAL units

Single NAL units are classified directly from the first payload byte. This covers common packets such as SPS, PPS, IDR slices, non-IDR slices, and SEI messages.

The analyzer increments counters only from structure-level observation. It does not parse the full H.264 bitstream syntax.

## 6. STAP-A aggregation packets

STAP-A packets allow multiple small NAL units to be aggregated in a single RTP payload. The current implementation detects STAP-A packets and counts them as part of the H.264 inspection output.

Malformed or truncated STAP-A payloads are covered by deterministic malformed tests to ensure they do not cause out-of-bounds reads or crashes.

## 7. FU-A fragmentation units

FU-A is used to fragment larger H.264 NAL units across multiple RTP packets. The analyzer detects FU-A packets and tracks:

- total FU-A packet count;
- FU-A start count;
- FU-A end count.

If start and end counters are imbalanced, the findings pipeline can report a fragmentation consistency warning. This can happen when the capture starts or stops in the middle of a fragmented frame, or when packets are lost.

The current implementation does not reassemble FU-A fragments into complete H.264 frames.

## 8. Counters exposed by the report

The JSON report exposes H.264 counters under the `h264` object. Typical fields include:

- `sps`;
- `pps`;
- `sei`;
- `idr_slices`;
- `non_idr_slices`;
- `stap_a_packets`;
- `fu_a_packets`;
- `fu_a_starts`;
- `fu_a_ends`;
- `unknown_nal_units`.

See [report_format.md](report_format.md) for the complete report contract.

## 9. Findings related to H.264

H.264-related findings may include:

- SPS/PPS observed;
- missing SPS/PPS during the capture;
- no unknown H.264 NAL units;
- unknown H.264 NAL units observed;
- FU-A start/end counters balanced;
- FU-A start/end imbalance.

Findings should be interpreted with capture duration in mind. Short captures may miss SPS/PPS depending on camera behavior and GOP structure.

## 10. Current limitations

The current H.264 inspection layer does not:

- decode pixels;
- display video;
- reconstruct complete video frames;
- fully reassemble FU-A fragments;
- validate the full H.264 bitstream grammar.

## 11. Future work

Possible future improvements:

- H.264 RTP depacketizer with FU-A reassembly;
- more detailed SPS/PPS parsing;
- GOP-level summary metrics;
- optional frame-boundary reconstruction;
- additional tests for vendor-specific H.264 packetization behavior.
