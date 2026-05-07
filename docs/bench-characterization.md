# Bench Characterization Worksheet (SDS §9.1, §A)

Required before Phase 1 (IIB schematic capture) starts. Capture every measurement; do not estimate.

**Date of capture:** ____________________
**Operator:** ____________________
**Test equipment:** ____________________ (scope model, multimeter model, signal source if any)

---

## A.1 Continuity Test — Wire-Color → Lee Dan Signal

Procedure:
1. Disconnect 5-conductor cable at the wire-nut junction. Mark each wire with tape & label first.
2. Multimeter on continuity mode.
3. Probe pairs while pressing each button in turn. Note which wires close on which button.
4. The wire that connects to multiple buttons in some combination is **Audio Common**.
5. The wire that does NOT close on any button is likely the **X (tone)** line.

| Wire color | Connects to button(s) | Lee Dan signal | Notes |
|------------|-----------------------|----------------|-------|
| Red        |                       |                |       |
| Yellow     |                       |                |       |
| Black      |                       |                |       |
| Blue       |                       |                |       |
| Green      |                       |                |       |
| Door pair A | Door                 | Door contact   |       |
| Door pair B | Door                 | Door contact   |       |

---

## A.2 X / Tone-In Line — Live Buzz Capture

Setup: Channel 1 on suspected X line; ground clip on suspected Audio Common. Trigger: rising edge ~0.5 V, single-shot. Timebase: 100 ms/div initially; zoom in once a buzz is captured.

Have a helper press the lobby panel buzzer for the apartment.

| Measurement | Value | Notes |
|-------------|-------|-------|
| Idle voltage (DC) |  | Should be near 0 V |
| Active waveform Vpp |  |  |
| Fundamental frequency (FFT) |  | Hz |
| Modulation pattern | steady / warble | If warble: rate ___ Hz |
| Single-buzz duration |  | seconds |
| Inter-buzz gap (if repeated) |  |  |
| Common-mode voltage relative to safety ground |  | Critical for isolation choice |

**Saved capture file:** `docs/scope-captures/x-line-buzz.csv`

---

## A.3 Audio-to-Station — Lobby → Suite Voice Path

Have a helper speak at the lobby panel (normal conversational level). Probe the suspected Audio-to-Station line vs Audio Common.

**Note**: Per SDS §3.3.1, signal is only present at the station while the Listen button is pressed. Hold Listen during capture.

| Measurement | Value | Notes |
|-------------|-------|-------|
| Peak voltage during normal speech |  | Vp |
| Source impedance (estimated) |  | Ω |
| Audible bandwidth (rough) |  | Hz–Hz |
| Noise floor when Listen held but lobby silent |  | mVrms |

**Saved capture file:** `docs/scope-captures/audio-to-station.csv`

---

## A.4 Audio-from-Station — Suite → Lobby Voice Path

Press Talk on the IR-445SS and speak into the FEB300 transducer. Probe the suspected Audio-from-Station line vs Audio Common.

| Measurement | Value | Notes |
|-------------|-------|-------|
| Peak voltage during normal speech |  | Vp |
| Voltage required for intelligible audio at lobby (inject 1 kHz reference) |  |  |
| Noise floor with Talk held but silent |  |  |

**Saved capture file:** `docs/scope-captures/audio-from-station.csv`

---

## A.5 Door Button — Contact Verification

The IIB door relay is sized based on what voltage exists across the door-button contact pair. Per SDS §3.3.2, this should be a dry contact.

| Measurement | Value | Notes |
|-------------|-------|-------|
| Idle voltage across open contact (AC) |  | Must be ~0 V for "dry contact" classification |
| Idle voltage across open contact (DC) |  |  |
| Voltage across contact with Door pressed (sanity) |  | Effectively short |
| Confirmed dry contact? | yes / no | If no: note wetting voltage and update relay spec (see Risk R7) |

---

## A.6 Open Decisions Resolved by These Measurements

After this worksheet is filled in, the following SDS §9.5 decisions can be locked:

- [ ] Final tone-detection frequency (`TONE_FREQ_HZ` in `tone_detect.c`)
- [ ] Mechanical relays vs analog switches for button-parallel switching (depends on audio-line voltage levels)
- [ ] Onboard ESP32-P4 codec vs external codec on IIB (depends on audio peak voltages and SNR requirements)
- [ ] Door relay spec (default 5 A / 250 VAC unless A.5 shows lower)

## Sign-off

Phase 1 (IIB schematic capture) is unblocked when this document is fully populated and reviewed.

| Reviewer | Date | Notes |
|----------|------|-------|
|          |      |       |
