/*
 * capture.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _CAPTURE_H
#define _CAPTURE_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/Capture.h>

/* Environment passed when creating the thread */
typedef struct CaptureEnv {
	Rendezvous_Handle hRendezvousInit;
	Rendezvous_Handle hRendezvousCleanup;
	Rendezvous_Handle hRendezvousPrime;
	Fifo_Handle       from_video_c;
	Fifo_Handle       to_video_c;
	Fifo_Handle       from_resize_c;
	Fifo_Handle       to_resize_c;
} CaptureEnv;

typedef struct DetectEnv {
	Rendezvous_Handle hRendezvousInit;
	Rendezvous_Handle hRendezvousCleanup;
	Fifo_Handle hCaptureOutFifo ;
	Fifo_Handle hCaptureInFifo ;
	Fifo_Handle hWriterOutFifo ;
	Fifo_Handle hWriterInFifo ;
} DetectEnv ;

union revise {
	int value;
	struct __h_v__ {
		short h;
		short v;
	} h_v;
};

/*调节边框*/
extern int revise_picture(short h, short v);
/*VP口采集数据-YUV422格式*/
extern Void *captureThrFxn(Void *arg);
/******************************************************************************
 * DetectThrFxn
 ******************************************************************************/
extern void *detectThrFxn(Void *arg);

#endif /* _CAPTURE_H */

