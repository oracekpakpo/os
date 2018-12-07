#ifndef __PCI_H__
#define __PCI_H__

#include <types.h>
#include <io.h>
#include <memory.h>
#include <lib.h>


#define TOTAL_PCI_DEVICE 256
#define PCI_CONFIG_ADDR 0xcf8
#define PCI_DATA_ADDR 0xcfc

#define VENDOR_ID_REG_OFF 0x00
#define DEVICE_ID_REG_OFF 0x02
#define COMMAND_REG_OFF 0x04
#define DEVICE_STATUS_REG_OFF 0x06
#define REVISION_ID_REG_OFF 0x08
#define PROGRAMING_REG_OFF 0x09
#define SUBCLASS_REG_OFF 0x0a
#define BASECLASS_REG_OFF 0x0b
#define MASTER_LATENCY_REG_OFF 0x0d
#define PRIMARY_CMD_TASK_REG_OFF 0x10
#define PRIMARY_CTRL_TASK_REG_OFF 0x14
#define SECONDARY_CMD_TASK_REG_OFF 0x18
#define SECONDARY_CTRL_TASK_REG_OFF 0x1c
#define BUS_MASTER_REG_OFF 0x20
#define INTERRUPT_LINE_REG_OFF 0x3c
#define INTERRUPT_PIN_REG_OFF 0x3d


typedef struct PCIDevice
{
    u8 pci_bus;
    u8 pci_device;
    u8 pci_function;
    u8 device_name[64];
} pci_device;

u32 rd_pci_device(u8 bus, u8 dev, u8 function, u8 offset);
void wr_pci_device(u8 bus, u8 dev, u8 function, u8 offset, u32 data);
void lookup_pci_devices();
void set_name_to_pci_device(pci_device* , u8* );
u32 read_pci_device(pci_device* , u8 offset);
void write_pci_device(pci_device* , u8 offset, u32 data);
pci_device* find_pci_device(u8 subclass, u8 baseclass);
/* functions */
#endif // __PCI_H__
