# Metrics Reference

## 1. Overview

RTSP Stream Inspector exposes explicit stream metrics rather than a single aggregate stability score. Metrics are derived from RTP, RTCP, H.264, and packet timing observations.

The report is designed to show what was observed and how it was interpreted, while keeping the raw protocol parsing inside the native C++ analyzer.

## 2. Metrics pipeline

The implemented metrics pipeline is:

```text
RtpStats + RtcpStats + H264Analyzer + JitterEstimator
    -> MetricsCollector
    -> AnalysisReport
    -> AnomalyDetector
    -> findings added to AnalysisReport
    -> JsonReportWriter / MarkdownReportWriter
```

`MetricsCollector` derives stream-level rates and averages. `AnomalyDetector` evaluates the completed report and appends structured findings.

## 3. RTP metrics

The RTP report section may include:

- packets received;
- packets lost;
- packet loss rate;
- out-of-order packets;
- first and last sequence number;
- sequence wrap-around handling;
- payload type;
- SSRC;
- RTP timestamps;
- total payload bytes.

Packet loss and out-of-order detection are based on RTP sequence progression and must be interpreted as capture-level estimates.

## 4. RTCP metrics

The RTCP report section may include:

- RTCP frames received;
- packets parsed;
- malformed packets;
- Sender Reports;
- Receiver Reports;
- SDES packets;
- BYE packets;
- application-defined packets;
- unknown packets;
- last sender SSRC;
- last RTP timestamp from Sender Report;
- last sender packet count;
- last sender octet count;
- last report-block jitter, fraction lost, and cumulative lost when available.

Some cameras may not emit RTCP during short captures. Missing RTCP is reported as a finding, but it can depend on camera behavior and capture duration.

## 5. H.264 metrics

The H.264 analyzer exposes counters for:

- SPS;
- PPS;
- SEI;
- IDR slices;
- non-IDR slices;
- STAP-A packets;
- FU-A packets;
- FU-A starts;
- FU-A ends;
- unknown NAL units.

These metrics describe stream structure. They do not imply video decoding.

## 6. Stream metrics

Stream metrics include derived rates and averages:

- capture duration;
- RTP bitrate;
- H.264 payload bitrate;
- RTP packets per second;
- average RTP packet size;
- average H.264 payload size.

These values are computed from observed capture data and may vary with frame count, camera encoding settings, network conditions, and capture duration.

## 7. RTP quality metrics

The RTP quality section includes timing-oriented metrics:

- packets observed by the jitter estimator;
- jitter in milliseconds;
- average inter-arrival gap;
- maximum inter-arrival gap.

The jitter estimator follows the RFC 3550-style interarrival jitter update:

```text
J = J + (|D(i-1,i)| - J) / 16
```

Here, `D(i-1,i)` is the difference between RTP timestamp spacing and packet arrival spacing for two consecutive packets, converted using the RTP clock rate.

## 8. Findings generated from metrics

Metric-derived findings may include:

- no packet loss;
- packet loss detected;
- no out-of-order packets;
- out-of-order packets detected;
- jitter within threshold;
- high jitter;
- no large inter-arrival gap;
- large inter-arrival gap;
- RTCP observed or missing;
- malformed RTCP packets;
- missing SPS/PPS;
- FU-A imbalance.

Consumers should use stable finding codes rather than relying on display order.

## 9. Current limitations

Current limitations:

- no aggregate stream-health score is exposed by the backend report contract;
- packet loss is estimated from observed RTP sequence numbers;
- jitter depends on RTP timestamp quality and arrival timing;
- short captures may miss periodic data such as SPS/PPS or RTCP Sender Reports;
- no UDP-specific metrics are currently part of the primary supported path.

## 10. Future work

Possible future metrics:

- explicit report schema version;
- RTCP-derived sender/receiver comparisons;
- historical report comparison;
- optional aggregate stream-health score;
- long-running stability summaries;
- per-window metrics over time.
