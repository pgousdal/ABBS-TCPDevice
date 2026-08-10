# ABBS-TCPDevice — M0

Experimental classic AmigaOS device intended to look like a serial/modem device
to ABBS while eventually using TCP/IP underneath.

M0 is deliberately **not** a TCP implementation. Its purpose is to prove the
device contract and observe what ABBS actually asks of a serial-like device.

## M0 goals

- Native AmigaOS device name: `abbstcp.device`
- Open device unit 0
- Log device commands
- Tiny Hayes parser for `AT`, `ATZ`, `ATA`, `ATH`
- Synthetic replies: `OK`, `RING`, `CONNECT 38400`, `NO CARRIER`
- Standalone probe harness
- No sockets yet

## Important limitation

This is a source delivery, not a claim of a working Amiga binary. A real
AmigaOS NDK/toolchain and FS-UAE runtime test are part of M0 validation.
