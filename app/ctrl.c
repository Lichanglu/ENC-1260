/*
****************************************************
Copyright (C), 1988-1999, Reach Tech. Co., Ltd.

File name:     	ctrl.c

Description:    主要是统计当前帧率和码率

Date:     		2010-11-17

Author:	  		yangshh

version:  		V1.0

*****************************************************
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#include <xdc/std.h>

#include <ti/sdo/ce/Engine.h>

#include <ti/sdo/dmai/Ir.h>
#include <ti/sdo/dmai/Cpu.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Rendezvous.h>

#include "common.h"
#include "ctrl.h"
#include "demo.h"
#include "stream_output_struct.h"

/* How often to poll for new IR commands */
#define REMOTECONTROLLATENCY 300000
extern int g_FPGA_update;

extern int gVideoProcessFlag;

extern int gCaptureFlag;

int cpuload = 0, dspload = 0;


int getArmLoad(void)
{
	return cpuload;
}
void setArmLoad(int load)
{
	cpuload = load;
}

int getDspLoad(void)
{
	return dspload;
}
void setDspLoad(int load)
{
	dspload = load;
}

/******************************************************************************
 * drawDynamicData
 ******************************************************************************/
static Void drawDynamicData(Engine_Handle hEngine,
                            Cpu_Handle hCpu, OsdData *op)
{
	Char                  tmpString[200];
	struct timeval        tv;
	time_t                spentTime;
	ULong                 newTime;
	ULong                 deltaTime;
	Int                   armLoad;
	Int                   dspLoad;
	Int                   fps;
	Int                   videoKbps;
	Int                   soundKbps;
	Float                 fpsf;
	Float                 videoKbpsf;
	Float                 soundKbpsf;


#define TEST_RPINTF
#ifdef TEST_PRINTF
	float 		 tskbpsf;
	float 		rtpkbpsf;
	float			rtptskbpsf;

	int 			tskbps;
	int 			rtpkbps;
	int 			rtptskbps;
#endif
	int len = 0;

	op->time = -1;

	if(gettimeofday(&tv, NULL) == -1) {
		ERR("Failed to get os time\n");
		return;
	}

	newTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	if(!op->firstTime) {
		op->firstTime = newTime;
		op->prevTime = newTime;
		return;
	}

	/* Only update user interface every second */
	deltaTime = newTime - op->prevTime;

	if(deltaTime <= 1000) {
		return;
	}

	op->prevTime = newTime;

	spentTime = (newTime - op->firstTime) / 1000;

	if(spentTime <= 0) {
		return;
	}

	op->time     = spentTime;

	/* Calculate the frames per second */
	fpsf         = gblGetAndResetFrames() * 1000.0 / deltaTime;
	fps          = fpsf + 0.5;

	/* Calculate the video bit rate */
	videoKbpsf   = gblGetAndResetVideoBytesProcessed() * 8.0 / deltaTime;
	videoKbps    = videoKbpsf + 0.5;

	/* Calculate the audio or speech bit rate */
	soundKbpsf   = gblGetAndResetSoundBytesProcessed() * 8.0 / deltaTime;
	soundKbps    = soundKbpsf + 0.5;


#ifdef TEST_PRINTF
	/* Calculate the ts bit rate */
	tskbpsf = multicast_get_sbytes_process(TYPE_TS) * 8.0 / deltaTime;
	tskbps = tskbpsf + 0.5;

	/* Calculate the ts bit rate */
	rtpkbpsf = multicast_get_sbytes_process(TYPE_RTP) * 8.0 / deltaTime;
	rtpkbps = rtpkbpsf + 0.5;

	/* Calculate the ts bit rate */
	rtptskbpsf = multicast_get_sbytes_process(TYPE_TS_OVER_RTP) * 8.0 / deltaTime;
	rtptskbps = rtptskbpsf + 0.5;

#endif

	/* Get the local ARM cpu load */
	if(Cpu_getLoad(hCpu, &armLoad) < 0) {
		armLoad = 0;
		ERR("Failed to get ARM CPU load\n");
	}

	if(armLoad > 100) {
		armLoad = 100;
	}

	/* Get the DSP load */
	dspLoad = Engine_getCpuLoad(hEngine);

	if(dspLoad < 0) {
		dspLoad = 0;
		ERR("Failed to get DSP CPU load\n");
	}

	setArmLoad(armLoad);
	setDspLoad(dspLoad);
	len = sprintf(tmpString, "ARM load: %d%% DSP load: %d%%  Audio Rate: %dKbps Video Rate: %dKbps Current Frames: %dfps, flag = %d, cflag = %d",
	              armLoad, dspLoad, soundKbps, videoKbps, fps, gVideoProcessFlag, gCaptureFlag);
#ifdef TEST_PRINTF
	snprintf(tmpString + len, sizeof(tmpString) - len, "  ts: %dKbps rtp: %dKbps rtpts: %dKbps", tskbps, rtpkbps, rtptskbps);
	PRINTF("ts =%f,rtp=%f,rtpts=%f\n", (float)tskbps / (videoKbps + soundKbps), (float)rtpkbps / (videoKbps), (float)rtptskbps / (videoKbps + soundKbps));
#endif

	DEBUG(DL_DEBUG, "%s\n", tmpString);
	return;
}

/******************************************************************************
 * ctrlThrFxn
 ******************************************************************************/
Void *ctrlThrFxn(Void *arg)
{
	DEBUG(DL_DEBUG, "[ctrlThrFxn] start ...\n");
	CtrlEnv                *envp                = (CtrlEnv *) arg;
	Void                   *status              = THREAD_SUCCESS;
	OsdData                 osdData             = OSD_DATA_INIT;
	Cpu_Attrs               cpuAttrs            = Cpu_Attrs_DEFAULT;
	Engine_Handle           hEngine             = NULL;
	Cpu_Handle              hCpu                = NULL;

	/* Open the codec engine */
	hEngine = Engine_open(envp->engineName, NULL, NULL);

	if(hEngine == NULL) {
		ERR("Failed to open codec engine %s\n", envp->engineName);
		cleanup(THREAD_FAILURE);
	}

	/* Create the Cpu object to obtain ARM cpu load */
	hCpu = Cpu_create(&cpuAttrs);

	if(hCpu == NULL) {
		ERR("Failed to create Cpu Object\n");
	}

	/* Signal that initialization is done and wait for other threads */
	Rendezvous_meet(envp->hRendezvousInit);

	while(!gblGetQuit()) {
		if(IIS_FPGA_UPDATE()) {
			DEBUG(DL_DEBUG, "FPGA_UPDATE == g_FPGA_update %x\n", g_FPGA_update);

			if(app_get_reboot_flag() != 1) {
				writeWatchDog();
			}

			sleep(1);
			continue;
		}

		sleep(3);
		//	mid_timer_print();
		/* Update the dynamic data, either on the OSD or on the console */
		drawDynamicData(hEngine, hCpu, &osdData);
		/* Wait a while before polling the Ir and keyboard again */
		usleep(REMOTECONTROLLATENCY);
	}

cleanup:
	DEBUG(DL_DEBUG, "[ctrlThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX \n");
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	/* Meet up with other threads before cleaning up */
	Rendezvous_meet(envp->hRendezvousCleanup);

	/* Clean up the thread before exiting */
	if(hCpu) {
		Cpu_delete(hCpu);
	}

	if(hEngine) {
		Engine_close(hEngine);
	}

	DEBUG(DL_DEBUG, "[ctrlThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX end \n");
	return status;
}
