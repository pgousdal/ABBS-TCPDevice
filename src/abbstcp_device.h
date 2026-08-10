#ifndef ABBSTCP_DEVICE_H
#define ABBSTCP_DEVICE_H

#include <exec/io.h>

#define ABBTCP_DEVICE_NAME "abbstcp.device"
#define ABBTCP_VERSION 0
#define ABBTCP_REVISION 1

/*
 * Private M0 test command.  CMD_NONSTD is the NDK-defined start of the
 * old-style third-party command space.  This is deliberately not part of the
 * production modem contract and must not be used by ABBS.
 */
#define ABBTCP_PRIVATE_BASE (CMD_NONSTD + 0x0100)
#define ABBTCP_CMD_INJECT_RING (ABBTCP_PRIVATE_BASE + 0)

enum AbbTcpState {
    ABBTCP_IDLE = 0,
    ABBTCP_RINGING = 1,
    ABBTCP_ONLINE = 2
};

#endif
