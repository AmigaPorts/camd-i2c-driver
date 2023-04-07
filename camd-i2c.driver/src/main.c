/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/resident.h>
#include <exec/initializers.h>
#include <exec/execbase.h>
#include <clib/debug_protos.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/input.h>

#include <midi/camddevices.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERS VERSION_NAME_DATE "\r\n"
#define NUMPORTS 1

/*    Prototypes    */
extern BOOL Init(struct ExecBase *SysBase asm("a6"));

extern void Expunge(void);

extern struct MidiPortData *OpenPort(
	struct MidiDeviceData *data asm("a3"),
	LONG portnum asm("d0"),
	ULONG (*transmitfunc)(APTR userdata asm("a2")) asm("a0"),
	void (*receivefunc)(UWORD input asm("d0"), APTR userdata asm("a2")) asm("a1"),
	APTR userdata asm("a2")
);

extern void ClosePort(
	struct MidiDeviceData *data asm("a3"),
	LONG portnum asm("d0")
);

extern void ActivateXmit(APTR userdata asm("a2"), ULONG portnum asm("d0"));
/*   End prototypes  */


int __attribute__((no_reorder)) _start() {
	return -1;
}

const char libraryIdString[] __attribute__((used)) = VERSION_STRING;

/***********************************************************************
   The mididevicedata struct.
   Note. It doesn't have to be declared with the const qualifier, since
   NPorts may be set at Init. You should set the name-field to the
   same as the filename, that might be a demand...
***********************************************************************/
const struct MidiDeviceData MidiDeviceDataStruct __attribute__((used)) = {
	MDD_Magic,
	LIBRARY_NAME,
	VERS,
	LIBRARY_VERSION,
	LIBRARY_REVISION,
	Init,
	Expunge,
	OpenPort,
	ClosePort,
	NUMPORTS,
	1
};
