#include <exec/types.h>
#include <exec/io.h>
#include <devices/serial.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>
#include "abbstcp_device.h"

static int do_io(struct IOExtSer *s, UWORD command, APTR data, ULONG length)
{
    s->IOSer.io_Command = command; s->IOSer.io_Data = data;
    s->IOSer.io_Length = length; s->IOSer.io_Actual = 0;
    DoIO((struct IORequest *)s);
    return s->IOSer.io_Error;
}

static int write_text(struct IOExtSer *s, const char *text)
{
    int e = do_io(s, CMD_WRITE, (APTR)text, strlen(text));
    if (e || s->IOSer.io_Actual != strlen(text)) { printf("WRITE failed: %ld actual %lu\n", (LONG)e, s->IOSer.io_Actual); return 0; }
    return 1;
}

static int read_text(struct IOExtSer *s, const char *expected)
{
    char b[40]; ULONG n = strlen(expected); int e;
    e = do_io(s, CMD_READ, b, n); b[n] = 0;
    if (e || s->IOSer.io_Actual != n || memcmp(b, expected, n)) {
        printf("READ failed: %ld actual %lu got '%s' expected '%s'\n", (LONG)e, s->IOSer.io_Actual, b, expected); return 0;
    }
    printf("<- %s", b); return 1;
}

int main(void)
{
    struct MsgPort *port = CreateMsgPort();
    struct IOExtSer *s;
    int ok = 0;
    if (!port) { puts("CreateMsgPort failed"); return RETURN_FAIL; }
    s = (struct IOExtSer *)CreateIORequest(port, sizeof(*s));
    if (!s) { puts("CreateIORequest failed"); DeleteMsgPort(port); return RETURN_FAIL; }
    if (OpenDevice(ABBTCP_DEVICE_NAME, 0, (struct IORequest *)s, 0)) { puts("OpenDevice failed"); goto out; }
    puts("Opened abbstcp.device unit 0");
    if (!write_text(s, "AT\r") || !read_text(s, "OK\r\n")) goto close;
    if (do_io(s, ABBTCP_CMD_INJECT_RING, 0, 0)) { puts("RING injection failed"); goto close; }
    if (!read_text(s, "RING\r\n")) goto close;
    if (!write_text(s, "ATA\r") || !read_text(s, "CONNECT 38400\r\n")) goto close;
    if (!write_text(s, "ATH\r") || !read_text(s, "NO CARRIER\r\n")) goto close;
    ok = 1;
close:
    CloseDevice((struct IORequest *)s);
out:
    DeleteIORequest((struct IORequest *)s); DeleteMsgPort(port);
    puts(ok ? "M0.1 probe passed" : "M0.1 probe FAILED");
    return ok ? RETURN_OK : RETURN_FAIL;
}
