/*
 * video.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _VIDEO_H
#define _VIDEO_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>

/* 线程创建输入环境变量 */
typedef struct VideoEnv {
	Rendezvous_Handle hRendezvousInit;
	Rendezvous_Handle hRendezvousCleanup;
	Rendezvous_Handle hRendezvousWriter;
	Pause_Handle      hPauseProcess;
	Fifo_Handle       to_writer;
	Fifo_Handle       from_writer;
	Fifo_Handle       to_capture;
	Fifo_Handle       from_capture;

	Fifo_Handle       to_videoresize_c;
	Fifo_Handle       from_videoresize_c;

	Fifo_Handle       to_resize;
	Fifo_Handle       from_resize;

	Char             *videoEncoder;
	Char             *engineName;
	Void             *params;
	Void             *dynParams;
	Int32             outBufSize;
	Int               videoBitRate;
	Int32             imageWidth;
	Int32             imageHeight;
} VideoEnv;

/* 视频编码线程定义 */
extern Void *videoThrFxn(Void *arg);
/*
 *****************************************************************************
 * videoLowRateThrFxn
 *****************************************************************************
*/
extern Void *videoLowRateThrFxn(Void *arg);
/*
 *****************************************************************************
 * ResizeLowThrFxn
 *****************************************************************************
*/
extern Void *ResizeLowThrFxn(Void *arg);
#endif /* _VIDEO_H */
