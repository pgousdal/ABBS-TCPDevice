/* Classic AmigaOS 3.x byte-stream device.  M0.1 intentionally has no network code. */
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/errors.h>
#include <devices/serial.h>
#include <proto/exec.h>
#include <proto/debug.h>

#include "abbstcp_device.h"
#include "device_internal.h"

struct ExecBase *SysBase;

static const char device_name[] = ABBTCP_DEVICE_NAME;
static const char id_string[] = "abbstcp.device 0.1 (10.8.2026)\r\n";

static void raw_string(const char *s)
{
    KPutStr((CONST_STRPTR)s);
}

static void raw_ulong(ULONG value)
{
    static const ULONG places[] = { 1000000000,100000000,10000000,1000000,100000,10000,1000,100,10,1 };
    ULONG place, digit;
    UWORD i;
    BOOL started = FALSE;
    for (i = 0; i < 10; ++i) {
        place = places[i];
        digit = 0;
        while (value >= place) { value -= place; ++digit; }
        if (digit || started || place == 1) { KPutChar((LONG)('0' + digit)); started = TRUE; }
    }
}

static const char *state_name(UWORD state)
{
    if (state == ABBTCP_RINGING) return "RINGING";
    if (state == ABBTCP_ONLINE) return "ONLINE";
    return "IDLE";
}

static void log_request(struct IOStdReq *io, ULONG requested,
                        const char *event, UWORD old_state, UWORD new_state)
{
    ULONG i, n;
    raw_string("ABBTCP unit=0 cmd="); raw_ulong(io->io_Command);
    raw_string(" flags="); raw_ulong(io->io_Flags);
    raw_string(" requested="); raw_ulong(requested);
    raw_string(" actual="); raw_ulong(io->io_Actual);
    if (io->io_Command == CMD_WRITE && io->io_Data) {
        n = requested;
        if (n == (ULONG)-1) {
            const UBYTE *p = (const UBYTE *)io->io_Data;
            n = 0; while (p[n]) ++n;
        }
        raw_string(" payload=\"");
        for (i = 0; i < n; ++i) {
            UBYTE c = ((const UBYTE *)io->io_Data)[i];
            if (c >= 32 && c < 127 && c != '\\' && c != '"') KPutChar(c);
            else { raw_string("\\x"); KPutChar("0123456789ABCDEF"[c >> 4]); KPutChar("0123456789ABCDEF"[c & 15]); }
        }
        KPutChar('"');
    }
    raw_string(" event="); raw_string(event);
    raw_string(" state="); raw_string(state_name(old_state));
    raw_string("->"); raw_string(state_name(new_state)); raw_string("\n");
}

static void finish(struct IOStdReq *io)
{
    if (!(io->io_Flags & IOF_QUICK)) ReplyMsg(&io->io_Message);
}

static void fifo_clear(struct AbbTcpUnit *u)
{
    u->fifo_head = 0; u->fifo_count = 0;
}

static void service_reads(struct AbbTcpUnit *u)
{
    struct IOStdReq *io;
    ULONG i, wanted;
    while (u->reads.mlh_Head->mln_Succ) {
        io = (struct IOStdReq *)u->reads.mlh_Head;
        wanted = io->io_Length;
        if (wanted > u->fifo_count) break;
        Remove(&io->io_Message.mn_Node);
        for (i = 0; i < wanted; ++i) {
            ((UBYTE *)io->io_Data)[i] = u->fifo[u->fifo_head];
            u->fifo_head = (UWORD)((u->fifo_head + 1) & (ABBTCP_FIFO_SIZE - 1));
        }
        u->fifo_count = (UWORD)(u->fifo_count - wanted);
        io->io_Actual = wanted; io->io_Error = 0;
        log_request(io, wanted, "complete", u->state, u->state);
        ReplyMsg(&io->io_Message);
    }
}

static BOOL fifo_put(struct AbbTcpUnit *u, const char *text)
{
    ULONG n = 0, i, tail;
    while (text[n]) ++n;
    if (n > ABBTCP_FIFO_SIZE - u->fifo_count) return FALSE;
    tail = (u->fifo_head + u->fifo_count) & (ABBTCP_FIFO_SIZE - 1);
    for (i = 0; i < n; ++i) { u->fifo[tail] = (UBYTE)text[i]; tail = (tail + 1) & (ABBTCP_FIFO_SIZE - 1); }
    u->fifo_count = (UWORD)(u->fifo_count + n);
    service_reads(u);
    return TRUE;
}

static BOOL same_line(const UBYTE *s, UWORD n, const char *literal)
{
    UWORD i = 0;
    while (literal[i]) { if (i >= n || s[i] != (UBYTE)literal[i]) return FALSE; ++i; }
    return i == n;
}

static BOOL process_line(struct AbbTcpUnit *u)
{
    if (same_line(u->line, u->line_len, "AT")) return fifo_put(u, "OK\r\n");
    if (same_line(u->line, u->line_len, "ATZ")) { u->state = ABBTCP_IDLE; return fifo_put(u, "OK\r\n"); }
    if (same_line(u->line, u->line_len, "ATA")) {
        if (u->state == ABBTCP_RINGING) { u->state = ABBTCP_ONLINE; return fifo_put(u, "CONNECT 38400\r\n"); }
        return fifo_put(u, "NO CARRIER\r\n");
    }
    if (same_line(u->line, u->line_len, "ATH")) { u->state = ABBTCP_IDLE; return fifo_put(u, "NO CARRIER\r\n"); }
    return fifo_put(u, "ERROR\r\n");
}

static BOOL consume_write(struct AbbTcpUnit *u, const UBYTE *p, ULONG n)
{
    ULONG i;
    for (i = 0; i < n; ++i) {
        UBYTE c = p[i];
        if (c == '\r' || c == '\n') {
            if (c == '\r' && u->line_len && !process_line(u)) return FALSE;
            u->line_len = 0;
        } else if (u->line_len < ABBTCP_LINE_SIZE) u->line[u->line_len++] = c;
        else return FALSE;
    }
    return TRUE;
}

static void abort_reads(struct AbbTcpUnit *u, struct IOStdReq *except)
{
    struct IOStdReq *io, *next;
    for (io = (struct IOStdReq *)u->reads.mlh_Head; io->io_Message.mn_Node.ln_Succ; io = next) {
        next = (struct IOStdReq *)io->io_Message.mn_Node.ln_Succ;
        if (io != except) { Remove(&io->io_Message.mn_Node); io->io_Error = IOERR_ABORTED; io->io_Actual = 0; ReplyMsg(&io->io_Message); }
    }
}

struct AbbTcpBase *device_init(struct AbbTcpBase *base, BPTR seg,
                               struct ExecBase *sys)
{
    SysBase = sys; base->sys_base = sys; base->seg_list = seg;
    base->device.dd_Library.lib_Node.ln_Type = NT_DEVICE;
    base->device.dd_Library.lib_Node.ln_Name = (char *)device_name;
    base->device.dd_Library.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
    base->device.dd_Library.lib_Version = ABBTCP_VERSION;
    base->device.dd_Library.lib_Revision = ABBTCP_REVISION;
    base->device.dd_Library.lib_IdString = (char *)id_string;
    base->unit0.reads.mlh_Head = (struct MinNode *)&base->unit0.reads.mlh_Tail;
    base->unit0.reads.mlh_Tail = 0;
    base->unit0.reads.mlh_TailPred = (struct MinNode *)&base->unit0.reads.mlh_Head;
    base->unit0.state = ABBTCP_IDLE;
    raw_string("ABBTCP init\n");
    return base;
}

BPTR device_expunge(struct AbbTcpBase *base);

LONG device_open(struct AbbTcpBase *base, struct IORequest *io, ULONG unit)
{
    SysBase = base->sys_base;
    if (unit != 0 || io->io_Message.mn_Length < sizeof(struct IOExtSer)) {
        io->io_Error = IOERR_OPENFAIL; return IOERR_OPENFAIL;
    }
    base->device.dd_Library.lib_OpenCnt++;
    base->unit0.unit.unit_OpenCnt++;
    base->device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
    io->io_Device = (struct Device *)base; io->io_Unit = (struct Unit *)&base->unit0;
    io->io_Error = 0; raw_string("ABBTCP open unit=0\n"); return 0;
}

BPTR device_close(struct AbbTcpBase *base, struct IORequest *io)
{
    SysBase = base->sys_base;
    Disable();
    if (base->device.dd_Library.lib_OpenCnt) base->device.dd_Library.lib_OpenCnt--;
    if (base->unit0.unit.unit_OpenCnt) base->unit0.unit.unit_OpenCnt--;
    if (!base->device.dd_Library.lib_OpenCnt) abort_reads(&base->unit0, 0);
    io->io_Device = 0; io->io_Unit = 0;
    Enable(); raw_string("ABBTCP close\n");
    if (!base->device.dd_Library.lib_OpenCnt && (base->device.dd_Library.lib_Flags & LIBF_DELEXP)) return device_expunge(base);
    return 0;
}

BPTR device_expunge(struct AbbTcpBase *base)
{
    BPTR seg; ULONG size; UWORD neg;
    SysBase = base->sys_base;
    if (base->device.dd_Library.lib_OpenCnt) { base->device.dd_Library.lib_Flags |= LIBF_DELEXP; return 0; }
    seg = base->seg_list; neg = base->device.dd_Library.lib_NegSize;
    size = (ULONG)neg + base->device.dd_Library.lib_PosSize;
    Remove(&base->device.dd_Library.lib_Node);
    FreeMem((UBYTE *)base - neg, size);
    return seg;
}

ULONG device_reserved(void) { return 0; }

void device_begin_io(struct AbbTcpBase *base, struct IOStdReq *io)
{
    struct AbbTcpUnit *u = &base->unit0;
    ULONG requested = io->io_Length, n;
    UWORD old_state;
    SysBase = base->sys_base; io->io_Error = 0; io->io_Actual = 0;
    Disable(); old_state = u->state;
    switch (io->io_Command) {
    case CMD_READ:
        if ((!io->io_Data && requested) || requested == (ULONG)-1 || requested > ABBTCP_FIFO_SIZE) io->io_Error = IOERR_BADLENGTH;
        else if (requested <= u->fifo_count) {
            for (n = 0; n < requested; ++n) { ((UBYTE *)io->io_Data)[n] = u->fifo[u->fifo_head]; u->fifo_head = (u->fifo_head + 1) & (ABBTCP_FIFO_SIZE - 1); }
            u->fifo_count -= (UWORD)requested; io->io_Actual = requested;
        } else {
            io->io_Flags &= ~IOF_QUICK; AddTail((struct List *)&u->reads, &io->io_Message.mn_Node);
            log_request(io, requested, "queued", old_state, u->state); Enable(); return;
        }
        break;
    case CMD_WRITE:
        n = requested;
        if (!io->io_Data) io->io_Error = IOERR_BADADDRESS;
        else {
            if (n == (ULONG)-1) { n = 0; while (((UBYTE *)io->io_Data)[n]) ++n; }
            if (!consume_write(u, (const UBYTE *)io->io_Data, n)) io->io_Error = IOERR_ABORTED;
            else io->io_Actual = n;
        }
        break;
    case CMD_CLEAR: fifo_clear(u); break;
    case CMD_FLUSH: abort_reads(u, io); break;
    case CMD_RESET: abort_reads(u, io); fifo_clear(u); u->line_len = 0; u->state = ABBTCP_IDLE; break;
    case ABBTCP_CMD_INJECT_RING:
        u->state = ABBTCP_RINGING;
        if (!fifo_put(u, "RING\r\n")) io->io_Error = IOERR_ABORTED;
        break;
    default: io->io_Error = IOERR_NOCMD; break;
    }
    log_request(io, requested, "complete", old_state, u->state);
    Enable(); finish(io);
}

LONG device_abort_io(struct AbbTcpBase *base, struct IOStdReq *target)
{
    struct AbbTcpUnit *u = &base->unit0;
    struct IOStdReq *io;
    SysBase = base->sys_base; Disable();
    for (io = (struct IOStdReq *)u->reads.mlh_Head; io->io_Message.mn_Node.ln_Succ; io = (struct IOStdReq *)io->io_Message.mn_Node.ln_Succ) {
        if (io == target) {
            Remove(&io->io_Message.mn_Node); io->io_Error = IOERR_ABORTED; io->io_Actual = 0;
            log_request(io, io->io_Length, "aborted", u->state, u->state);
            Enable(); ReplyMsg(&io->io_Message); return 0;
        }
    }
    Enable(); return IOERR_NOCMD;
}
