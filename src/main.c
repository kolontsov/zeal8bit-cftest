#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ide.h"


static void dump_sector(uint8_t *buf) {
    printf("    ");
    for (int i = 0; i < 64; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0)
            printf("\n    ");
    }
    printf("...\n");
}

int main(int argc, char** argv) {
    uint16_t *ident = ide_init();
    printf("==> Identifier: 0x%04x\n", *ident);
    uint8_t buf[512];

    printf("\nReading sector 0\n");
    ide_read_sector(buf, 1);
    dump_sector((uint8_t *)buf);

    printf("\nStatus: %s\n", status2str(ide_read_status()));

    for (int i=0; i < 512; i++) {
        buf[i] = (i % 256);
    }
    printf("\nWriting sector 0\n");
    ide_write_sector(buf, 1);

    printf("\nStatus: %s\n", status2str(ide_read_status()));

    return 0;
}
