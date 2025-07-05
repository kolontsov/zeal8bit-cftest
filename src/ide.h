#define IDE(x) (0x70+x)

typedef enum {
     IDE_STAT_BUSY = 0x80,
     IDE_STAT_RDY  = 0x40,
     IDE_STAT_DWF  = 0x20,
     IDE_STAT_DSC  = 0x10,
     IDE_STAT_DRQ  = 0x08,
     IDE_STAT_CORR = 0x04,
     IDE_STAT_IDX  = 0x02,
     IDE_STAT_ERR  = 0x01
} ide_status_t;

typedef enum {
    IDE_REG_DATA    = 0,
    IDE_REG_FEATURE = 1,
    IDE_REG_ERROR   = 1, // Same as feature
    IDE_REG_SEC_CNT = 2,
    IDE_REG_LBA_0   = 3,
    IDE_REG_LBA_8   = 4,
    IDE_REG_LBA_16  = 5,
    IDE_REG_LBA_24  = 6,
    IDE_REG_COMMAND = 7,
    IDE_REG_STATUS  = 7 // Same as command
} ide_registers_t;

typedef enum {
    IDE_CMD_NOP             = 0x00,
    IDE_CMD_READ_SECTOR     = 0x20,
    IDE_CMD_READ_SECTOR_NR  = 0x21,
    IDE_CMD_WRITE_SECTOR    = 0x30,
    IDE_CMD_WRITE_SECTOR_NR = 0x31,
    IDE_CMD_READ_BUFFER     = 0xE4,
    IDE_CMD_WRITE_BUFFER    = 0xE8,
    IDE_CMD_IDENTIFY        = 0xEC,
    IDE_CMD_SET_FEATURE     = 0xEF,
} ide_commands_t;

typedef enum {
    IDE_FEAT_ENABLE_8BIT = 0x01,
    IDE_FEAT_DISABLE_8BIT = 0x81,
} ide_feature_t;

typedef enum {
    IDE_ERR_AMNF  = 0,
    IDE_ERR_TKZNF = 1,
    IDE_ERR_ABRT  = 2,
    IDE_ERR_MCR   = 3,
    IDE_ERR_IDNF  = 4,
    IDE_ERR_MC    = 5,
    IDE_ERR_UNC   = 6,
    IDE_ERR_BBK   = 7,
} ide_error_t;


uint16_t *ide_init(void);
void ide_seek(uint32_t lba);
void ide_read_sector(uint8_t *buf, uint16_t sec_cnt);
void ide_write_sector(uint8_t *buf, uint16_t sec_cnt);
uint8_t ide_read_status(void);
uint8_t ide_wait_for_flag(ide_status_t flag);
void write_reg(uint8_t reg, uint8_t value);
uint8_t read_reg(uint8_t reg);
void send_cmd(ide_commands_t cmd);
void ide_set_feature(ide_feature_t feature);
void out_portb(uint8_t port, uint8_t value);
uint8_t in_portb(uint8_t port);
char *status2str(uint8_t status);
char *reg2str(uint8_t reg, uint8_t is_read);
char *flag2str(ide_status_t flag);
char *err2str(ide_status_t flag);
