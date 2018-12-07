#include <pci.h>

u32 pci_devices=0;
static pci_device* sys_devices[TOTAL_PCI_DEVICE];

u32 rd_pci_device(u32 bus, u32 dev, u32 function, u32 offset)
{
   u32 value=
    1<<31 | bus<<16 | dev<<11 | function<<8 | (offset & 0xfc);
    outl(PCI_CONFIG_ADDR,value);
    u32 result=inl(PCI_DATA_ADDR);
    return result>>(8*(offset%4));

}

void wr_pci_device(u32 bus, u32 dev, u32 function, u32 offset, u32 data)
{
	 u32 value=
    1<<31 | bus<<16 | dev<<11 | function<<8 | (offset & 0xfc);
    outl(PCI_CONFIG_ADDR,value);
    outl(PCI_DATA_ADDR,data);
}

void set_name_to_pci_device(pci_device* dev, u8* name)
{
    strcpy(dev->device_name,name);
}

u32 read_pci_device(pci_device* dev, u8 offset)
{
    return rd_pci_device(dev->pci_bus,dev->pci_device,dev->pci_function,offset);
}

void write_pci_device(pci_device* dev, u8 offset, u32 data)
{
    wr_pci_device(dev->pci_bus,dev->pci_device,dev->pci_function,offset,data);
}


pci_device* find_pci_device(u8 subclass, u8 baseclass)
{
    for(u32 i=0;i<pci_devices;i++)
    {
        if(sys_devices[i])
        {
            if(read_pci_device(sys_devices[i],SUBCLASS_REG_OFF)==subclass && read_pci_device(sys_devices[i],BASECLASS_REG_OFF)==baseclass)
            {
                return sys_devices[i];
            }
        }
    }
    return NULL;
}


void lookup_pci_devices()
{
    pci_devices = 0; // Update devices count value;

    for(u32 bus=0;bus<8;bus++)
    {
        for(u32 dev=0;dev<32;dev++)
        {
            for(u32 funct=0; funct<8; funct++)
            {
                if(rd_pci_device(bus,dev,funct,VENDOR_ID_REG_OFF)==0xffff || rd_pci_device(bus,dev,funct,VENDOR_ID_REG_OFF)==0x0000)
                break;

                sys_devices[pci_devices] = (pci_device*)sys_alloc(sizeof(pci_device));
                sys_devices[pci_devices]->pci_bus = bus;
                sys_devices[pci_devices]->pci_device = dev;
                sys_devices[pci_devices]->pci_function = funct;
                pci_devices++;
            }
        }
    }
}

