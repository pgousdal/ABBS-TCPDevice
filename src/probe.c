#include <stdio.h>

int main(void)
{
    puts("ABBS-TCPDevice M0 probe");
    puts("This scaffold still needs real AmigaOS NDK OpenDevice glue.");
    puts("Planned runtime sequence:");
    puts("  OpenDevice(\"abbstcp.device\", 0, ...)");
    puts("  WRITE AT -> READ OK");
    puts("  inject RING -> READ RING");
    puts("  WRITE ATA -> READ CONNECT 38400");
    puts("  WRITE ATH -> READ NO CARRIER");
    return 0;
}
