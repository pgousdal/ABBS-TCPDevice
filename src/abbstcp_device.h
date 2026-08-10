#ifndef ABBSTCP_DEVICE_H
#define ABBSTCP_DEVICE_H

enum AbbTcpState {
    ABBTCP_IDLE = 0,
    ABBTCP_RINGING = 1,
    ABBTCP_ONLINE = 2
};

#define ABBTCP_DEVICE_NAME "abbstcp.device"
#define ABBTCP_VERSION 0
#define ABBTCP_REVISION 1

#endif
