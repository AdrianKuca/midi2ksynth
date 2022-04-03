// https://github.com/korneliuszo/avr-touchpad/blob/master/enter_bootloader.c
#include "enter_bootloader.h"

static uint32_t Boot_Key ATTR_NO_INIT;
#if (BOARD == BOARD_LEONARDO)
static uint16_t bootKey = 0x7777;
static volatile uint16_t *const bootKeyPtr = (volatile uint16_t *)0x0800;
#else
#error Unsupported board for this entering bootloader file.
#endif

void enter_bootloader(void)
{
    Endpoint_ClearSETUP();
    Endpoint_ClearStatusStage();
    USB_USBTask();

    // If USB is used, detach from the bus and reset it
    USB_Disable();

    // Disable all interrupts
    cli();

    // Wait two seconds for the USB detachment to register on the host
    Delay_MS(2000);

    // Set the bootloader key to the magic value and force a reset
    *bootKeyPtr = bootKey;
    wdt_enable(WDTO_250MS);
    for (;;)
        ;
}