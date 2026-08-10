#ifndef ABBTCP_DEVICE_INTERNAL_H
#define ABBTCP_DEVICE_INTERNAL_H

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/devices.h>
#include <exec/lists.h>
#include <dos/dos.h>

#define ABBTCP_FIFO_SIZE 512
#define ABBTCP_LINE_SIZE 80

struct AbbTcpUnit {
    struct Unit unit;
    struct MinList reads;
    UBYTE fifo[ABBTCP_FIFO_SIZE];
    UWORD fifo_head, fifo_count;
    UBYTE line[ABBTCP_LINE_SIZE];
    UWORD line_len;
    UWORD state;
};

struct AbbTcpBase {
    struct Device device;
    BPTR seg_list;
    struct ExecBase *sys_base;
    struct AbbTcpUnit unit0;
};

#endif
