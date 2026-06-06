# Testing

## 1. Overview

RTSP Stream Inspector uses deterministic tests to validate protocol parsing, metrics, findings, report generation, and build behavior. Tests are designed to run in CI without a live camera, private network, or real credentials.

## 2. Test philosophy

The test suite prioritizes:

- deterministic inputs;
- synthetic byte buffers;
- no live network dependencies;
- no real credentials;
- controlled parser failure behavior;
- report sanitization checks;
- CI reproducibility.

Malformed inputs may throw controlled exceptions or return error states, depending on the parser. The important property is that malformed input must not cause a crash or out-of-bounds access.

## 3. Unit testing with Catch2

C++ tests use Catch2 and are executed through `ctest`.

Common local workflow:

```bash
./scripts/build.sh
ctest --test-dir build --output-on-failure
```

Generic CMake alternative:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 4. Parser tests

Parser tests cover valid and invalid protocol data for:

- RTP packet parsing;
- RTP sequence tracking and wrap-around;
- RTCP packet parsing;
- RTCP compound packets;
- H.264 NAL-unit inspection;
- SDP/RTSP parsing where functions are isolated and testable.

These tests use synthetic byte vectors and strings, not live camera traffic.

## 5. Malformed/fuzz-style tests

The project includes deterministic malformed/fuzz-style tests. These are not a replacement for a real fuzzing engine, but they provide CI-friendly robustness coverage.

Examples include:

- empty RTP packets;
- RTP packets shorter than the minimum header;
- unsupported RTP versions;
- inconsistent RTP CSRC counts;
- RTP extension headers with insufficient data;
- RTCP packets shorter than the header;
- unsupported RTCP versions;
- RTCP length fields larger than the available buffer;
- truncated RTCP Sender Reports and Receiver Reports;
- truncated RTCP compound packets;
- empty H.264 payloads;
- incomplete FU-A and STAP-A payloads;
- unknown NAL-unit types;
- malformed SDP/RTSP strings where testable;
- sanitizer edge cases.

The expected behavior is controlled failure or safe no-op handling, not undefined behavior.

## 6. Report writer tests

Report writer tests verify that JSON and Markdown reports include expected sections and do not serialize sensitive data. Important report sections include:

- metadata;
- configuration;
- source;
- video;
- interleaved;
- RTP;
- RTCP;
- H.264;
- stream metrics;
- RTP quality;
- findings;
- TEARDOWN status.

## 7. AnomalyDetector tests

AnomalyDetector tests verify findings for:

- packet loss;
- out-of-order packets;
- unknown H.264 NAL units;
- missing SPS/PPS;
- FU-A start/end imbalance;
- high jitter;
- large inter-arrival gaps;
- RTCP observed or missing;
- RTCP Sender Report observed or missing;
- malformed RTCP packets;
- RTSP TEARDOWN status;
- insecure RTSP transport and Basic Auth over plain RTSP warnings.

Tests should search findings by stable codes or invariant properties rather than relying on display order.

## 8. Electron build validation

The Electron app is validated through its build command:

```bash
cd electron-app
npm install
npm run build
```

This verifies that the React/Vite frontend compiles and that report model changes do not break the dashboard.

## 9. CI strategy

CI should validate:

- C++ Debug and Release builds where configured;
- Catch2 tests through `ctest`;
- Electron build;
- no dependency on live cameras;
- no dependency on credentials.

Additional future CI jobs may include sanitizer builds and security grep checks.

## 10. Running tests locally

Recommended full local check:

```bash
rm -rf build
./scripts/build.sh
ctest --test-dir build --output-on-failure
cd electron-app
npm install
npm run build
```

When running from WSL on a Windows-mounted filesystem, build tools may report clock skew warnings. If tests pass, this is usually an environment timestamp issue rather than a source issue.

## 11. What is intentionally not tested in CI

CI intentionally does not test:

- private camera access;
- real credentials;
- vendor-specific RTSP URLs;
- live RTSP network behavior;
- local firewall/NAT behavior;
- Electron packaging installers;
- long-running soak tests.

Live RTSP testing is manual and must use local/private credentials only. Real URLs or credentials must not be committed.
