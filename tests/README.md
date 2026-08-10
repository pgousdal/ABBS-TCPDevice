# M0 tests

Host-side checks can validate only source invariants and the tiny modem state
machine. They cannot validate an AmigaOS device ABI.

Run `make check` for those source-only invariants. The authoritative M0.1 test
is the documented FS-UAE/AmigaOS probe using the VBCC build against official
NDK 3.2 R4 headers.
