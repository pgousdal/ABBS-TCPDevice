# ABBS-TCPDevice — M0.1

`abbstcp.device` is a classic AmigaOS 68k byte-stream device used to discover
the device contract expected by ABBS. M0.1 contains a loadable resident device
and an `OpenDevice()` probe. It deliberately contains no TCP/IP, sockets,
Telnet, DNS, or filesystem access during normal device operation.

## Toolchain and target

The canonical build uses the current stable releases of VBCC, vlink and
vasmm68k_mot, the VBCC `m68k-amigaos` target, and the official AmigaOS 3.2 NDK
Release 4. Code is built for a plain 68000 and uses only APIs available in
AmigaOS 3.0/3.1+. The device has no C startup and neither artifact uses ixemul,
clib2, or PosixLib.

Set `VBCC` as required by the VBCC installation and point `NDK32R4` at the
unpacked `NDK3.2` directory containing `Include_H`:

```sh
export VBCC=/opt/vbcc
export NDK32R4=/opt/NDK3.2
make clean all
```

Override `VC`, `VASM`, or `NDK_INC` only for an equivalent installation. The
outputs are `abbstcp.device` and `abbstcp-probe`.

## FS-UAE validation

The authoritative M0.1 test is a real AmigaOS run. From an Amiga shell in the
shared build directory:

```text
copy abbstcp.device DEVS:
copy abbstcp-probe C:
abbstcp-probe
echo $RC
```

The probe must print `M0.1 probe passed` and `$RC` must be `0`. A successful
host link alone is not runtime acceptance.

Only after that succeeds, make the observational ABBS configuration:

```text
Device: abbstcp.device
Unit:   0
```

This test only observes diagnostics. Do not configure networking for M0.1.
