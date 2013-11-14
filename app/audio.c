/*
****************************************************
Copyright (C), 1988-1999, Reach Tech. Co., Ltd.

File name:     	audio.c

Description:    音频采集编码线程

Date:     		2010-11-17

Author:	  		yangshh

version:  		V1.0

*****************************************************
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <xdc/std.h>

#include <ti/sdo/ce/Engine.h>
#include <linux/soundcard.h>

#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/Sound.h>
#include <ti/sdo/dmai/Buffer.h>
#include <ti/sdo/dmai/Loader.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/ce/Aenc1.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>


#include "audio.h"
#include "iaacenc.h"
#include "sysparam.h"
#include "common.h"
#include "tcpcom.h"
#include "demo.h"
#include "log_common.h"

#include "app_stream_output.h"

//#define PCM_DETECT

#define LAGC_CTRL_A			26
#define LAGC_CTRL_B			27
#define LAGC_CTRL_C			28
#define RAGC_CTRL_A			29
#define RAGC_CTRL_B			30
#define RAGC_CTRL_C			31

#ifdef PCM_DETECT
#include "mid_mutex.h"
#include "mid_ringbuff.h"
#include "log_common.h"
#endif

#define AIC32_REG_RIGHT_ADDR 26
#define AIC32_REG_LEFT_ADDR 29
#define AIC32_REG_LEFT_MIC_ADDR 15
#define AIC32_REG_RIGHT_MIC_ADDR 16



/*视/音频编码参数表*/
extern VAENC_params gVAenc_p;
extern int g_FPGA_update;
#define MIXER_DEVICE    "/dev/mixer"


extern unsigned long long app_get_start_time();

#ifdef PCM_DETECT
static int audio_get_data(ReachRingBuffer *ring, char *ptr, int *readlen);
static int audio_get_data_size(ReachRingBuffer *ring);
static int audio_push_data(ReachRingBuffer *ring , char *ptr, int len);
#endif
extern int g_gpio_fd;
void SetAIC32Reg(int fd, int AicReg, int RegValue)
{
	struct aic3x_regval aic32;
	aic32.reg = AicReg;
	aic32.val = RegValue;
	ioctl(fd, GPIO_S_AIC32_REG, &aic32);
}

static int WriteToFile(char *buf, int len)
{
	FILE *fp;
	fp = fopen("audio_00.pcm", "a+");

	if(fp == NULL) {
		DEBUG(DL_ERROR, "Open File Failed!!!\n");
		return -1;
	}

	fwrite(buf, len, 1, fp);
	fclose(fp);
	return 0;
}


//the val is from 0-30
int changeVoiceVal(int oldVal, int *newVal)
{
	switch(oldVal) {
		case 0:
		case 1:
		case 2:
		case 3:
			*newVal = 0xf0;
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			*newVal = 0xe0;
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			*newVal = 0xd0;
			break;

		case 12:
		case 13:
		case 14:
		case 15:
			*newVal = 0xc0;
			break;

		case 16:
		case 17:
		case 18:
		case 19:
			*newVal = 0xb0;
			break;

		case 20:
		case 21:
		case 22:
		case 23:
			*newVal = 0xa0;
			break;

		case 24:
		case 25:
		case 26:
		case 27:
			*newVal = 0x90;
			break;

		case 28:
		case 29:
		case 30:
		case 31:
			*newVal = 0x80;
			break;

		default:
			break;

	}
}


void ChangeVolume(VAENC_params VAenc_p)
{
	PRINTF("VAenc_p.aenc.inputmode =%d,%d,%d\n", VAenc_p.aenc.inputmode , Sound_Input_LINE, Sound_Input_MIC);

	if(VAenc_p.aenc.inputmode == Sound_Input_LINE) {
		SetAIC32Reg(g_gpio_fd, AIC32_REG_RIGHT_ADDR, 0xF0 - 16 * (VAenc_p.aenc.rvolume / 4)); //右声道音量
		SetAIC32Reg(g_gpio_fd, AIC32_REG_LEFT_ADDR, 0xF0 - 16 * (VAenc_p.aenc.lvolume / 4)); //左声道音量
		SetAIC32Reg(g_gpio_fd, AIC32_REG_LEFT_MIC_ADDR, 0x1f);
		SetAIC32Reg(g_gpio_fd, AIC32_REG_RIGHT_MIC_ADDR, 0x1f);
		PRINTF("\n-----------------------g_gpio_fd=%d \n\n",g_gpio_fd);
		PRINTF("line in r value =%d,l value=%d \n", 0xF0 - 16 * (VAenc_p.aenc.rvolume / 4), 0xF0 - 16 * (VAenc_p.aenc.lvolume / 4));

	} else if(VAenc_p.aenc.inputmode == Sound_Input_MIC) {
		SetAIC32Reg(g_gpio_fd, AIC32_REG_RIGHT_ADDR, 0x70); //右声道音量
		SetAIC32Reg(g_gpio_fd, AIC32_REG_LEFT_ADDR, 0x70); //左声道音量
		SetAIC32Reg(g_gpio_fd, AIC32_REG_LEFT_MIC_ADDR, VAenc_p.aenc.lvolume);    //右声道音量
		SetAIC32Reg(g_gpio_fd, AIC32_REG_RIGHT_MIC_ADDR, VAenc_p.aenc.rvolume);    //左声道音量
		PRINTF("mic in r value =%d,l value=%d \n", VAenc_p.aenc.lvolume, VAenc_p.aenc.rvolume);

	}
}


/*
##
##check video encode param set function runtime
##return:  1: set params  0: noset params
##
*/
int AencRunTimeFxn(AENC_params *pAenc, Sound_Attrs *sAttr, IAACENC_Params *params, AUDENC1_DynamicParams *dynParams)
{
	if(H264_ENC_PARAM_FLAG == pAenc->uflag) {
		/*mic/line change*/
		sAttr->soundInput = pAenc->inputmode;
		/*		if(Sound_Input_MIC == sAttr->soundInput) {
					set_gpio_bit(AUDIO_MIC_MODE);  //
				}*/
		sAttr->sampleRate  = pAenc->samplerate;

		if(sAttr->sampleRate < 44100) {
			sAttr->sampleRate = 44100;
		}

		if(sAttr->sampleRate > 48000) {
			sAttr->sampleRate = 48000;
		}

		if(pAenc->bitrate < 64000) {
			pAenc->bitrate = 64000;
		}

		if(pAenc->bitrate > 128000) {
			pAenc->bitrate = 128000;
		}

		/*sample rate*/
		gblSetSamplingFrequency(pAenc->samplerate);
		params->audenc_params.sampleRate = dynParams->sampleRate = pAenc->samplerate;
		/*audio bit rate*/
		params->audenc_params.bitRate = dynParams->bitRate = pAenc->bitrate;
		pAenc->uflag = 0;
	} else {
		return 0;
	}

	return 1;
}

/******************************************************************************
 * audioThrFxn
 ******************************************************************************/
Void *audioThrFxn(Void *arg)
{
	PRINTF("get pid= %d\n", getpid());
	DEBUG(DL_DEBUG, "[audioThrFxn] start ...\n");
	AudioEnv               *envp                = (AudioEnv *) arg;
	Void                   *status              = THREAD_SUCCESS;
	Sound_Attrs             sAttrs              = Sound_Attrs_STEREO_DEFAULT;
	Buffer_Attrs            bAttrs              = Buffer_Attrs_DEFAULT;
	AUDENC1_Params          defaultParams       = Aenc1_Params_DEFAULT;
	AUDENC1_DynamicParams   defaultDynParams    = Aenc1_DynamicParams_DEFAULT;
	Engine_Handle           hEngine             = NULL;
	Sound_Handle            hSound              = NULL;
	Aenc1_Handle            hAe1                = NULL;
	Buffer_Handle           hOutBuf             = NULL;
	Buffer_Handle           hInBuf              = NULL;
	Buffer_Handle			hInMuteBuf			= NULL;
	int 					paramRet 			= 0;
	AUDENC1_Params         *params;
	AUDENC1_DynamicParams  *dynParams;
	IAACENC_Params     		aacparam ;
	UINT8                   oldLvol = 0;
	UINT8					oldRvol = 0;
	UINT8					curLvol, curRvol;
	int 					newLvol, newRvol;
	int						i;
	Buffer_Handle hTempBuf = NULL;
	char *temp = NULL;

	unsigned int sound_time = 0;
	unsigned long long rtp_time = 0;
	unsigned long long start_time = 0;
	unsigned long long last_time = 0;
	
	APP_AUDIO_DATA_INFO info;
/*
	unsigned int test_aac_num = 0;
	unsigned long old_time = 0;
	unsigned long test_begin_time = 0;
*/
#ifdef PCM_DETECT
	ReachRingBuffer *ringbuff = NULL;
	char *temp = NULL;
	int soundreadlen = 0;
#endif
	int config_change = 0;
	//FILE *fp;
	//fp = fopen("audio4.2.7.pcm","wb");
	sAttrs.soundInput = Sound_Input_LINE;
	DEBUG(DL_DEBUG, "Audio envp->engineName = %s\n", envp->engineName);
	/* Open the codec engine */
	hEngine = Engine_open(envp->engineName, NULL, NULL);

	if(hEngine == NULL) {
		DEBUG(DL_ERROR, "Failed to open codec engine %s\n", envp->engineName);
		//cleanup(THREAD_FAILURE);
		system("reboot -f");
	}

	/* Use supplied params if any, otherwise use defaults */
	params = &defaultParams;
	dynParams = &defaultDynParams;
	envp->sampleRate = gVAenc_p.aenc.samplerate;
	envp->soundBitRate = gVAenc_p.aenc.bitrate;
	sAttrs.soundInput = gVAenc_p.aenc.inputmode;
	params->sampleRate = dynParams->sampleRate = envp->sampleRate;
	params->bitRate = dynParams->bitRate = envp->soundBitRate ;

	DEBUG(DL_DEBUG, "Sample Rate = %d  bitrate =%d inputmod = %d\n",
	      envp->sampleRate, envp->soundBitRate, sAttrs.soundInput);
	aacparam.audenc_params = *params;
	aacparam.outObjectType = AACENC_OBJ_TYP_HEAAC ; //AACENC_OBJ_TYP_HEAAC;
	aacparam.outFileFormat = AACENC_TT_ADTS;
	aacparam.bitRateMode = AACENC_BR_MODE_VBR_5;
	aacparam.usePns  = AACENC_TRUE;
	aacparam.useTns  = AACENC_TRUE;
	aacparam.ancRate = -1;
	aacparam.downMixFlag   = AACENC_FALSE;
	aacparam.audenc_params.size = sizeof(IAACENC_Params);
	hAe1 = Aenc1_create(hEngine, envp->audioEncoder, (AUDENC1_Params *)&aacparam, dynParams);

	if(hAe1 == NULL) {
		DEBUG(DL_ERROR, "Failed to create audio encoder: %s\n", envp->audioEncoder);
		cleanup(THREAD_FAILURE);
	}

	/* Ask the codec how much space it needs for output data */
	hOutBuf = Buffer_create(Aenc1_getOutBufSize(hAe1), &bAttrs);
	/* Ask the codec how much input data it needs */
	hInBuf = Buffer_create(Aenc1_getInBufSize(hAe1), &bAttrs);

	if(hInBuf == NULL || hOutBuf == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate audio buffers\n");
		cleanup(THREAD_FAILURE);
	}

	hInMuteBuf = Buffer_create(Aenc1_getInBufSize(hAe1), &bAttrs);

	if(hInMuteBuf == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate audio buffers\n");
		cleanup(THREAD_FAILURE);
	}

	memset(Buffer_getUserPtr(hInMuteBuf), 0, Aenc1_getInBufSize(hAe1));
#ifdef PCM_DETECT
	hTempBuf = Buffer_create(Aenc1_getInBufSize(hAe1), &bAttrs);

	if(hTempBuf == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate audio buffers\n");
		cleanup(THREAD_FAILURE);
	}

	memset(Buffer_getUserPtr(hTempBuf), 0, Aenc1_getInBufSize(hAe1));

	PRINTF("Aenc1_getInBufSize size = %d\n", Aenc1_getInBufSize(hAe1));
	ringbuff = mid_create_ringbuff(Aenc1_getInBufSize(hAe1) * 4, 95);

	if(ringbuff == NULL) {
		PRINTF("Failed to allocate audio buffers\n");
		cleanup(THREAD_FAILURE);
	}

#endif

	/* Create the sound device */
	sAttrs.sampleRate = envp->sampleRate;
	sAttrs.mode = Sound_Mode_INPUT;
	sAttrs.leftGain = 0;
	sAttrs.rightGain = 0;
	sAttrs.channels = 2;
	hSound = Sound_create(&sAttrs);
	config_change = 1;

	if(hSound == NULL) {
		DEBUG(DL_ERROR, "Failed to create audio device\n");
		cleanup(THREAD_FAILURE);
	}

	change_mic_linein_input(sAttrs.soundInput);
#ifdef DSS_ENC_1100_1200
	ChangeVolume(gVAenc_p);
#endif
	/* Signal that initialization is done and wait for other threads */
	Rendezvous_meet(envp->hRendezvousInit);


#ifdef PCM_DETECT
	unsigned long t1, t2;
	int templen = 0;
	int readnum = 0;
	unsigned long readlen = 0;

	unsigned long  rate = 0;
	unsigned long  utime = 0;
	unsigned long oldsamplerate = 0;
	unsigned long needlen = 0;


	
	t1 = t2  = 0;
	t1 =	get_run_time();
	oldsamplerate =     sAttrs.sampleRate;
#endif

//modify lichl
//	test_log_server_init();

	while(!gblGetQuit()) {
		if(IIS_FPGA_UPDATE()) {
			sleep(1);
			continue;
		}



		/* Read samples from the Sound device */
		if(Sound_read(hSound, hInBuf) < 0) {
			ERR("Failed to read audio buffer\n");

			cleanup(THREAD_FAILURE);
		}



		sound_time =get_run_time();


		start_time =  app_get_start_time();
		rtp_time = getCurrentTime();
		if(rtp_time < start_time)
		{
			PRINTF("warnning,the rtp_time=%lld,the start time = %lld\n",rtp_time,start_time);
			app_set_start_time();
			start_time =app_get_start_time();
			rtp_time = 0;
		}
		else
		{
			rtp_time = rtp_time -start_time;
		}

		if(last_time > rtp_time)
		{
			rtp_time = last_time + 1;
			//last_time = last_time; 
		}

/*	
		test_aac_num++;
		
		if(test_aac_num == 600)
		{
			old_time = getCurrentTime();
			test_begin_time = getCurrentTime();
		}
		if(test_aac_num % 200 == 0 &&test_aac_num>= 600)
		{
			printf("12test_aac_num =%d==time=%u=\n",test_aac_num,(getCurrentTime()-test_begin_time));
		//	system("date;hwclock --show");
			printf("=%u=%u==%d\n",test_aac_num,((test_aac_num - 600)*1024/24) ,Buffer_getNumBytesUsed(hInBuf));
		//	system("hwclock -s");
			//system("hwclock -s"); //
			
		}
		
		test_server_send_audio_test((char *)Buffer_getUserPtr(hInBuf),Buffer_getNumBytesUsed(hInBuf));
*/				
		
		curLvol = gVAenc_p.aenc.lvolume;
		curRvol = gVAenc_p.aenc.rvolume;

		if(curLvol == 0 && curRvol == 0) {
			temp = (char *)Buffer_getUserPtr(hInBuf);
			memset(temp, 0, Buffer_getNumBytesUsed(hInBuf));
		}

#ifdef PCM_DETECT

		if(1) {

			t2 =	get_run_time();
			//	PRINTF("t1 = %u,t2=%u\n",t1,t2);
			readnum ++;
			readlen += Buffer_getNumBytesUsed(hInBuf);

			if(t2 - t1 > 5000) {
				if(sAttrs.sampleRate != oldsamplerate) {
					oldsamplerate = sAttrs.sampleRate;
					t1 = t2;
					rate = 0;
					readlen = 0;
					readnum = 0;
					PRINTF("DEBUG,the smaplerate is changel ,the old rate is %d,the new is  %d\n", oldsamplerate, sAttrs.sampleRate);
				} else {
					utime = t2 - t1;
					t1 = get_run_time();
					rate = readlen * 8 * 1000 / utime;
					needlen = (oldsamplerate / 100) * 16 * 2 * utime / 80 ;

					//PRINTF("I use the time %u s,the readlen = %u,the rate = %u,readnum=%d\n",utime,readlen*8,rate,readnum);
					if(needlen < readlen) {
						PRINTF("I use the time %u s,the readlen = %u,the rate = %u,readnum=%d\n", utime, readlen * 8, rate, readnum);
						templen = Buffer_getNumBytesUsed(hInBuf) - (readlen - needlen);
						PRINTF("i need the len = %u,i get the len = %u,THE ONE=%d=%d\n", needlen, readlen, Buffer_getNumBytesUsed(hInBuf), templen);

						if(templen > 0) {
							//Buffer_setNumBytesUsed(hInBuf,templen);
							audio_push_data(ringbuff, Buffer_getUserPtr(hInBuf), templen);
							PRINTF("audio_get_data_size() ==%d\n", audio_get_data_size(ringbuff));
						} else {
							audio_push_data(ringbuff, Buffer_getUserPtr(hInBuf), Buffer_getNumBytesUsed(hInBuf));
						}
					} else {
						soundreadlen = audio_get_data_size(ringbuff);
						PRINTF("readlen = %d\n", soundreadlen);
						audio_get_data(ringbuff, temp, &soundreadlen);
						PRINTF("readlen = %d\n", soundreadlen);
						audio_push_data(ringbuff, Buffer_getUserPtr(hInBuf), Buffer_getNumBytesUsed(hInBuf));
					}

					readlen = 0;
					readnum = 0;
				}
			} else {
				audio_push_data(ringbuff, Buffer_getUserPtr(hInBuf), Buffer_getNumBytesUsed(hInBuf));
			}

			//	sleep(10);
			//PRINTF("\n");
			soundreadlen = Aenc1_getInBufSize(hAe1);

			//PRINTF("audio_get_data_size() =%d=%d\n",soundreadlen,audio_get_data_size( ringbuff) );
			if(audio_get_data_size(ringbuff) < soundreadlen) {
				PRINTF("ERROR,the audio have data =%d,need data =%d==%d\n", audio_get_data_size(ringbuff), soundreadlen, Buffer_getNumBytesUsed(hInBuf));
				continue;
			}

			temp = (char *)Buffer_getUserPtr(hTempBuf);
			audio_get_data(ringbuff, temp, &soundreadlen);

			if(soundreadlen != 8192) {
				PRINTF("readlen = %d\n", soundreadlen);
			}

			Buffer_setNumBytesUsed(hTempBuf, soundreadlen);

		}

#endif
		paramRet = AencRunTimeFxn(&gVAenc_p.aenc, &sAttrs, &aacparam, dynParams);

		if(paramRet) {
			/*clear sound card handle*/
			if(hSound) {
				Sound_delete(hSound);
			}

			hSound = Sound_create(&sAttrs);
			config_change = 1;

			if(hSound == NULL) {
				DEBUG(DL_ERROR, "Failed to create the sound device\n");
				cleanup(THREAD_FAILURE);
			}

			change_mic_linein_input(sAttrs.soundInput);

#ifdef DSS_ENC_1100_1200
			ChangeVolume(gVAenc_p);
#endif

			/* Clean up the thread before exiting */
			if(hAe1) {
				Aenc1_delete(hAe1);
				hAe1 = NULL;
			}

			gblEncodeLcok();
			hAe1 = Aenc1_create(hEngine, envp->audioEncoder, (AUDENC1_Params *)&aacparam, dynParams);
			gblEncodeunLcok();

			if(hAe1 == NULL) {
				DEBUG(DL_ERROR, "Failed to create audio encoder: %s\n", envp->audioEncoder);
				cleanup(THREAD_FAILURE);
			}
		}

#ifdef PCM_DETECT

		/* Encode the audio buffer */
		if(START == gblGetMuteStatus()) {
			Buffer_setNumBytesUsed(hInMuteBuf, Buffer_getNumBytesUsed(hTempBuf));

			/* Encode the audio buffer */
			if(Aenc1_process(hAe1, hInMuteBuf, hOutBuf) < 0) {
				DEBUG(DL_ERROR, "Failed to encode audio buffer\n");
				cleanup(THREAD_FAILURE);
			}
		} else {
			/* Encode the audio buffer */
			if(Aenc1_process(hAe1, hTempBuf, hOutBuf) < 0) {
				DEBUG(DL_ERROR, "Failed to encode audio buffer\n");
				cleanup(THREAD_FAILURE);
			}
		}

#else

		/* Encode the audio buffer */
		if(START == gblGetMuteStatus()) {
			Buffer_setNumBytesUsed(hInMuteBuf, Buffer_getNumBytesUsed(hInBuf));

			/* Encode the audio buffer */
			if(Aenc1_process(hAe1, hInMuteBuf, hOutBuf) < 0) {
				DEBUG(DL_ERROR, "Failed to encode audio buffer\n");
				cleanup(THREAD_FAILURE);
			}
		} else {
			//DEBUG(DL_DEBUG, "Buffer_getNumBytesUsed(hInBuf) = %d\n",Buffer_getNumBytesUsed(hInBuf));
			//fwrite((unsigned char *)Buffer_getUserPtr(hInBuf),Buffer_getNumBytesUsed(hInBuf),1,fp);
			/* Encode the audio buffer */
			if(Aenc1_process(hAe1, hInBuf, hOutBuf) < 0) {
				DEBUG(DL_ERROR, "Failed to encode audio buffer\n");
				cleanup(THREAD_FAILURE);
			}
		}

#endif

		/* Write encoded buffer to the speech file */
		if(Buffer_getNumBytesUsed(hOutBuf)) {
			SendAudioToClient(Buffer_getNumBytesUsed(hOutBuf), (unsigned char *)Buffer_getUserPtr(hOutBuf), 0, 0, dynParams->sampleRate);
#if 0
#ifdef DSS_ENC_1100_1200

			if(config_change == 1) {
				PRINTF("the audio changed ,update audio sdp.\n");
				rtsp_porting_audio_filter_sdp_info((unsigned char *)Buffer_getUserPtr(hOutBuf), sAttrs.channels, sAttrs.sampleRate, 0);
				config_change = 0;
			}

			//f(GetMultStatus(TS_STREAM) || (GetMultStatus(RTSP_STREAM) && rtsp_ts_get_client_num() > 0)) {

			//PRINTF("%d\n");
			//TsAudioPack(Buffer_getNumBytesUsed(hOutBuf), (unsigned char *)Buffer_getUserPtr(hOutBuf));
			if(app_ts_get_active() == 1) {
				app_ts_build_audio(Buffer_getNumBytesUsed(hOutBuf), (unsigned char *)Buffer_getUserPtr(hOutBuf));
			}

			//	if(GetMultStatus(RTP_STREAM)) {
			if(app_rtp_get_active() == 1) {
				rtp_build_audio_data(Buffer_getNumBytesUsed(hOutBuf), (unsigned char *)Buffer_getUserPtr(hOutBuf), sAttrs.sampleRate, app_get_direct_rtp_mtu());
			}


			//	if(GetMultStatus(RTMP_STREAM) && RtmpGetClientNum()) {
			//		RtmpAudioPack(Buffer_getNumBytesUsed(hOutBuf), (unsigned char *)Buffer_getUserPtr(hOutBuf));
			//	}

#endif
#else
			memset(&info, 0, sizeof(info));
			info.channel = sAttrs.channels;
			info.samplerate = sAttrs.sampleRate;
			info.recreate = config_change;
			info.timestamp = sound_time;
			info.rtp_time = rtp_time&0xffffffff;
			last_time = rtp_time;
			
			app_set_media_info(-1, -1, info.samplerate , 0);
			app_stream_audio_output((unsigned char *)Buffer_getUserPtr(hOutBuf), Buffer_getNumBytesUsed(hOutBuf), &info);
			config_change = 0;
#endif
		} else {
			DEBUG(DL_WARNING, "Warning, zero bytes audio encoded\n");
		}

		/* Increment the number of bytes encoded for the user interface */
		gblIncSoundBytesProcessed(Buffer_getNumBytesUsed(hOutBuf));

	}

cleanup:
	DEBUG(DL_DEBUG, "[audioThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX \n");
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	/* Meet up with other threads before cleaning up */
	Rendezvous_meet(envp->hRendezvousCleanup);

	/* Clean up the thread before exiting */
	if(hAe1) {
		Aenc1_delete(hAe1);
		hAe1 = NULL;
	}

	if(hSound) {
		Sound_delete(hSound);
		hSound = NULL;
	}

	if(hInBuf) {
		Buffer_delete(hInBuf);
	}

	if(hOutBuf) {
		Buffer_delete(hOutBuf);
	}

	if(hEngine) {
		Engine_close(hEngine);
		hEngine = NULL;
	}

	DEBUG(DL_DEBUG, "[audioThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX end\n");
	return status;
}



#ifdef PCM_DETECT
static int audio_push_data(ReachRingBuffer *ring , char *ptr, int len)
{
	//return 1;
	int ret = 0;
	//PRINTF("ptr = %p,len=%d\n",ptr,len);
	ret = mid_push_ringbuff(ring, ptr, len);
	return ret ;

}

static int audio_get_data(ReachRingBuffer *ring, char *ptr, int *readlen)
{
	int ret = 0;
	int len = 0;
	ret = mid_get_ringbuff(ring, ptr, *readlen, &len);
	*readlen = len;
	//PRINTF("ptr = %p,len=%d\n",ptr,len);
	return ret ;

}

static int audio_get_data_size(ReachRingBuffer *ring)
{

	int len = 0;
	len =  mid_get_usesize_ringbuff(ring);
	//	PRINTF("len=%d\n",len);
	return len ;

}
#endif
