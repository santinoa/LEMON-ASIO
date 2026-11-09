# LEMON ASIO architecture — target USB microphone + USB headphones

## Intended signal path

FL Studio (ASIO host)
  -> LEMON ASIO 64-bit driver interface
  -> LEMON duplex engine
       -> WASAPI capture client: selected USB microphone
       -> clock-drift correction / asynchronous sample-rate conversion
       -> ASIO buffer exchange and host callbacks
       -> WASAPI render client: selected USB headphones

## Critical engineering constraint

The USB microphone and USB headphones are separate audio devices. Their hardware clocks are not guaranteed to run at exactly the same rate. A duplex engine must monitor the capture/render queue relationship and compensate for drift (typically with adaptive resampling or a carefully controlled rate-matching strategy). Simply opening both streams at 48 kHz does not synchronize their physical clocks.

## Current milestone (0.1.1)

The application enumerates active Windows capture and render endpoints independently and reads their shared-mode format and device-period metadata. It does not open a streaming client and does not expose an ASIO driver to FL Studio.

## Next milestones

- 0.2: stable full-duplex WASAPI shared-mode stream, with device selection and clean shutdown/recovery.
- 0.3: ASIO host interface, buffers, sample-position/time callbacks, channel metadata and control panel integration.
- 0.4: settings persistence, installer/registration and diagnostics/logging.
- 0.5: measured latency, dropout, clock-drift and long-session tests on the target PC.
