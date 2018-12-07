#ifndef __ATA_H__
#define __ATA_H__

#include <pci.h>
#include <idt.h>
#include <io.h>
#include <memory.h>

#define ATA_PRIMARY_IO 0x1f0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_CTRL 0x3f6
#define ATA_SECONDARY_CTRL 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

#define ATA_SR_BSY     0x80
#define ATA_SR_DRDY    0x40
#define ATA_SR_DF      0x20
#define ATA_SR_DSC     0x10
#define ATA_SR_DRQ     0x08
#define ATA_SR_CORR    0x04
#define ATA_SR_IDX     0x02
#define ATA_SR_ERR     0x01

#define ATA_ER_BBK      0x80
#define ATA_ER_UNC      0x40
#define ATA_ER_MC       0x20
#define ATA_ER_IDNF     0x10
#define ATA_ER_MCR      0x08
#define ATA_ER_ABRT     0x04
#define ATA_ER_TK0NF    0x02
#define ATA_ER_AMNF     0x01

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define      ATAPI_CMD_READ       0xA8
#define      ATAPI_CMD_EJECT      0x1B

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// Directions:
#define ATA_DMA_READ 0x08
#define ATA_DMA_WRITE 0x00

// Bus master register offset
#define ATA_BMS_CR_1 0x0
#define ATA_BMS_SR_1 0x2
#define ATA_BMS_CR_2 0x08
#define ATA_BMS_SR_2 0x0a

#define ATA_BMS_PRD_R_1 0x04
#define ATA_BMS_PRD_R_2 0x0c

#define ATA_PCI_CR 0x09

extern void asm_ata_irq_handler1();
extern void asm_ata_irq_handler2();

#define READ 0x01
#define WRITE 0x02

enum devtype{
    block_dev,
    char_dev,
    socket,
};

typedef struct
{
    u16 io_base;
    u32 bus;
    u32 drive;
    u16 bmr;
    devtype type;
    pci_device* dev;
}ata;

typedef struct
{
    u32 phy_addr;
    u16 bytes;
    u16 reserved;
}prd;

typedef struct
{
    u16 io;
    u16 bmr;
    bool operation;
}ata_operation;

void create_ata(ata* dev, u32 bus, u32 drive, u32 subcode, u32 basecode);
void ide_primary_irq();
void ide_secondary_irq();
u8 ide_identify(u8 bus, u8 drive);
void ide_400ns_delay(u16 io);
void ide_poll(u16 io);
u32 ide_read_write(ata* dev, char* buffer, u32 lba, u16 count, bool read)
u32 disk_read(u32 lba, u32 offset, void* buffer, u32 bytes)
u32 disk_write(u32 lba, u32 offset, void* buffer, u32 bytes);
int ata_init();
#endif // __ATA_H__
