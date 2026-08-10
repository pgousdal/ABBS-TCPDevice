#!/bin/sh
set -eu
test -f src/resident.c
test -f src/device_entry.s
for symbol in DevInit DevOpen DevClose DevExpunge DevBeginIO DevAbortIO; do
    grep -q "$symbol" src/device_entry.s
done
for command in CMD_READ CMD_WRITE CMD_CLEAR CMD_FLUSH CMD_RESET IOERR_NOCMD; do
    grep -q "$command" src/abbstcp_device.c
done
grep -q 'OpenDevice' src/probe.c
grep -q 'ABBTCP_CMD_INJECT_RING' src/probe.c
if grep -R -E 'bsdsocket|socket\(|gethostby|telnet|DNS' src >/dev/null; then
    echo "network API unexpectedly present" >&2; exit 1
fi
echo "M0.1 source invariants passed (not an AmigaOS runtime test)"
