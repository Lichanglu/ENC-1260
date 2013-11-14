/**
****************************************************
Copyright (C), 1988-1999, Reach Tech. Co., Ltd.

File name:     	main.c

Description:    编码模块程序的入口文件,完成编码的采集-->
				编码-->通过TCP连接发送

Date:     		2010-11-16

Author:	  		yangshh11111

version:  		V1.0

*****************************************************
*/
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <strings.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>
#include <xdc/std.h>
#include <sys/wait.h>

#include <ti/sdo/ce/trace/gt.h>
#include <ti/sdo/ce/CERuntime.h>
#include <ti/sdo/ce/utils/trace/TraceUtil.h>

#include <ti/sdo/dmai/Dmai.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Sound.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>

#include <semaphore.h>
#include <signal.h>

#include "common.h"
#include "sysparam.h"
#include "audio.h"
#include "video.h"
#include "capture.h"
#include "writer.h"
#include "ctrl.h"
#include "tcpcom.h"
#include "demo.h"
#include "ui.h"
#include "app_adjust_green.h"
#include "track.h"



#include "build_info.h"
#include "version.h"
#include "logo.h"
#include "app_logo_text.h"
#include "app_network.h"
#include "app_sdk_tcp.h"


/* Global variable declarations for this application */
GlobalData gbl = GBL_DATA_INIT;
/*音/视频参数表结构体*/
SavePTable  gSysParaT;
/*音/视频编码库参数设置表*/
VAENC_params gVAenc_p;
/*client param infomation*/
DSPCliParam gDSPCliPara[PORT_NUM];
/*FPGA update Flag*/
int g_FPGA_update = FPGA_NO_UPDATE;
ResizeParam gLowRate = INIT_LOWBIT_PARAM;
/*HV Table*/
HVTable gHVTable;
extern int gRemoteFD;
extern Logo_Handle gLogoinfo;
/* Thread priorities */
#define DETECT_THREAD_PRIORITY  	sched_get_priority_max(SCHED_FIFO) - 10
#define WRITER_THREAD_PRIORITY  	sched_get_priority_max(SCHED_FIFO) - 3
//#define SPEECH_THREAD_PRIORITY  	sched_get_priority_max(SCHED_FIFO) - 2
#define AUDIO_THREAD_PRIORITY   	sched_get_priority_max(SCHED_FIFO) - 1
#define VIDEO_THREAD_PRIORITY   	sched_get_priority_max(SCHED_FIFO) - 2
#define CAPTURE_THREAD_PRIORITY 	sched_get_priority_max(SCHED_FIFO)

#ifdef DSS_ENC_1100_1200
/*protocol struct*/
Protocol gProtocol;
#endif

extern int addtextdisplay(textinfo *text);
extern Int ModuleResize(Int argc, Char *argv[]);

#if 0
static void signal_handle(int signo)
{
	DEBUG(DL_DEBUG, "System signal(%d) received,exit \n", signo);
	gblSetQuit();
}

void signal_set(void)
{
	struct sigaction act, old_act;

	act.sa_handler = (void *)signal_handle;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if(sigaction(SIGFPE, &act, &old_act) < 0) {
		DEBUG(DL_DEBUG, "FATAL error for sigaction in function signal_set \n");
	}

	//   sigaction(SIGINT,&act,NULL);
	/*
	  sigaction(SIGKILL,&act,NULL);

	  sigaction(SIGQUIT,&act,NULL);

	  sigaction(SIGILL,&act,NULL);

	  sigaction(SIGSEGV,&act,NULL);

	  sigaction(SIGBUS,&act,NULL);

	  sigaction(SIGABRT,&act,NULL);

	  sigaction(SIGSYS,&act,NULL);

	  sigaction(SIGTERM,&act,NULL);
	//   sigaction(SIGLOST,&act,NULL);
	  sigaction(SIGXCPU,&act,NULL);
	    */
	/*网络死机*/
	signal(SIGPIPE, SIG_IGN);
	/*僵尸进程*/
	signal(SIGCHLD, sig_child_catch);


	return;
}
#endif
/*
##子进程捕捉回调函数
*/
void sig_chld(int signo)
{
	pid_t	pid;
	int 	stat;

	while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		DEBUG(DL_DEBUG, "child %d terminated\n", pid);
		pid = fork();

		if(pid < 0) {
			DEBUG(DL_DEBUG, "fork error\n");
			exit(1);
		}

		usleep(1000000);

		if(pid == 0) {
			//child
			usleep(1000000);
			ModuleResize(0, NULL);
		}
	}

	return;
}


typedef	void Sigfunc(int);

/*捕捉信号量*/
Sigfunc *signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if(signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}

	if(sigaction(signo, &act, &oact) < 0) {
		return(SIG_ERR);
	}

	return(oact.sa_handler);
}

/*信号包裹函数*/
Sigfunc *Signal(int signo, Sigfunc *func)
{
	Sigfunc	*sigfunc;

	if((sigfunc = signal(signo, func)) == SIG_ERR) {
		DEBUG(DL_ERROR, "ERROR:  signal error \n");
	}

	return(sigfunc);
}


/*
##create tcp communication task
*/
void CreateTCPTask(pthread_t *id_listen)
{
	int result;
	//pthread_t tmp;

	//pthread_t rtmpid ;
	result = pthread_create(&id_listen[PORT_ONE], NULL, (void *)TCPServerTask, (void *)NULL);

	if(result < 0) {
		DEBUG(DL_ERROR, "create DSP1TCPTask() failed\n");
		exit(0);
	}

	if(pthread_create(&id_listen[4], NULL, (void *)ListenSeries, (void *)NULL)) {
		DEBUG(DL_ERROR, "create ListenSeries() failed\n");
		exit(0);
	}

#ifdef DSS_ENC_1100_1200
	result = pthread_create(&id_listen[2], NULL, (void *)LCDModuleTask, (void *)NULL);

	if(result < 0) {
		DEBUG(DL_ERROR, "create DSP1TCPTask() failed\n");
		exit(0);
	}

#endif

	if(pthread_create(&id_listen[3], NULL, (void *)listenSdkTask, (void *)NULL)) {
		DEBUG(DL_ERROR, "create ListenSeries() failed\n");
		exit(0);
	}

	app_stream_output_init(gSysParaT.sysPara.dwAddr);
	mid_time_adjust_init();

}

pthread_mutex_t		gCapEnc_m;
extern int g_gpio_fd;
#ifdef DSS_ENC_1100_1200
extern Void *weblistenThrFxn(Void *arg);
extern int gRemoteFD;
#endif
/*
##############################################
##Int ModuleResize(Int argc, Char *argv[])
##############################################
*/
Int ModuleResize(Int argc, Char *argv[])
{
	Uns                 initMask            = 0;
	Int                 status              = EXIT_SUCCESS;
	Rendezvous_Attrs    rzvAttrs            = Rendezvous_Attrs_DEFAULT;
	Fifo_Attrs          fAttrs              = Fifo_Attrs_DEFAULT;
	Rendezvous_Handle   hRendezvousInit     = NULL;
	Rendezvous_Handle   hRendezvousWriter   = NULL;
	Rendezvous_Handle   hRendezvousCleanup  = NULL;
	Int                 numThreads			= 0;
	pthread_t 			id_listen[5] 		= {0};
	Void               *ret;

	char 				devicebuf[16] 		= {0};

	CaptureEnv          captureEnv;
	WriterEnv           writerEnv;
	WriterEnv			writerLowRateEnv;
	DetectEnv			detectEnv;
	VideoEnv            videoEnv;
	VideoEnv            videoLowRateEnv;//dd
	VideoEnv            LowRateResize;
	AudioEnv            audioEnv;
	CtrlEnv             ctrlEnv;
	char box_version[64] = {0};
	OutputVideoInfo		outputhandle;
	textinfo			*texthandle;
	int 				DHCPVAL = 0, tmp = 0;
	char 				gateway[255] = {0};

	struct sched_param  schedParam;
	pthread_t           captureThread;
	pthread_t           detectThread;
	pthread_t           writerThread;
	pthread_t			writerLowThread;
	pthread_t           videoThread;
	pthread_t           audioThread;
	pthread_t			videoLowThread;
	pthread_t			resizeLowThread;
#ifdef DSS_ENC_1100_1200
	pthread_t           webListenThread;
#endif
	pthread_attr_t      attr;
	int       index = 0;
	int result = 0;
	char ts_version[128] = {0};
	/* Zero out the thread environments */
	Dmai_clear(captureEnv);
	Dmai_clear(writerEnv);
	Dmai_clear(videoEnv);
	Dmai_clear(audioEnv);
	Dmai_clear(ctrlEnv);

	mid_task_init();
	trace_init();
	open_gpio_port();

	ts_build_get_version(ts_version, sizeof(ts_version));

	strcpy(box_version, BOX_VER);
	strcat(box_version, CODE_TYPE);
	strcat(box_version, DEUBG);
	printf("[%s] Module Encode Program %s V%s\n", CODE_COND, BOARD_TYPE, box_version);
	printf("the build time is %s,the git vesion is %s.the ts version is %s\n\n", g_make_build_date, _VERSION, ts_version);

	initMutexPthread();
	InitgblCommonMutex();
	InitSysParams();
	initOutputVideoParam();
	InitHVTable(&gHVTable);
	//	webgetDHCPFlag(tmp, &DHCPVAL);
	//	readDHCPValue(DHCPCONFIG_FILE, &DHCPVAL);
	//	setDHCPFlag(DHCPVAL);
	gLogoinfo = initLogoMod();
	initTextinfo();

	ReadEncodeParamTable(CONFIG_NAME, &gSysParaT);
	DHCPVAL = gSysParaT.sysPara.nTemp[0];
	printf("----mic=%x:%x:%x:%x:%x:%x\n",gSysParaT.sysPara.szMacAddr[0],gSysParaT.sysPara.szMacAddr[1],gSysParaT.sysPara.szMacAddr[2],
			gSysParaT.sysPara.szMacAddr[3],gSysParaT.sysPara.szMacAddr[4],gSysParaT.sysPara.szMacAddr[5]);

	ReadLowbitParamTable(LOWBIT_PARAM, &gSysParaT);

	memset(&outputhandle, 0, sizeof(OutputVideoInfo));
	getOutputvideohandle(&outputhandle);

	readOutputVideoParam(VIDEOENCODE_FILE, &outputhandle);

	setOutputvideohandle(&outputhandle);
	//sleep(10);
	ReadLogoinfo(LOGOCONFIGNAME, gLogoinfo);
	//setLogoInfoHandle(logoEnv);
	//sleep(10);
	texthandle = getTextInfoHandle();
	readTextFromfile(ADDTEXT_FILE, texthandle);
	//	DEBUG(DL_DEBUG, "%d,%d,%d,%d,%d,%d,%s\n", DHCPVAL, texthandle->xpos, texthandle->ypos,
	//	      texthandle->enable, texthandle->showtime, texthandle->alpha, texthandle->msgtext);
	//	sleep(10);
#ifdef DSS_ENC_1100_1200
	ReadProtocolIni(PROTOCOL_NAME, &gProtocol);
#endif

	ReadRemoteCtrlIndex(REMOTE_NAME, &index);
	/*Read I frames Interval*/
	ReadIframeInterval(IFRAMES_NAME);
	/*green Save Module*/
	app_init_green_adjust_module();
#ifdef CL4000_DVI
	app_init_screen_adjust_module();
#endif
	gblSetRemoteIndex(index);
	ReadHVTable(&gHVTable, 0);
	ReadHVTable(&gHVTable, 1);
#ifdef CL4000_DVI_SDI
	ReadIPParamTable(IP_PARAM, &gSysParaT);
#endif

	if(DHCPVAL) {
		printf("i will set dhcp.\n");
#if 1
		system("kill -1 `cat /var/run/dhcpcd-eth0.pid`");
		system("/sbin/dhcpcd eth0");
		system("ifconfig eth0");
#endif
		gSysParaT.sysPara.dwNetMark = GetNetmask("eth0");
		gSysParaT.sysPara.dwAddr = GetIPaddr("eth0");
		get_gateway(gateway);
		gSysParaT.sysPara.dwGateWay = 	get_gateway(gateway);
		DEBUG(DL_DEBUG, "gateway =%s\n", gateway);
	} else {
		printf("i will set static ip.\n");
		SetEthConfigIP(gSysParaT.sysPara.dwAddr, gSysParaT.sysPara.dwNetMark);
		SetEthConfigGW(gSysParaT.sysPara.dwGateWay);
	}

	system("ifconfig");

	strcpy(gSysParaT.sysPara.strVer, box_version);
	initSetParam();
	DEBUG(DL_DEBUG, "logo=%d text=%d ,texthandle->enable=%d,texthandle->showtime=%d\n", outputhandle.logo_show, outputhandle.text_show, texthandle->enable, texthandle->showtime);
#ifdef DSS_ENC_1100_1200
	/*open lcd initial*/
	OpenLCDCom(); //matchbox ++
	gblLoadIDX(); //matchbox ++

	if(-2 == ReadDeviceType(DTYPECONFIG_NAME, 1)) {
		ReadDeviceType(DTYPECONFIG_NAME, 0);
	}

	GetDeviceType(devicebuf);
	DEBUG(DL_DEBUG, "DTYPECONFIG_NAME gDeviceType = %s\n", devicebuf);
#endif

	/*取消PIPE坏的信号*/
	Signal(SIGPIPE, SIG_IGN);
	/* Set the priority of this whole process to max (requires root) */
	setpriority(PRIO_PROCESS, 0, -20);
	/*初始化高码流视频编码库参数*/
	InitVideoEncParams(&gSysParaT.videoPara[PORT_ONE]);
	/*初始化低码流视频编码库参数*/
	InitLowRateParams(&gSysParaT.videoPara[PORT_TWO]);
	/*初始化音频编码库参数*/
	InitAudioEncParams(&gSysParaT.audioPara[PORT_ONE]);
	/* Initialize the mutex which protects the global data */
	pthread_mutex_init(&gbl.mutex, NULL);
	/* Initialize Codec Engine runtime */
	CERuntime_init();
	/* Initialize Davinci Multimedia Application Interface */
	Dmai_init();
	closeWatchDog();

	mid_timer_init();
		
	initWatchDog();
#ifdef CL4000_DVI_SDI

	if(gblGetRemoteIndex() < MAX_FAR_CTRL_NUM) {
		result = InitRemoteStruct(gblGetRemoteIndex());
	}

	gRemoteFD = CameraCtrlInit(PORT_COM2);

	if(gRemoteFD <= 0) {
		DEBUG(DL_ERROR, "Initial CameraCtrlInit() Error\n");
	}

#else
#ifndef ENABLE_DEUBG

	if(gblGetRemoteIndex() < MAX_FAR_CTRL_NUM) {
		result = InitRemoteStruct(gblGetRemoteIndex());
	}

	gRemoteFD = CameraCtrlInit(PORT_COM1);

	if(gRemoteFD <= 0) {
		DEBUG(DL_ERROR, "Initial CameraCtrlInit() Error\n");
	}

#endif
#endif
	CreateTCPTask(id_listen);
	/* Initialize the logs. Must be done after CERuntime_init() */
	/*  if(TraceUtil_start(engine->engineName) != TRACEUTIL_SUCCESS)
		{
		    ERR("Failed to TraceUtil_start\n");
			cleanup(EXIT_FAILURE);
		}	*/
	//initMask |= LOGSINITIALIZED;
	app_set_logoshow_flag(outputhandle.logo_show);
	app_set_textshow_flag(outputhandle.text_show)	;
	//setShowLogoTextFlag(outputhandle->logotext);
	addtextdisplay(texthandle);
	/* Determine the number of threads needing synchronization */
	numThreads = 1;
	/*视频线程个数*/
	numThreads += 7;
	/*音频线程个数*/
	numThreads += 1;

	/* Create the objects which synchronizes the thread init and cleanup */
	hRendezvousInit = Rendezvous_create(numThreads, &rzvAttrs);
	hRendezvousCleanup = Rendezvous_create(numThreads, &rzvAttrs);
	hRendezvousWriter = Rendezvous_create(3, &rzvAttrs);

	if(hRendezvousInit == NULL ||
	   hRendezvousCleanup == NULL ||
	   hRendezvousWriter == NULL) {
		ERR("Failed to create Rendezvous objects\n");
		cleanup(EXIT_FAILURE);
	}

	/* Initialize the thread attributes */
	if(pthread_attr_init(&attr)) {
		ERR("Failed to initialize thread attrs\n");
		cleanup(EXIT_FAILURE);
	}

	/* Force the thread to use custom scheduling attributes */
	if(pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) {
		ERR("Failed to set schedule inheritance attribute\n");
		cleanup(EXIT_FAILURE);
	}

	/* Set the thread to be fifo real time scheduled */
	if(pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) {
		ERR("Failed to set FIFO scheduling policy\n");
		cleanup(EXIT_FAILURE);
	}


	/* Create the capture fifos */
	captureEnv.to_video_c = Fifo_create(&fAttrs);
	captureEnv.from_video_c = Fifo_create(&fAttrs);
	captureEnv.to_resize_c = Fifo_create(&fAttrs);
	captureEnv.from_resize_c = Fifo_create(&fAttrs);

	if(captureEnv.to_video_c == NULL || captureEnv.from_video_c == NULL ||
	   captureEnv.to_resize_c == NULL || captureEnv.from_resize_c == NULL) {
		ERR("Failed to open display fifos\n");
		cleanup(EXIT_FAILURE);
	}

	LowRateResize.to_videoresize_c = Fifo_create(&fAttrs);
	LowRateResize.from_videoresize_c = Fifo_create(&fAttrs);

	if(LowRateResize.to_videoresize_c == NULL || LowRateResize.from_videoresize_c == NULL) {
		ERR("Failed to open Resize fifos\n");
		cleanup(EXIT_FAILURE);
	}

	/* Set the capture thread priority */
	schedParam.sched_priority = CAPTURE_THREAD_PRIORITY;

	if(pthread_attr_setschedparam(&attr, &schedParam)) {
		ERR("Failed to set scheduler parameters\n");
		cleanup(EXIT_FAILURE);
	}

	/* Create the capture thread */
	captureEnv.hRendezvousInit    = hRendezvousInit;
	captureEnv.hRendezvousCleanup = hRendezvousCleanup;
	DEBUG(DL_DEBUG, "captureThrFxn thread!!!!\n");


	if(pthread_create(&captureThread, &attr, captureThrFxn, &captureEnv)) {
		ERR("Failed to create capture thread\n");
		cleanup(EXIT_FAILURE);
	}

	/* Create the writer fifos */
	writerEnv.to_video_c = Fifo_create(&fAttrs);
	writerEnv.from_video_c = Fifo_create(&fAttrs);
	writerLowRateEnv.to_writelow_c = Fifo_create(&fAttrs);
	writerLowRateEnv.from_writelow_c = Fifo_create(&fAttrs);

	if(writerEnv.to_video_c == NULL || writerEnv.from_video_c == NULL ||
	   writerLowRateEnv.to_writelow_c == NULL || writerLowRateEnv.from_writelow_c == NULL) {
		ERR("Failed to open display fifos\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= CAPTURETHREADCREATED ;
	/*detect thread*/
	detectEnv.hRendezvousInit	  = hRendezvousInit;
	detectEnv.hRendezvousCleanup  = hRendezvousCleanup;

	/* Set the video thread priority */
	schedParam.sched_priority = DETECT_THREAD_PRIORITY;

	if(pthread_attr_setschedparam(&attr, &schedParam)) {
		ERR("Failed to set scheduler parameters\n");
		cleanup(EXIT_FAILURE);
	}

	if(pthread_create(&detectThread, &attr, detectThrFxn, &detectEnv)) {
		ERR("Failed to create detect thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= DETECTTHREADCREATED ;

	/* Set the video thread priority */
	schedParam.sched_priority = VIDEO_THREAD_PRIORITY;

	if(pthread_attr_setschedparam(&attr, &schedParam)) {
		ERR("Failed to set scheduler parameters\n");
		cleanup(EXIT_FAILURE);
	}

	DEBUG(DL_DEBUG, "videoThrFxn thread!!!!\n");

	/* Create the video thread */
	videoEnv.hRendezvousInit    = hRendezvousInit;
	videoEnv.hRendezvousCleanup = hRendezvousCleanup;
	videoEnv.hRendezvousWriter  = hRendezvousWriter;
	videoEnv.to_capture    		= captureEnv.from_video_c;
	videoEnv.from_capture     	= captureEnv.to_video_c;
	videoEnv.to_writer     		= writerEnv.from_video_c;
	videoEnv.from_writer     	= writerEnv.to_video_c;
	videoEnv.videoEncoder       = engine->videoEncoders->codecName;
	videoEnv.engineName         = engine->engineName;


	if(pthread_create(&videoThread, &attr, videoThrFxn, &videoEnv)) {
		ERR("Failed to create video thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= VIDEOTHREADCREATED;

	/* Create the videoResize thread */
	videoLowRateEnv.hRendezvousInit    = hRendezvousInit;
	videoLowRateEnv.hRendezvousCleanup = hRendezvousCleanup;
	videoLowRateEnv.hRendezvousWriter  = hRendezvousWriter;
	videoLowRateEnv.to_resize		  = LowRateResize.from_videoresize_c;
	videoLowRateEnv.from_resize		  = LowRateResize.to_videoresize_c;
	videoLowRateEnv.from_writer    	  = writerLowRateEnv.to_writelow_c;
	videoLowRateEnv.to_writer          = writerLowRateEnv.from_writelow_c;
	videoLowRateEnv.videoEncoder       = engine->videoEncoders->codecName;
	videoLowRateEnv.engineName         = engine->engineName;
	DEBUG(DL_DEBUG, "videoLowRateThrFxn thread!!!!\n");

	if(pthread_create(&videoLowThread, &attr, videoLowRateThrFxn, &videoLowRateEnv)) {
		ERR("Failed to create video thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= VIDEOLOWRATETHREAD;

	/* Create the video thread */
	LowRateResize.hRendezvousInit    = hRendezvousInit;
	LowRateResize.hRendezvousCleanup = hRendezvousCleanup;
	LowRateResize.hRendezvousWriter  = hRendezvousWriter;
	LowRateResize.from_capture		 = captureEnv.to_resize_c;
	LowRateResize.to_capture		 = captureEnv.from_resize_c;
	LowRateResize.videoEncoder       = engine->videoEncoders->codecName;
	LowRateResize.engineName         = engine->engineName;
	/* Set the video thread priority */
	schedParam.sched_priority = VIDEO_THREAD_PRIORITY;

	if(pthread_attr_setschedparam(&attr, &schedParam)) {
		ERR("Failed to set scheduler parameters\n");
		cleanup(EXIT_FAILURE);
	}

	DEBUG(DL_DEBUG, "ResizeLowThrFxn thread!!!!\n");

	if(pthread_create(&resizeLowThread, &attr, ResizeLowThrFxn, &LowRateResize)) {
		ERR("Failed to create video thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= RESIZELOWRATETHREAD;

	Rendezvous_meet(hRendezvousWriter);

	/* Set the writer thread priority */
	schedParam.sched_priority = WRITER_THREAD_PRIORITY;

	if(pthread_attr_setschedparam(&attr, &schedParam)) {
		ERR("Failed to set scheduler parameters\n");
		cleanup(EXIT_FAILURE);
	}

	/* Create the writer thread */
	writerEnv.hRendezvousInit    = hRendezvousInit;
	writerEnv.hRendezvousCleanup = hRendezvousCleanup;
	writerEnv.outBufSize         = videoEnv.outBufSize;

	DEBUG(DL_DEBUG, "writerThrFxn thread!!!!\n");

	if(pthread_create(&writerThread, &attr, writerThrFxn, &writerEnv)) {
		ERR("Failed to create writer thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= WRITERTHREADCREATED;

	/* Create the writer thread */
	writerLowRateEnv.hRendezvousInit         = hRendezvousInit;
	writerLowRateEnv.hRendezvousCleanup      = hRendezvousCleanup;
	writerLowRateEnv.outBufSize              = videoLowRateEnv.outBufSize;

	DEBUG(DL_DEBUG, "writerLowThrFxn thread!!!!\n");

	if(pthread_create(&writerLowThread, &attr, writerLowThrFxn, &writerLowRateEnv)) {
		ERR("Failed to create writerResize thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= WRITELOWRATETHREAD;

	/* Set the thread priority */
	schedParam.sched_priority = AUDIO_THREAD_PRIORITY;

	if(pthread_attr_setschedparam(&attr, &schedParam)) {
		ERR("Failed to set scheduler parameters\n");
		cleanup(EXIT_FAILURE);
	}

	DEBUG(DL_DEBUG, "Audio thread Function!!!!\n");
	/* Create the audio thread */
	audioEnv.hRendezvousInit    = hRendezvousInit;
	audioEnv.hRendezvousCleanup = hRendezvousCleanup;
	audioEnv.engineName         = engine->engineName;
	audioEnv.audioEncoder       = engine->audioEncoders->codecName;

	if(pthread_create(&audioThread, &attr, audioThrFxn, &audioEnv)) {
		ERR("Failed to create speech thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= AUDIOTHREADCREATED;
#ifdef DSS_ENC_1100_1200

	if(pthread_create(&webListenThread, &attr, weblistenThrFxn, NULL)) {
		ERR("Failed to create web listen thread\n");
		cleanup(EXIT_FAILURE);
	}

	initMask |= WEBLISTENCREATED;
#endif
	/* Main thread becomes the control thread */
	ctrlEnv.hRendezvousInit    = hRendezvousInit;
	ctrlEnv.hRendezvousCleanup = hRendezvousCleanup;
	ctrlEnv.engineName         = engine->engineName;
	ret = ctrlThrFxn(&ctrlEnv);

	if(ret == THREAD_FAILURE) {
		status = EXIT_FAILURE;
	}

	DEBUG(DL_DEBUG, "Exit All Thread!!\n");
cleanup:

	/* Make sure the other threads aren't waiting for init to complete */
	if(hRendezvousWriter) {
		Rendezvous_force(hRendezvousWriter);
	}

	if(hRendezvousInit) {
		Rendezvous_force(hRendezvousInit);
	}

	DEBUG(DL_DEBUG, "EXIT Common Mutex!!!\n");
	DestorygblCommonMutex();
	DEBUG(DL_DEBUG, "EXIT pthread Mutex!!!\n");
	DestroyMutexPthread();

	if(initMask & AUDIOTHREADCREATED) {
		if(pthread_join(audioThread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				status = EXIT_FAILURE;
			}
		}
	}

	DEBUG(DL_DEBUG, "EXIT audio pThread!!!\n");

	if(initMask & VIDEOTHREADCREATED) {
		if(pthread_join(videoThread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				status = EXIT_FAILURE;
			}
		}
	}

	DEBUG(DL_DEBUG, "EXIT video pThread!!!\n");

	if(initMask & WRITERTHREADCREATED) {
		if(pthread_join(writerThread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				status = EXIT_FAILURE;
			}
		}
	}

	DEBUG(DL_DEBUG, "EXIT write pThread!!!\n");

	if(initMask & CAPTURETHREADCREATED) {
		if(pthread_join(captureThread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				status = EXIT_FAILURE;
			}
		}
	}

	DEBUG(DL_DEBUG, "EXIT capture pThread!!!\n");

	if(initMask & VIDEOLOWRATETHREAD) {
		if(pthread_join(videoLowThread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				status = EXIT_FAILURE;
			}
		}
	}

	if(initMask & RESIZELOWRATETHREAD) {
		if(pthread_join(resizeLowThread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				status = EXIT_FAILURE;
			}
		}
	}

	if(initMask & WRITELOWRATETHREAD) {
		if(pthread_join(writerLowThread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				status = EXIT_FAILURE;
			}
		}
	}

	if(pthread_join(id_listen[PORT_ONE], &ret) == 0) {
		if(ret == THREAD_FAILURE) {
			status = EXIT_FAILURE;
		}
	}

	if(captureEnv.to_video_c) {
		Fifo_delete(captureEnv.to_video_c);
	}

	if(captureEnv.from_video_c) {
		Fifo_delete(captureEnv.from_video_c);
	}

	if(captureEnv.to_resize_c) {
		Fifo_delete(captureEnv.to_resize_c);
	}

	if(captureEnv.from_resize_c) {
		Fifo_delete(captureEnv.from_resize_c);
	}

	if(writerEnv.to_video_c) {
		Fifo_delete(writerEnv.to_video_c);
	}

	if(writerEnv.from_video_c) {
		Fifo_delete(writerEnv.from_video_c);
	}

	if(writerLowRateEnv.from_writelow_c) {
		Fifo_delete(writerLowRateEnv.from_video_c);
	}

	if(writerLowRateEnv.to_writelow_c) {
		Fifo_delete(writerLowRateEnv.to_writelow_c);
	}

	if(LowRateResize.to_videoresize_c) {
		Fifo_delete(LowRateResize.to_videoresize_c);
	}

	if(LowRateResize.from_videoresize_c) {
		Fifo_delete(LowRateResize.from_videoresize_c);
	}

	DEBUG(DL_DEBUG, "EXIT Rendezvous cleanup pThread!!!\n");

	if(hRendezvousCleanup) {
		Rendezvous_delete(hRendezvousCleanup);
	}

	DEBUG(DL_DEBUG, "EXIT Rendezvous init pThread!!!\n");

	if(hRendezvousInit) {
		Rendezvous_delete(hRendezvousInit);
	}

	DEBUG(DL_DEBUG, "EXIT Rendezvous cleanup pThread!!!\n");
	/*
	    if (initMask & LOGSINITIALIZED) {
	        TraceUtil_stop();
		}	*/
	DEBUG(DL_DEBUG, "EXIT TraceUtil_stop !!!\n");
	pthread_mutex_destroy(&gbl.mutex);
	DEBUG(DL_DEBUG, "process EXIT!!!\n");
	exit(1);
}


/*
##############################################
##AppMain()
##############################################
*/
int AppMain(void)
{
	Int				status			= EXIT_SUCCESS;
	pid_t 			pid 			= 0;
	pid = fork();

	if(pid < 0) {
		DEBUG(DL_DEBUG, "fork error\n");
		exit(1);
	}

	if(pid == 0)  {  //child
		ModuleResize(0, NULL);
	}  else {
		Signal(SIGCHLD, sig_chld);

		//while(!gblGetQuit())
		while(1) {
			sleep(1);
		}
	}

	return status;
}




