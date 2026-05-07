# Smart Intercom & Door Camera Integration (SII-001)

Monorepo for the Smart Intercom project: ESP-IDF firmware for an Elecrow CrowPanel 10.1" ESP32-P4 head unit, a custom Intercom Interface Board (IIB) that tap-couples to a Lee Dan IR-445SS suite station, and two RTSP IP cameras.

See `docs/SII-001_SDS_Smart_Intercom_Rev0.1_DRAFT.docx` for the full Software Design Specification.

## Layout

```
firmware/      ESP-IDF v5.3.x project (target: esp32p4) + LVGL 9.x UI
hardware/      KiCad project for the IIB (Phase 1 — populated after bench characterization)
enclosures/    Fusion 360 / OnShape exports for head-unit bezel and IIB enclosure
docs/          SDS, bench-characterization worksheet, decision records
tools/         Host-side utilities (scope-CSV parsers, calibration helpers)
.github/       CI workflows
```

## Status

Phase 0 — repo bootstrap complete. Bench characterization (SDS §9.1) pending.

## Build

### Firmware (ESP-IDF)

```
cd firmware
idf.py set-target esp32p4
idf.py build
```

### Host unit tests

```
cmake -S firmware/host_test -B firmware/host_test/build
cmake --build firmware/host_test/build
ctest --test-dir firmware/host_test/build --output-on-failure
```

## Phase Map

| Phase | Status | Notes |
|-------|--------|-------|
| 0 — Bootstrap + bench | In progress | Repo done; bench measurements outstanding |
| 1 — IIB hardware | Blocked on Phase 0 | KiCad schematic + PCB |
| 2 — FW foundation (host-testable) | Ready to start | State machine, event queue, Goertzel |
| 3 — Network & video | Pending | Dual RTSP, MJPEG/H.264 |
| 4 — UI | Pending | Hand-coded LVGL per SDS §6 |
| 5 — Audio pipeline | Pending | I²S in/out, half-duplex, optional AEC |
| 6 — Integration | Pending | Bench rig + live calibration |
| 7 — Enclosures + install | Pending | 3D print + reversible install |
| 8 — Hardening + optional | Pending | OTA, MQTT/Home Assistant |
