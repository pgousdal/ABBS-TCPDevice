# M0 build scaffold.
# Select a validated AmigaOS 68k compiler and NDK before enabling binary targets.

.PHONY: help clean

help:
	@echo "ABBS-TCPDevice M0: select AmigaOS 68k toolchain/NDK first"

clean:
	rm -f src/*.o abbstcp.device abbstcp-probe
