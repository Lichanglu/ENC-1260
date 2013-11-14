/*
 * ctrl.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _CTRL_H
#define _CTRL_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>

/* Defining infinite time */
#define FOREVER         -1


/* Environment passed when creating the thread */
typedef struct CtrlEnv {
	Rendezvous_Handle       hRendezvousInit;
	Rendezvous_Handle       hRendezvousCleanup;
	Pause_Handle            hPauseProcess;
	Int                     keyboard;
	Int                     time;
	Char                   *engineName;
} CtrlEnv;

/* Initial values of osd data structure */
#define OSD_DATA_INIT        { -1, 0, 0, 0, 0, 0 }


/* Structure containing the state of the OSD */
typedef struct OsdData {
	Int           time;
	ULong         firstTime;
	ULong         prevTime;
	Int           samplingFrequency;
	Int           imageWidth;
	Int           imageHeight;
} OsdData;


extern int getArmLoad(void);
extern void setArmLoad(int load);
extern int getDspLoad(void);
extern void setDspLoad(int load);
/* Thread function prototype */
extern Void *ctrlThrFxn(Void *arg);

#endif /* _CTRL_H */
