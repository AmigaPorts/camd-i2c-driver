/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <exec/memory.h>
#include <exec/interrupts.h>
#include <exec/exec.h>
#include <inline/alib.h>
#include <libraries/i2c.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <proto/expansion.h>
#include <proto/i2c.h>

#include <midi/camddevices.h>
#include <stdio.h>

static ULONG CAMDv40 = FALSE;

unsigned long iobase = 0x42;

APTR I2C_Base = NULL;
struct DosLibrary *DOSBase = NULL;
struct ExecBase *SysBase = NULL;


struct PortInfo {
	ULONG (*Transmit)(APTR driverdata);

	void (*Receive)(UWORD data, APTR driverdata);

	APTR UserData;
};

struct PortInfo pi;

void ActivateXmit(APTR userdata asm("a2"), ULONG portnum asm("d0")) {
	ULONG b;
	while ( (b = pi.Transmit(userdata)) != 0x100 ) {
		if ((b & 0x00008100) == 0x0000 ) {
			UBYTE midiData = b & 0xFF;
			ULONG result = SendI2C(iobase, 1, &midiData);
			if ((result & 0xFF) != 0 )
				break;
		}

		if ((!CAMDv40 && (b & 0x00ff0000) != 0) ||
			(b & 0x00008100) != 0 ) {
			break;
		}
	}
}

struct MidiPortData MidiPortData =
	{
		ActivateXmit
	};

void Expunge() {
}

BOOL Init(struct ExecBase *sysBase asm("a6")) {
	SysBase = sysBase;
	return TRUE;
}

struct MidiPortData *OpenPort(
	struct MidiDeviceData *data asm("a3"),
	LONG portnum asm("d0"),
	ULONG (*transmitfunc)(APTR userdata asm("a2")) asm("a0"),
	void (*receivefunc)(UWORD input asm("d0"), APTR userdata asm("a2")) asm("a1"),
	APTR userdata asm("a2")
) {
	DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 37);

	if ( DOSBase == NULL) {
		return NULL;
	}

	static struct Library *camdBase = NULL;

	if ( camdBase == NULL) {
		camdBase = OpenLibrary("camd.library", 0);

		if ( camdBase != NULL) {
			CAMDv40 = (camdBase->lib_Version >= 40);
		}

		// Close library but leave pointer set, so we never execute this
		// code again.
		CloseLibrary(camdBase);
	}

	// init i2c stuff here
	I2C_Base = (struct I2C_Base *)OpenLibrary("i2c.library", 0);
	if ( I2C_Base == NULL) {
		Printf("Unable to open 'i2c.library'.\n");
		return NULL;
	}

	Printf(LIBRARY_NAME " Initialized\n");

	pi.Transmit = transmitfunc;
	pi.Receive = receivefunc;
	pi.UserData = userdata;

	Printf(LIBRARY_NAME " OpenPort succesfull\n");

	return &MidiPortData;
}

void ClosePort(
	struct MidiDeviceData *data asm("a3"),
	LONG portnum asm("d0")
) {
	CloseLibrary((struct Library *)I2C_Base);
	CloseLibrary((struct Library *)DOSBase);

	I2C_Base = NULL;
}
