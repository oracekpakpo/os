#include <ata.h>

extern void isr_14();
extern void isr_15();

static u16 ide[256];

static prd bus0prd __attribute__((aligned(sizeof(u32))));
static prd bus1prd __attribute__((aligned(sizeof(u32))));

ata hd;
static ata_operation ata_state;

void create_ata(ata* dev, u32 bus, u32 drive, u32 subcode, u32 basecode)
{
    dev->bus = bus;
    dev->drive = drive;
    dev->dev = find_pci_device(subcode,basecode);
    if(subcode == 0x01 && basecode == 0x01)
        dev->type = block_dev;
    if(!bus)
        dev->io_base = ATA_PRIMARY_IO;
    else
        dev->io_base = ATA_SECONDARY_IO;

    write_pci_device(dev->dev,COMMAND_REG_OFF,0x05); // Make ATA can be a bus master
    
    dev->bmr = read_pci_device(dev->dev,BUS_MASTER_REG_OFF);
    dev->bmr &= ~1;

}

void ide_select_drive(u8 bus, u8 drive)
{
    if(bus == ATA_PRIMARY)
        if(drive == ATA_MASTER)
            outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL,0xa0);
        else  
            outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL,0xb0);
    else
        if(drive == ATA_MASTER)
            outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL,0xa0);
        else
            outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL,0xb0);
}

u8 ide_identify(u8 bus, u8 drive)
{
    ide_select_drive(bus,drive);
    u16 io = 0;
    if(bus == ATA_PRIMARY)
        io = ATA_PRIMARY_IO;
    else
        io = ATA_SECONDARY_IO;

    outb(io+ATA_REG_SECCOUNT0,0);
    outb(io+ATA_REG_LBA0,0);
    outb(io+ATA_REG_LBA1,0);
    outb(io+ATA_REG_LBA2,0);

    /* Now send identify */
    outb(io+ATA_REG_COMMAND,ATA_CMD_IDENTIFY);

    /*Now read status port */
    u8 status = inb(io+ATA_REG_STATUS);

    if(status)
    {
        while(inb(io+ATA_REG_STATUS) & ATA_SR_BSY !=0);
stat_read:   status = inb(io+ATA_REG_STATUS);
        if(status & ATA_SR_ERR)
        {
            #ifdef __DEBUG__
            kprintf("ERR set last use occur an error !\n");
            #endif // __DEBUG__
            return 0;
        }

        while(!(status & ATA_SR_DRQ)) goto stat_read;
        #ifdef __DEBUG__
        kprintf("ATA is ready to perform a transaction..\n");
        #endif // __DEBUG__
        /* Now read data */
        for(int i = 0; i < 256; i++) ide[i]=inw(io+ATA_REG_DATA);
        return 1;
    }
}


void ide_400ns_delay(u16 io)
{
    for(int i = 0; i < 4; i++)
    {
        inb(io+ATA_REG_ALTSTATUS);
    }
}

void ide_poll(u16 io)
{
    for(int i = 0; i < 4; i++)
    {
        inb(io+ATA_REG_ALTSTATUS);
    }

    retry:;
    u8 status = inb(io+ATA_REG_STATUS);
    if(status & ATA_SR_BSY)
    {
		goto retry;
	}

    retry2: status = inb(io+ATA_REG_STATUS);
    if(status & ATA_SR_ERR)
    {
        #ifdef __DEBUG__
        kprintf("ERR set.\n");
        #endif // __DEBUG__
    }

    if(!(status & ATA_SR_DRQ)) goto retry2;
    return ;
}

void ide_primary_irq()
{
    u8 state = inb(ata_state.bmr + ATA_BMS_SR_1);
    if(state & 0x04)
    {
        state &= ~1;
        outb(bmr,state);
    }

    if(ata_state.operation == WRITE)
    {
        outb(hd.io_base + ATA_REG_COMMAND,ATA_CMD_CACHE_FLUSH);
        ide_400ns_delay(ATA_PRIMARY_IO);
    }
}

void ide_secondary_irq()
{
    u8 state = inb(ata_state.bmr + ATA_BMS_SR_2);
    if(state & 0x04)
    {
        state &= ~1;
        outb(bmr,state);
    }

    if(ata_state.operation == WRITE)
    {
        outb(hd.io_base + ATA_REG_COMMAND,ATA_CMD_CACHE_FLUSH);
        ide_400ns_delay(ATA_SECONDARY_IO);
    }
}

/* 48 bit adressage mode */
u32 ide_read_write(ata* dev, char* buffer, u32 lba, u16 count, bool read) // read == 1 --> disk_write
                                                                          // read == 0 --> disk_read
{
    if(read)
        ata_state.operation = READ;
    else
        ata_state.operation = WRITE;

    ata_state.io = dev->io_base;

    u8 cmd = (dev->drive == ATA_MASTER) ? 0x40 : 0x50;

    u16 bmr = dev->bmr;
    ata_state.bmr = bmr;

    /* Now reset bus master command register */

    if(dev->bus == ATA_PRIMARY)
        outb(bmr + ATA_BMS_CR_1,0x0);
    else
        outb(bmr + ATA_BMS_CR_2,0x0);

    /* Now setup prd */
    if(dev->bus == ATA_PRIMARY)
    {
        bus0prd.phy_addr = (u32)buffer;
        bus0prd.bytes = count * 512;
        bus0prd.reserved = (1<<15);

        outl(bmr+ATA_BMS_PRD_R_1,(32_t)&bus0prd);
    }else
    {
        bus1prd.phy_addr = (u32)buffer;
        bus1prd.bytes = count * 512;
        bus1prd.reserved = (1<<15);

        outl(bmr+ATA_BMS_PRD_R_2,(u32_t)&bus1prd);
    }


    /* Now setup sector location */
    outb(dev->io_base + ATA_REG_HDDEVSEL,cmd);
    
    outb(dev->io_base + ATA_REG_SECCOUNT0,(count >> 8) & 0xff);
    outb(dev->io_base + ATA_REG_LBA0,(lba >> 24) & 0xff);
    outb(dev->io_base + ATA_REG_LBA1,0);
    outb(dev->io_base + ATA_REG_LBA2,0);

    outb(dev->io_base + ATA_REG_SECCOUNT0, count & 0xff);
    outb(dev->io_base + ATA_REG_LBA0, lba & 0xff);
    outb(dev->io_base + ATA_REG_LBA1,(lba >> 8) & 0xff);
    outb(dev->io_base + ATA_REG_LBA2,(lba >> 16) & 0xff);

    if(read)
        outb(dev->io_base + ATA_REG_COMMAND,ATA_CMD_READ_DMA_EXT);
    else
        outb(dev->io_base + ATA_REG_COMMAND,ATA_CMD_WRITE_DMA_EXT);

    #ifdef __DEBUG__
    ide_poll(dev->io_base);
    #endif //__DEBUG__

    /* Now clear IDE ERROR and INTERRUPT bit */

    if(dev->bus == ATA_PRIMARY)
    {
        u8 state = inb(bmr + ATA_BMS_SR_1);
        state = state|2|4;
        outb(bmr + ATA_BMS_SR_1,state);
    }
    else
    {
        u8 state = inb(bmr + ATA_BMS_SR_2);
        state = state|2|4;
        outb(bmr + ATA_BMS_SR_2,state);
    }

    /* Now select operation direction */

    if(dev->bus == ATA_PRIMARY)
        if(read)
            outb(bmr + ATA_BMS_CR_1,ATA_DMA_READ|0x01);
        else
            outb(bmr + ATA_BMS_CR_1,ATA_DMA_WRITE|0x01);
    else
        if(read)
            outb(bmr + ATA_BMS_CR_2,ATA_DMA_READ|0x01);
        else
            outb(bmr + ATA_BMS_CR_2,ATA_DMA_WRITE|0x01);

    while(1)
    {
        u8 status = (dev->bus == ATA_PRIMARY)? inb(bmr + ATA_BMS_SR_1):inb(bmr + ATA_BMS_SR_2) ;
        u8 dstatus = inb(dev->io_base + ATA_REG_STATUS);

        if(!(status & 0x04))
            continue;

        if(!(dstatus & 0x80))
            break;
    }

    return 1;
}



u32 disk_read(u32 lba, u32 offset, void* buffer, u32 bytes)
{
    if(!bytes)
        return bytes;

    char* bl_buffer = NULL;
    u32 __lba = lba + (offset / 512);
    u32 __count = bytes / 512;
    if(bytes % 512)
        __count++;
    bl_buffer = sys_alloc(__count * 512);

    ide_read_write(&hd,bl_buffer,__lba,__count,1);
    memcpy(buffer,bl_buffer + (offset % 512),bytes);
    sys_free(bl_buffer);
    return bytes;
}

u32 disk_write(u32 lba, u32 offset, void* buffer, u32 bytes)
{
    if(!bytes)
        return bytes;

    char* bl_buffer = NULL;
    u32 __lba = lba + (offset / 512);
    u32 __count = bytes / 512;
    if(bytes % 512)
        __count++;
    bl_buffer = sys_alloc(__count * 512);
    if(offset)
        ide_read_write(&hd,bl_buffer,__lba,__count,1);
    memcpy(bl_buffer + (offset % 512),buffer,bytes);
    ide_read_write(&hd,bl_buffer,__lba,__count,0);
    sys_free(bl_buffer);
    return bytes;
}


int ata_init()
{
    kprintf("Installing PCI devices context...");
    lookup_pci_devices(); kprintf("[ok]\n");
    
    for(int bus = 0; bus < 2; bus++)
    {
        for(int drive = 0; drive < 2; drive++)
        {
            kprintf("Detecting %s %s ...\n",(bus == 0)? "ATA_PRIMARY":"ATA_SECONDRY",
                (drive == 0)? "ATA_MASTER":"ATA_SLAVE");

            if(ide_identify(bus,drive))
            {
                create_ata(&hd,bus,drive,0x01,0x01);
                return 1;
            }
        }
    }
    
    kprintf("Installing ATA irq handler...");
    set_irq_handler((u32)isr_14,ATA_PRIMARY_IRQ);
    set_irq_handler((u32)isr_15,ATA_SECONDARY_IRQ);
    kprintf("[ok]\n");

    return -1;
}

