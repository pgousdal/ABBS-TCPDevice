# Canonical M0.1 toolchain: current stable VBCC m68k-amigaos target, vlink,
# vasmm68k_mot, and the official AmigaOS 3.2 NDK R4 Include_H directory.
VC       ?= vc
VASM     ?= vasmm68k_mot
NDK_INC  ?= $(NDK32R4)/Include_H
NDK_LIB  ?= $(NDK32R4)/lib

CFLAGS   = +aos68k -c99 -cpu=68000 -O2 -I$(NDK_INC) -Isrc -c
ASFLAGS  = -m68000 -Fhunk -quiet
DEVICE_OBJS = build/resident.o build/device_entry.o build/abbstcp_device.o

.PHONY: all clean check-toolchain check
all: check-toolchain abbstcp.device abbstcp-probe

check-toolchain:
	@test -n "$(NDK32R4)" || { echo "Set NDK32R4 to the official NDK 3.2 R4 directory"; exit 1; }
	@test -f "$(NDK_INC)/devices/serial.h" || { echo "Missing $(NDK_INC)/devices/serial.h"; exit 1; }
	@command -v $(VC) >/dev/null || { echo "Missing VBCC driver: $(VC)"; exit 1; }
	@command -v $(VASM) >/dev/null || { echo "Missing assembler: $(VASM)"; exit 1; }
	@command -v vlink >/dev/null || { echo "Missing linker: vlink"; exit 1; }

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(VC) $(CFLAGS) -o $@ $<

build/device_entry.o: src/device_entry.s | build
	$(VASM) $(ASFLAGS) -o $@ $<

# No C startup and no POSIX compatibility library are linked into the device.
abbstcp.device: $(DEVICE_OBJS)
	$(VC) +aos68k -nostdlib -L$(NDK_LIB) -o $@ $(DEVICE_OBJS) -lamiga -ldebug

# The normal VBCC Amiga startup/vclib are used; neither requires ixemul.
abbstcp-probe: build/probe.o
	$(VC) +aos68k -o $@ build/probe.o

check:
	sh tests/source-check.sh

clean:
	rm -f build/*.o abbstcp.device abbstcp-probe
