# Device contract notes

M0 should implement only the minimum Exec/device surface required by the probe
and then by ABBS.

Candidate generic commands to support/log:

- CMD_READ
- CMD_WRITE
- CMD_CLEAR
- CMD_FLUSH
- CMD_RESET
- CMD_START
- CMD_STOP

Serial-specific commands must be added only after checking the AmigaOS NDK and
observing what ABBS actually sends.

## Hayes state machine

    IDLE -> RINGING -> ONLINE -> IDLE

- `AT` -> `OK`
- `ATZ` -> `OK`
- synthetic incoming call -> `RING`
- `ATA` while ringing -> `CONNECT 38400`
- `ATH` -> `NO CARRIER`

## Logging

Record at least unit, command number, flags, requested length, actual length,
payload, and state transitions.

The key M0 question is:

> Which Exec/serial commands does ABBS actually issue to a modem device?
