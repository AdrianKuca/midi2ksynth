// https://github.com/korneliuszo/avr-touchpad/blob/master/enter_bootloader.h
#ifndef _ENTER_BOOTLOADER_H_
#define _ENTER_BOOTLOADER_H_
/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include <LUFA/Common/Common.h>
#include <LUFA/Drivers/USB/USB.h>

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

/* Macros: */
#define MAGIC_BOOT_KEY 0xDC42ACCA
#if (BOARD == BOARD_LEONARDO)
#define BOOTLOADER_START_ADDRESS (0x3C00)
#else
#error Unsupported board for this entering bootloader file.
#endif
/* Function Prototypes: */
void enter_bootloader(void);

#endif