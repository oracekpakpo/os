#ifndef __COLOR__H__
#define __COLOR__H__


#define black 0x00
#define blue 0x01
#define green 0x02
#define cyan 0x03
#define red 0x04
#define magenta 0x05
#define yellow 0x06
#define white 0x07

/** mask to enable cli and intensite bit **/

#define NO_CLI_NO_INT 00000000b
#define NO_CLI_INT    00001000b
#define CLI_NO_INT    10000000b
#define CLI_INT       10001000b


#endif
