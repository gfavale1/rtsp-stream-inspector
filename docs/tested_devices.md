# Tested Devices

## 1. Purpose

This document records manual compatibility notes for RTSP Stream Inspector. It should describe behavior and compatibility without storing sensitive device information.

## 2. Data handling policy

Do not store:

- real camera IP addresses;
- real RTSP URLs;
- usernames;
- passwords;
- Authorization headers;
- Basic Auth tokens;
- screenshots containing credentials.

Use sanitized device names and generic notes.

## 3. Compatibility table

| Device | Firmware | Transport | Auth | Video | RTCP | Status | Notes |
|---|---|---|---|---|---|---|---|
| TP-Link Tapo C200 | not specified | TCP interleaved | Basic | H.264 | observed / not always present | tested manually | Use sanitized local paths only |
| MediaMTX synthetic stream | N/A | TCP interleaved | none | H.264 | depends on stream/server behavior | tested manually | Useful for deterministic local demos |

The table should stay high-level. Keep private network details outside the repository.

## 4. Example sanitized entry

A safe entry describes behavior without private values:

```text
Device: Example IP camera
Transport: RTP over RTSP/TCP interleaved
Auth: Basic
Video: H.264
RTCP: observed during longer captures
Status: tested manually
Notes: vendor-specific RTSP path, sanitized locally
```

## 5. Notes for adding devices

When adding a device:

- use a generic device name;
- avoid private IPs and serial numbers;
- do not include account names;
- do not include real RTSP URLs;
- mention whether RTCP was observed;
- mention whether authentication was Basic, none, or unsupported;
- mention whether testing was manual or automated.

## 6. What not to store

Do not store generated reports from real devices unless they have been reviewed and sanitized. Do not include screenshots that reveal local hostnames, private IPs, or credentials.
