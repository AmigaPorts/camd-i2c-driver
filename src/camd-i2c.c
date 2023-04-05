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

struct Interrupt TimerInterrupt;
unsigned int irq = 0;
unsigned long iobase = 0;
BOOL TimerRequestActive = FALSE;

struct Device *TimerBase = NULL;
struct timerequest *TimerIO = NULL;
struct MsgPort *ReplyMP = NULL;
APTR I2C_Base = NULL;
struct DosLibrary *DOSBase = NULL;
struct ExecBase *SysBase = NULL;

BOOL SetupTimerDevice(void);

void MicroDelay(unsigned int val);

void CleanUpTimerDevice(void);

struct PortInfo {
	ULONG (*Transmit)(APTR driverdata);

	void (*Receive)(UWORD data, APTR driverdata);

	APTR UserData;
};

struct PortInfo pi;

void ActivateXmit(APTR userdata asm("a2"), ULONG portnum asm("d0")) {
	Disable();

	if ( !TimerRequestActive ) {
		TimerRequestActive = TRUE;

		TimerIO->tr_node.io_Command = TR_ADDREQUEST;
		TimerIO->tr_time.tv_secs = 0;
		TimerIO->tr_time.tv_micro = 0;

		BeginIO((struct IORequest *)TimerIO);
	}

	Enable();
}

struct MidiPortData MidiPortData =
	{
		ActivateXmit
	};

void Expunge() {
}

LONG TimerInterruptCode(APTR data asm("a1"), APTR code asm("a5")) {
	//struct I2C_Base *dev = (struct I2C_Base *)data;
	struct timerequest *tr;
	Printf("TimerInterruptCode\n");

	tr = (struct timerequest *)GetMsg(ReplyMP);

	if ( tr != NULL && I2C_Base != NULL ) {
		ULONG b;

		TimerRequestActive = FALSE;

		while ( TRUE ) {
			/*
		  if( !OUTPUT_READY(iobase) )
		  {
			if( ! TimerRequestActive )
			{
			  TimerRequestActive = TRUE;

			  TimerIO->tr_node.io_Command = TR_ADDREQUEST;
			  TimerIO->tr_time.tv_micro   = TIMER_DELAY_US;

			  BeginIO( (struct IORequest *) TimerIO);
			}

			break;
		  }
		  */

			b = pi.Transmit(pi.UserData);

			if ((b & 0x00008100) == 0x0000 ) {
				ULONG result = SendI2C(iobase, 1, (UBYTE *)(b & 255));
				if ((result & 0xFF) != 0 )
					break;
			}

			if ((!CAMDv40 && (b & 0x00ff0000) != 0) ||
				(b & 0x00008100) != 0 ) {
				break;
			}
		}
	}

	return 1;
}

APTR Init(struct ExecBase *sysBase asm("a6")) {
	SysBase = sysBase;
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
		return FALSE;
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
		return FALSE;
	}

	Printf(LIBRARY_NAME " Initialized\n");

	pi.Transmit = transmitfunc;
	pi.Receive = receivefunc;
	pi.UserData = userdata;

	TimerInterrupt.is_Node.ln_Type = NT_INTERRUPT;
	TimerInterrupt.is_Node.ln_Pri = 0;
	TimerInterrupt.is_Node.ln_Name = "I2C timer";
	TimerInterrupt.is_Code = (void (*)(void))TimerInterruptCode;
	TimerInterrupt.is_Data = I2C_Base;

	SetupTimerDevice();

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
	TimerRequestActive = FALSE;
}

BOOL SetupTimerDevice() {
	ReplyMP = (struct MsgPort *)CreateMsgPort();
	if ( !ReplyMP ) {
		Printf("Could not create the reply port!\n");
		return FALSE;
	}
	ReplyMP->mp_Node.ln_Type = NT_MSGPORT;
	ReplyMP->mp_Flags = PA_SOFTINT;
	ReplyMP->mp_SigTask = (struct Task *)&TimerInterrupt;

	TimerIO = (struct timerequest *)CreateIORequest(ReplyMP, sizeof(struct timerequest));

	if ( TimerIO == NULL) {
		Printf("Out of memory.\n");
		return FALSE;
	}

	if ( OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)TimerIO, 0) != 0 ) {
		Printf("Unable to open 'timer.device'.\n");
		return FALSE;
	} else {
		TimerBase = (struct Device *)TimerIO->tr_node.io_Device;
	}

	return TRUE;
}

void MicroDelay(unsigned int val) {
	ReplyMP = (struct MsgPort *)CreateMsgPort();
	if ( !ReplyMP ) {
		//Printf("Could not create the reply port!\n");
		return;
	}

	TimerIO = (struct timerequest *)CreateIORequest(ReplyMP, sizeof(struct timerequest));

	if ( TimerIO == NULL) {
		//Printf("Out of memory.\n");
		return;
	}

	if ( OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)TimerIO, 0) != 0 ) {
		//Printf("Unable to open 'timer.device'.\n");
		return;
	} else {
		TimerBase = (struct Device *)TimerIO->tr_node.io_Device;
	}

	if ( TimerIO ) {
		TimerIO->tr_node.io_Command = TR_ADDREQUEST; /* Add a request.   */
		TimerIO->tr_time.tv_secs = 0;                /* 0 seconds.      */
		TimerIO->tr_time.tv_micro = val;             /* 'val' micro seconds. */
		DoIO((struct IORequest *)TimerIO);
		DeleteIORequest((struct IORequest *)TimerIO);
		TimerIO = NULL;
		CloseDevice((struct IORequest *)TimerIO);
	}

	if ( ReplyMP )
		DeleteMsgPort(ReplyMP);
}

void CleanUpTimerDevice() {
	if ( TimerIO != NULL) {
		CloseDevice((struct IORequest *)TimerIO);
	}

	if ( TimerIO ) {
		DeleteIORequest((struct IORequest *)TimerIO);
		TimerIO = NULL;
	}

	if ( ReplyMP ) {
		DeleteMsgPort(ReplyMP);
		ReplyMP = NULL;
	}
}
