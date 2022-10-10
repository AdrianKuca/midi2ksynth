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

/** Array of structures describing each note being generated */
static DDSNoteData NoteData[MAX_SIMULTANEOUS_NOTES];

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
	// 131 IN ATMEGA DOCS
	PLLFRQ = (PLLFRQ & ~(1<<PLLTM1)) | (1 << PLLTM0); // PLL enable for faster clock
	DDRB = (1 << 6) | (1 << 0);				// output pwm; output led yellow
	DDRD = (1 << 5);						// output led green
	TCCR4A = (1 << COM4B1) | (1 << PWM4B);  // Cleared on Compare Match. Set when TCNT4 = 0x000; FAST PWM mode
	TCCR4B = (1 << CS40);	// No prescaling
	TC4H = 0x3;
	OCR4C = 0xff;
	// TCCR4C = ;
	// TCCR4D = ; // FAST PWM
	// TCCR4E = ;
	GlobalInterruptEnable();
	uint8_t current_pitch = 0;
	for (;;)
	{
		MIDI_EventPacket_t ReceivedMIDIEvent;
		if (MIDI_Device_ReceiveEventPacket(&Keyboard_MIDI_Interface, &ReceivedMIDIEvent))
		{
			if ((ReceivedMIDIEvent.Event == MIDI_EVENT(0, MIDI_COMMAND_NOTE_ON)) && ((ReceivedMIDIEvent.Data1 & 0x0F) == 0))
			{
				DDSNoteData *LRUNoteStruct = &NoteData[0];

				/* Find a free entry in the note table to use for the note being turned on */
				for (uint8_t i = 0; i < MAX_SIMULTANEOUS_NOTES; i++)
				{
					/* Check if the note is unused */
					if (!(NoteData[i].Pitch))
					{
						/* If a note is unused, it's age is essentially infinite - always prefer unused note entries */
						LRUNoteStruct = &NoteData[i];
						break;
					}
					else if (NoteData[i].LRUAge >= LRUNoteStruct->LRUAge)
					{
						/* If an older entry that the current entry has been found, prefer overwriting that one */
						LRUNoteStruct = &NoteData[i];
					}

					NoteData[i].LRUAge++;
				}

				/* Update the oldest note entry with the new note data and reset its age */
				LRUNoteStruct->Pitch = ReceivedMIDIEvent.Data2;
				current_pitch = ReceivedMIDIEvent.Data2;
				LRUNoteStruct->TableIncrement = (uint32_t)(BASE_INCREMENT * SCALE_FACTOR) +
												((uint32_t)(BASE_INCREMENT * NOTE_OCTIVE_RATIO * SCALE_FACTOR) *
												 (ReceivedMIDIEvent.Data2 - BASE_PITCH_INDEX));
				LRUNoteStruct->TablePosition = 0;
				LRUNoteStruct->LRUAge = 0;

				TCCR4A = (1 << COM4B1);
			}
			else if ((ReceivedMIDIEvent.Event == MIDI_EVENT(0, MIDI_COMMAND_NOTE_OFF)) && ((ReceivedMIDIEvent.Data1 & 0x0F) == 0))
			{
				bool FoundActiveNote = false;

				/* Find the note in the note table to turn off */
				for (uint8_t i = 0; i < MAX_SIMULTANEOUS_NOTES; i++)
				{
					if (NoteData[i].Pitch == ReceivedMIDIEvent.Data2)
					{
						NoteData[i].Pitch = 0;
						current_pitch = 0;
						TCCR4A = (0 << COM4B1);
					}
					else if (NoteData[i].Pitch)
						FoundActiveNote = true;
				}
			}
		}
		// Uses PB6 to output voltage corresponding to the pitch.
		// 124 in atmega32u4 docs
		if(current_pitch != 0 ){
			uint32_t big_pitch = ((uint32_t) current_pitch << 28) / (127 - BASE_PITCH_INDEX); // pitch index / pitch range
			TC4H = (uint8_t) 	(big_pitch>>24 & 0x3);
			OCR4B = (uint8_t) 	(big_pitch>>16 & 0xff); 
		}
		else{
			TC4H = 0;
			OCR4B = 0;
		}
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
	if (((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_TYPE) == REQTYPE_VENDOR) && ((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_RECIPIENT) == REQREC_DEVICE))
	{
		if ((USB_ControlRequest.bmRequestType & CONTROL_REQTYPE_DIRECTION) == REQDIR_HOSTTODEVICE)
		{
			switch (USB_ControlRequest.bRequest)
			{
			case 0x01:
				Endpoint_ClearSETUP();
				Endpoint_ClearStatusStage();
				USB_USBTask();
				Delay_MS(200);

				enter_bootloader();
				break;
			}
		}
		else
		{
			switch (USB_ControlRequest.bRequest)
			{
			}
		}
	}
	else
	{
		MIDI_Device_ProcessControlRequest(&Keyboard_MIDI_Interface);
	}
}
