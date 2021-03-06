/*
 * writer.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _WRITER_H
#define _WRITER_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>

/* Environment passed when creating the thread */
typedef struct WriterEnv {
	Rendezvous_Handle hRendezvousInit;
	Rendezvous_Handle hRendezvousCleanup;
	Pause_Handle      hPauseProcess;
	Fifo_Handle       to_video_c;
	Fifo_Handle       from_video_c;
	Fifo_Handle       to_writelow_c;
	Fifo_Handle       from_writelow_c;
	Char             *videoFile;
	Int32             outBufSize;
} WriterEnv;

/* Thread function prototype */
extern Void *writerThrFxn(Void *arg);
/*LOW RATE Send thread*/
extern Void *writerLowThrFxn(Void *arg);

#endif /* _WRITER_H */
