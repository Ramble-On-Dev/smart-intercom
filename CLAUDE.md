# Project: Smart Intercom (SII-001)

Single source of truth for assistants working in this repo.

## Tech stack

- **Target MCU**: Elecrow CrowPanel 10.1" ESP32-P4 (1024×600, 32 MB PSRAM, dual RISC-V)
- **SDK**: ESP-IDF v5.3.x (pinned)
- **UI**: LVGL 9.x — hand-coded (Squareline Studio is NOT used; archived under `docs/archive/squareline-scaffold/`)
- **Language**: C for firmware; C++17 permitted in UI/state-machine layers
- **RTOS**: FreeRTOS (bundled with ESP-IDF)
- **Hardware**: KiCad for IIB schematic and PCB
- **CAD**: Fusion 360 / OnShape for enclosures

## Layout

See `README.md`. The SDS in `docs/SII-001_SDS_Smart_Intercom_Rev0.1_DRAFT.docx` is authoritative for requirements; if it conflicts with code, treat the SDS as the spec.

## Conventions

- Many small files > few large files. Target 200–400 lines per file, hard cap 800.
- Immutable patterns where C allows; no mutation of pre-existing structs in place when a new copy will do.
- All firmware modules with non-trivial logic must have host-side Unity tests under `firmware/host_test/`. Coverage target: 80% on host-testable modules.
- State machine is explicit (`switch (state)` in one place), not nested switches across files.
- No hardcoded secrets. Camera credentials and WiFi creds go through NVS provisioning, never in source.

## Phase gates

- **Phase 1 (IIB hardware) is blocked** until SDS §9.1 P0 bench measurements are captured into `docs/bench-characterization.md`.
- **Phase 2 (host-testable firmware)** runs in parallel with the bench work — state machine, event queue, and Goertzel can be developed and tested without hardware.

## Reversibility

This is a rental install. Original IR-445SS remains untouched and functional. Any change that defeats the parallel-tap principle is a hard no.
