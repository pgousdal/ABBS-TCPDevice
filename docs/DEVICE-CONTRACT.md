# Device contract notes

M0.1 exposes a serial-shaped `IOExtSer` interface but implements only generic
Exec commands confirmed in NDK 3.2 R4 `exec/io.h`: `CMD_READ`, `CMD_WRITE`,
`CMD_CLEAR`, `CMD_FLUSH`, and `CMD_RESET`.

The inspected R4 `devices/serial.h` defines `IOExtSer` and the serial-specific
commands beginning at `CMD_NONSTD`. M0.1 does not implement, redefine, or guess
those command values. Unsupported requests receive `IOERR_NOCMD` as specified
by `exec/errors.h`.

The one non-production command, `ABBTCP_CMD_INJECT_RING`, lives at the documented
project-private base `CMD_NONSTD + 0x0100` so it does not collide with NDK serial
commands. Only `abbstcp-probe` uses it.

The key post-probe question remains: which generic and serial commands does ABBS
actually issue? Configure device `abbstcp.device`, unit `0`, observe the kernel
diagnostics, and use those observations to define M1. No networking belongs in
this milestone.
