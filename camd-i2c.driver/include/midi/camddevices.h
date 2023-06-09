#ifndef MIDI_CAMDDEVICES_H
#define MIDI_CAMDDEVICES_H

/************************************************************************
*     C. A. M. D.	(Commodore Amiga MIDI Driver)                       *
*************************************************************************
*									                                    *
*             Design & Development	- Roger B. Dannenberg				*
*			                        - Jean-Christophe Dhellemmes		*
*			                        - Bill Barton						*
*                                   - Darius Taghavy                    *
*                                                                       *
* Copyright 1990 by Commodore Business Machines							*
*************************************************************************
*
* camddevices.h	- MIDI device driver include file
*
************************************************************************/

#ifndef EXEC_TYPES_H
  #include <exec/types.h>
#endif

struct MidiPortData
{
    void (*ActivateXmit)(); /* function to activate transmitter interrupt when idle */
};

struct MidiDeviceData
{
    ULONG Magic;	    /* MDD_Magic */
    char *Name; 	    /* driver name */
    char *IDString;
    UWORD Version;
    UWORD Revision;

	BOOL (*Init)(struct ExecBase* SysBase asm("a6"));     /* called after LoadSeg() */
    void (*Expunge)(void);  /* called before UnLoadSeg() */
    struct MidiPortData *(*OpenPort)(
		struct MidiDeviceData *data asm("a3"),
		LONG portnum asm("d0"),
		ULONG (*transmitfunc)(APTR userdata asm("a2")) asm("a0"),
		void (*receivefunc)(UWORD input asm("d0"), APTR userdata asm("a2")) asm("a1"),
		APTR userdata asm("a2")
	);
    void (*ClosePort)(
		struct MidiDeviceData *data asm("a3"),
		LONG portnum asm("d0")
	);

    UBYTE NPorts;	    /* number of ports */
    UBYTE Flags;	    /* currently none */
    #ifdef __amigaos4__
    struct Library          *CamdDriverBase;
    struct CamdDriverIFace  *ICamdDriver;
    #endif
};

#define MDD_SegOffset	8   /* offset to structure in segment (past NextSeg and MOVEQ/RTS) */
#define MDD_Magic	((ULONG)'M' << 24 | (ULONG)'D' << 16 | 'E' << 8 | 'V')

#endif
