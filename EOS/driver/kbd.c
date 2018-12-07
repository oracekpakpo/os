#include <kbd_driver.h>
#include <io.h>

#define keyboard_cmd_reg 0x64
#define keyboard_data_reg 0x60


void init_keyboard()
{
}

uchar read_cmd_reg()
{
    uchar data;
    data=inb(keyboard_cmd_reg);
    return data;
}

void write_cmd_reg(uchar data)
{
    outb(keyboard_cmd_reg,data);
}


uchar read_data_reg()
{
    uchar data;
    data=inb(keyboard_data_reg);
    return data;
}

void write_data_reg(uchar data)
{
    outb(keyboard_data_reg,data);
}



