/*
			 LUFA Library
	 Copyright (C) Dean Camera, 2021.

  dean [at] fourwalledcubicle [dot] com
		   www.lufa-lib.org
*/

/*
  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the MIDI2ksynth.
 */

#include "midi2ksynth.h"

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MIDI_Device_t Keyboard_MIDI_Interface =
	{
		.Config =
			{
				.StreamingInterfaceNumber = INTERFACE_ID_AudioStream,
				.DataINEndpoint =
					{
						.Address = MIDI_STREAM_IN_EPADDR,
						.Size = MIDI_STREAM_EPSIZE,
						.Banks = 1,
					},
				.DataOUTEndpoint =
					{
						.Address = MIDI_STREAM_OUT_EPADDR,
						.Size = MIDI_STREAM_EPSIZE,
						.Banks = 1,
					},
			},
};

/** 8-bit 256 entry Sine Wave lookup table */
static const uint8_t SineTable[256] =
	{
		128,
		131,
		134,
		137,
		140,
		143,
		146,
		149,
		152,
		156,
		159,
		162,
		165,
		168,
		171,
		174,
		176,
		179,
		182,
		185,
		188,
		191,
		193,
		196,
		199,
		201,
		204,
		206,
		209,
		211,
		213,
		216,
		218,
		220,
		222,
		224,
		226,
		228,
		230,
		232,
		234,
		236,
		237,
		239,
		240,
		242,
		243,
		245,
		246,
		247,
		248,
		249,
		250,
		251,
		252,
		252,
		253,
		254,
		254,
		255,
		255,
		255,
		255,
		255,
		255,
		255,
		255,
		255,
		255,
		255,
		254,
		254,
		253,
		252,
		252,
		251,
		250,
		249,
		248,
		247,
		246,
		245,
		243,
		242,
		240,
		239,
		237,
		236,
		234,
		232,
		230,
		228,
		226,
		224,
		222,
		220,
		218,
		216,
		213,
		211,
		209,
		206,
		204,
		201,
		199,
		196,
		193,
		191,
		188,
		185,
		182,
		179,
		176,
		174,
		171,
		168,
		165,
		162,
		159,
		156,
		152,
		149,
		146,
		143,
		140,
		137,
		134,
		131,
		128,
		124,
		121,
		118,
		115,
		112,
		109,
		106,
		103,
		99,
		96,
		93,
		90,
		87,
		84,
		81,
		79,
		76,
		73,
		70,
		67,
		64,
		62,
		59,
		56,
		54,
		51,
		49,
		46,
		44,
		42,
		39,
		37,
		35,
		33,
		31,
		29,
		27,
		25,
		23,
		21,
		19,
		18,
		16,
		15,
		13,
		12,
		10,
		9,
		8,
		7,
		6,
		5,
		4,
		3,
		3,
		2,
		1,
		1,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		1,
		1,
		2,
		3,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		12,
		13,
		15,
		16,
		18,
		19,
		21,
		23,
		25,
		27,
		29,
		31,
		33,
		35,
		37,
		39,
		42,
		44,
		46,
		49,
		51,
		54,
		56,
		59,
		62,
		64,
		67,
		70,
		73,
		76,
		79,
		81,
		84,
		87,
		90,
		93,
		96,
		99,
		103,
		106,
		109,
		112,
		115,
		118,
		121,
		124,
};

/** Array of structures describing each note being generated */
static DDSNoteData NoteData[MAX_SIMULTANEOUS_NOTES];

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
	{
		MIDI_Device_USBTask(&Keyboard_MIDI_Interface);
		USB_USBTask();
	}
}

void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#endif
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= MIDI_Device_ConfigureEndpoints(&Keyboard_MIDI_Interface);
}

/** Event handler for the library USB Control Request event including Kornel's enter_bootloader hack. */
void EVENT_USB_Device_ControlRequest(void)
{

	MIDI_Device_ProcessControlRequest(&Keyboard_MIDI_Interface);
}
