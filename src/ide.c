#include <stdio.h>
#include <stdint.h>
#include "ide.h"

uint16_t *ide_init(void)
{
    static uint16_t buf[256];

    // Initialize IDE controller
    printf("Initializing IDE controller...\n");
    ide_wait_for_flag(IDE_STAT_RDY);
    printf("LBA0: 0x%02x\n", read_reg(IDE_REG_LBA_0));
    printf("LBA8: 0x%02x\n", read_reg(IDE_REG_LBA_8));
    printf("LBA16: 0x%02x\n", read_reg(IDE_REG_LBA_16));
    printf("LBA24: 0x%02x\n", read_reg(IDE_REG_LBA_24));
    printf("\n");
    ide_seek(0);
    printf("LBA0: 0x%02x\n", read_reg(IDE_REG_LBA_0));
    printf("LBA8: 0x%02x\n", read_reg(IDE_REG_LBA_8));
    printf("LBA16: 0x%02x\n", read_reg(IDE_REG_LBA_16));
    printf("LBA24: 0x%02x\n", read_reg(IDE_REG_LBA_24));
    ide_wait_for_flag(IDE_STAT_RDY);
    ide_set_feature(IDE_FEAT_ENABLE_8BIT);
    ide_wait_for_flag(IDE_STAT_RDY);

    printf("Setting PIO Mode\n");
    write_reg(IDE_REG_SEC_CNT, 0x0); // PIO Mode 0, just in case
    ide_set_feature(0x3);
    ide_wait_for_flag(IDE_STAT_RDY);
    printf("OK");

    write_reg(IDE_REG_SEC_CNT, 0x01);

    send_cmd(IDE_CMD_IDENTIFY);
    ide_wait_for_flag(IDE_STAT_DRQ);
    uint8_t *buf8 = (uint8_t *)buf;
    for (int i = 0; i < 512; i++)
        buf8[i] = read_reg(IDE_REG_DATA);
    return buf;
}

void ide_seek(uint32_t lba)
{
    write_reg(IDE_REG_LBA_0, lba & 0xFF);
    write_reg(IDE_REG_LBA_8, (lba >> 8) & 0xFF);
    write_reg(IDE_REG_LBA_16, (lba >> 16) & 0xFF);
    write_reg(IDE_REG_LBA_24, (lba >> 24) & 0xF | 0xE0); // high bits: LBA mode, also master device
}

void ide_read_sector(uint8_t *buf, uint16_t sec_cnt)
{
    ide_wait_for_flag(IDE_STAT_RDY);
    write_reg(IDE_REG_FEATURE, 0);
    send_cmd(IDE_CMD_READ_SECTOR);
    ide_wait_for_flag(IDE_STAT_DRQ);
    int j=0;
    uint8_t status;
    for (int i = 0; i < 512*sec_cnt; i++) {
        while (!(ide_read_status() & IDE_STAT_DRQ)) j++;
        while (ide_read_status() & IDE_STAT_BUSY) j++;
        buf[i] = read_reg(IDE_REG_DATA);
    }
    printf("total drq waits: %d\n", j);
}

void ide_write_sector(uint8_t *buf, uint16_t sec_cnt)
{
    ide_wait_for_flag(IDE_STAT_RDY);
    send_cmd(IDE_CMD_WRITE_SECTOR);
    ide_wait_for_flag(IDE_STAT_DRQ);
    for (int i = 0; i < 512*sec_cnt; i++) {
        write_reg(IDE_REG_DATA, buf[i]);
    }
}

uint8_t ide_read_status(void) {
    return in_portb(IDE(IDE_REG_STATUS));
}

uint8_t ide_wait_for_flag(ide_status_t flag) {
    uint8_t status = ide_read_status();
    if (status & IDE_STAT_ERR) {
        uint8_t error = read_reg(IDE_REG_ERROR);
        printf("error: %s\n", err2str(error));
    }
    printf("..wait for %s in %s\n", flag2str(flag), status2str(status));
    while (!(status = ide_read_status() & flag)) {
        // Wait until the status register has the expected flag set
    };
    if (status & IDE_STAT_ERR) {
        uint8_t error = read_reg(IDE_REG_ERROR);
        printf("error: %s\n", err2str(error));
    }
    return status;
}

void write_reg(uint8_t reg, uint8_t value)
{
    out_portb(IDE(reg), value);
    if (reg != IDE_REG_DATA)
        printf("%s <- 0x%02x\n", reg2str(reg, 0), value);
}

uint8_t read_reg(uint8_t reg)
{
    uint8_t value = in_portb(IDE(reg));
    if (reg != IDE_REG_DATA && reg != IDE_REG_STATUS)
        printf("%s -> 0x%02x\n", reg2str(reg, 1), value);
    return value;
}

void send_cmd(ide_commands_t cmd)
{
    write_reg(IDE_REG_COMMAND, cmd);
    uint8_t status = 0;
    //for(int i=0; i<15; i++)
    //   status ^= ide_read_status();
    status = ide_wait_for_flag(IDE_STAT_RDY);
    if (status & IDE_STAT_ERR) {
        uint8_t error = read_reg(IDE_REG_ERROR);
        printf("cmd(0x%02x) error: 0x%02x\n", cmd, error);
    }
}

void ide_set_feature(ide_feature_t feature)
{
    write_reg(IDE_REG_FEATURE, feature);
    send_cmd(IDE_CMD_SET_FEATURE);
}

void out_portb(uint8_t port, uint8_t value) __naked
{
    __asm
    ld  c,a
    ld  a,l
    out (c), a
    ret
    __endasm;
}

uint8_t in_portb(uint8_t port) __naked
{
    __asm
    ld  c,a
    in  a,(c)
    ld l,a
    ret
    __endasm;
}

char *status2str(uint8_t status)
{
    static char res[64];
    sprintf(res,
        "[%s %s %s %s %s %s %s %s]: 0x%02x",
        (status & IDE_STAT_BUSY)? "BSY" : "bsy",
        (status & IDE_STAT_RDY) ? "RDY" : "rdy",
        (status & IDE_STAT_DWF) ? "DWF" : "dwf",
        (status & IDE_STAT_DSC) ? "DSC" : "dsc",
        (status & IDE_STAT_DRQ) ? "DRQ" : "drq",
        (status & IDE_STAT_CORR) ? "COR" : "cor",
        (status & IDE_STAT_IDX) ? "IDX" : "idx",
        (status & IDE_STAT_ERR) ? "ERR" : "err",
        status
    );
    return res;
}

char *err2str(uint8_t error) {
    static char res[64];
    switch (error) {
        case IDE_ERR_AMNF:  return "AMNF";
        case IDE_ERR_TKZNF: return "TKZNF";
        case IDE_ERR_ABRT:  return "ABRT";
        case IDE_ERR_MCR:   return "MCR";
        case IDE_ERR_IDNF:  return "IDNF";
        case IDE_ERR_MC:    return "MC";
        case IDE_ERR_UNC:   return "UNC";
        case IDE_ERR_BBK:   return "BBK";
    }
    return "";
}

char *flag2str(ide_status_t flag)
{
    static char res[32];
    switch (flag) {
        case IDE_STAT_BUSY: return "BSY";
        case IDE_STAT_RDY:  return "RDY";
        case IDE_STAT_DWF:  return "DWF";
        case IDE_STAT_DSC:  return "DSC";
        case IDE_STAT_DRQ:  return "DRQ";
        case IDE_STAT_CORR: return "COR";
        case IDE_STAT_IDX:  return "IDX";
        case IDE_STAT_ERR:  return "ERR";
    }
    return "";
}

char *reg2str(uint8_t reg, uint8_t is_read)
{
    static char res[32];
    switch (reg) {
        case IDE_REG_DATA:    return "DATA";
        case IDE_REG_FEATURE: return is_read ? "ERROR" : "FEATURE";
        case IDE_REG_SEC_CNT: return "SEC_CNT";
        case IDE_REG_LBA_0:   return "LBA_0";
        case IDE_REG_LBA_8:   return "LBA_8";
        case IDE_REG_LBA_16:  return "LBA_16";
        case IDE_REG_LBA_24:  return "LBA_24";
        case IDE_REG_COMMAND: return is_read ? "STATUS" : "COMMAND";
    }
    sprintf(res, "REG(0x%02x)", reg);
    return res;
}
