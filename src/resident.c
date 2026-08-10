#include <exec/types.h>
#include <exec/resident.h>
#include <exec/nodes.h>
#include <exec/libraries.h>
#include "abbstcp_device.h"
#include "device_internal.h"

extern APTR DevInit, DevOpen, DevClose, DevExpunge, DevReserved, DevBeginIO, DevAbortIO;

__section("code") static const char name[] = ABBTCP_DEVICE_NAME;
__section("code") static const char id[] = "abbstcp.device 0.1 (10.8.2026)\r\n";
__section("code") static APTR vectors[] = { &DevOpen, &DevClose, &DevExpunge, &DevReserved, &DevBeginIO, &DevAbortIO, (APTR)-1 };
__section("code") static const ULONG init_table[] = { sizeof(struct AbbTcpBase), (ULONG)vectors, 0, (ULONG)&DevInit };

__section("code") const struct Resident resident = {
    RTC_MATCHWORD, (struct Resident *)&resident, (APTR)(&resident + 1),
    RTF_AUTOINIT, ABBTCP_VERSION, NT_DEVICE, 0,
    (char *)name, (char *)id, (APTR)init_table
};
