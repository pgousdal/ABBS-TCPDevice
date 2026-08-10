/*
 * ABBS-TCPDevice M0 source scaffold.
 *
 * This is not yet a loadable AmigaOS device. The compiler/NDK-specific
 * Exec device entry glue must be added using a real AmigaOS toolchain.
 * M0 intentionally contains no networking.
 */
#include "abbstcp_device.h"

static const char *modem_reply(const char *line, enum AbbTcpState *state)
{
    if (!line || !state) return "ERROR\r\n";

    if (line[0]=='A' && line[1]=='T' && line[2]=='\0')
        return "OK\r\n";

    if (line[0]=='A' && line[1]=='T' && line[2]=='Z' && line[3]=='\0') {
        *state = ABBTCP_IDLE;
        return "OK\r\n";
    }

    if (line[0]=='A' && line[1]=='T' && line[2]=='A' && line[3]=='\0') {
        if (*state == ABBTCP_RINGING) {
            *state = ABBTCP_ONLINE;
            return "CONNECT 38400\r\n";
        }
        return "NO CARRIER\r\n";
    }

    if (line[0]=='A' && line[1]=='T' && line[2]=='H' && line[3]=='\0') {
        *state = ABBTCP_IDLE;
        return "NO CARRIER\r\n";
    }

    return "ERROR\r\n";
}

/*
 * TODO using real AmigaOS NDK/toolchain:
 * DeviceInit / Open / Close / Expunge / BeginIO / AbortIO
 * queue CMD_READ
 * process CMD_WRITE
 * diagnostic ring buffer
 * synthetic RING injection
 *
 * Do not guess serial-specific numeric command values.
 */

const char *abbstcp_m0_modem_reply(const char *line, int *state)
{
    enum AbbTcpState s;
    const char *reply;
    if (!state) return "ERROR\r\n";
    s = (enum AbbTcpState)*state;
    reply = modem_reply(line, &s);
    *state = (int)s;
    return reply;
}
