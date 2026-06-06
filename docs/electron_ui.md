# Electron UI

## Overview

The Electron UI provides a desktop-first dashboard for running the native analyzer and inspecting generated reports. It does not parse RTP, RTCP, or H.264 bytes directly. The JSON report produced by the C++ backend drives the dashboard.

## Layout

The UI is organized around:

- a sidebar for navigation;
- a connection/triage workflow;
- analytics panels for RTP, H.264, quality metrics, and findings;
- a reports panel;
- settings for non-sensitive configuration;
- a live log console.

## Start/Stop lifecycle

When analysis starts, Electron launches the native analyzer with `child_process.spawn`. Logs are streamed into the UI while the process runs. When the analysis completes, Electron reads the JSON report and updates the dashboard.

The Stop action requests termination of the analyzer process through the Electron main process.

## Report actions

The UI supports:

- opening report files;
- showing report files in the folder;
- copying report paths;
- displaying timestamped JSON and Markdown report paths.

## Binary and output configuration

Users can select or configure:

- native C++ analyzer path;
- output directory;
- timeout;
- frame count;
- packet log limit;
- Markdown report generation.

These are non-sensitive settings and may be persisted.

## RTSP URL persistence policy

The RTSP URL is intentionally not persisted because it may contain credentials. Users must re-enter it for each session.

## Report-driven dashboard

Electron consumes the JSON report contract documented in [report_format.md](report_format.md). Missing optional fields should be handled defensively to avoid UI crashes when report structure evolves.
