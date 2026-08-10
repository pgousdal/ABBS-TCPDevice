        section code,code

        xref    _device_init
        xref    _device_open
        xref    _device_close
        xref    _device_expunge
        xref    _device_reserved
        xref    _device_begin_io
        xref    _device_abort_io

        xdef    _DevInit
        xdef    _DevOpen
        xdef    _DevClose
        xdef    _DevExpunge
        xdef    _DevReserved
        xdef    _DevBeginIO
        xdef    _DevAbortIO

; Exec register ABI wrappers -> normal vbcc stack ABI.
_DevInit:
        move.l  a6,-(sp)
        move.l  a0,-(sp)
        move.l  d0,-(sp)
        jsr     _device_init
        lea     12(sp),sp
        rts
_DevOpen:
        move.l  d0,-(sp)
        move.l  a1,-(sp)
        move.l  a6,-(sp)
        jsr     _device_open
        lea     12(sp),sp
        rts
_DevClose:
        move.l  a1,-(sp)
        move.l  a6,-(sp)
        jsr     _device_close
        addq.l  #8,sp
        rts
_DevExpunge:
        move.l  a6,-(sp)
        jsr     _device_expunge
        addq.l  #4,sp
        rts
_DevReserved:
        jmp     _device_reserved
_DevBeginIO:
        move.l  a1,-(sp)
        move.l  a6,-(sp)
        jsr     _device_begin_io
        addq.l  #8,sp
        rts
_DevAbortIO:
        move.l  a1,-(sp)
        move.l  a6,-(sp)
        jsr     _device_abort_io
        addq.l  #8,sp
        rts
