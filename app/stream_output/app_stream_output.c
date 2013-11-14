/******************************************************************************
*  用于统一给app对接的函数
*
*
*
*
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

#include "mid_timer.h"

#include "rtsp_server.h"
#include "rtp_build.h"
#include "rtp_ts_build.h"
#include "ts_build.h"

#include "stream_output_struct.h"
#include "stream_output_client.h"
#include "multicast_output_server.h"
#include "rtsp_output_server.h"
#include "traffic_shaping.h"
#include "app_stream_output.h"
#include "rtsp_porting_sdp_info.h"

#include "log_common.h"


static void app_build_ts_audio(unsigned char *buff, int len, unsigned int timestamp, APP_MEDIA_INFO *info);
static void app_build_ts_video(unsigned char *buff, int len , unsigned int timestamp, APP_MEDIA_INFO *info);
static void app_build_ts_init();

/*ts send*/
static void *g_ts_build_handle = NULL;

static void app_build_ts_init()
{
	if(g_ts_build_handle != NULL) {
		PRINTF("Warnning,\n");
		return ;
	}

	g_ts_build_handle = ts_build_init(0x52, 0x43);

	if(g_ts_build_handle == NULL) {
		PRINTF("Errot,the g_ts_build_handle is NULL\n");
		return ;
	}

	return ;
}

static void app_build_ts_video(unsigned char *buff, int len , unsigned int timestamp, APP_MEDIA_INFO *info)
{
	if(g_ts_build_handle == NULL) {
		PRINTF("Error.\n");
		return ;
	}

	ts_build_video_data(g_ts_build_handle, buff, len, timestamp, (void *)info);
	return;
}

/*==============================================================================
	函数: app_build_audio_ts
	作用:  AAC转换到ts
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
static void app_build_ts_audio(unsigned char *buff, int len, unsigned int timestamp, APP_MEDIA_INFO *info)
{
	if(g_ts_build_handle == NULL) {
		PRINTF("Error.\n");
		return ;
	}

	ts_build_audio_data(g_ts_build_handle, buff, len, timestamp, (void *)info);
	return;
}






/*初始化stream output需要的资源*/
void app_stream_output_init(unsigned int localip)
{

	/*init some middle function*/
//	mid_timer_init();
	stream_client_init();
	tc_init();




	rtp_build_init();
	rtsp_porting_sdp_init();

	app_build_ts_init();
	app_multicast_server_init();

	rtsp_set_local_ip(localip);
	rtsp_module_init();
	app_rtsp_server_init();
	return ;
}

/*输出h264给stream output模块*/
int app_stream_video_output(unsigned char *buff, int len, APP_VIDEO_DATA_INFO *info)
{

	
	if(len / (1024) > 1024) {
		PRINTF("len= %d\n", len / (1024));
	}

	int width = info->width;
	int height = info->height;
	unsigned int timestamp = info->timestamp;
	unsigned int rtp_time = info->rtp_time;
	int IsIframe = info->IsIframe;
	int mtu = 0;
	int rtsp_mtu = 0;

	APP_MEDIA_INFO media_info ;
	memset(&media_info, 0, sizeof(APP_MEDIA_INFO));
	media_info.width = width;
	media_info.height = height;
	//if(IsIframe == 1)
	//	PRINTF("IsIframe = %u.\n", IsIframe);

	//	static int g_test = 0;

	/*if it is iframe,i will update the video sdp info*/
	if(IsIframe == 1) {
		rtsp_porting_video_filter_sdp_info(buff, width, height, 0);
		//PRINTF("i Frame\n");
	}

	/*if ts is need,i must build ts.*/
	if(mult_ts_get_active() == 1 || mult_rtpts_get_active() == 1) {
		app_build_ts_video(buff, len, timestamp, &media_info);
	}

	/*if rtp is need,i must build rtp ,maybe rtsp also need rtp*/
	if(mult_rtp_get_active() == 1 || stream_rtsp_get_active() == 1) {
		mtu = mult_get_direct_rtp_mtu();
		rtsp_mtu = stream_get_rtsp_mtu();

		if(rtsp_mtu < mtu) {
			mtu = rtsp_mtu;
		}

		//printf("len=%06d\n",len);
		//printf("buff=%p,buflen=%06d,g_test = %05d\n",buff,len,g_test++);
		//PRINTF("mtu=%d\n", mtu);
		//rtp_build_video_data(len, buff, IsIframe, mtu, timestamp, &media_info);
		rtp_build_video_data(len, buff, IsIframe, mtu, rtp_time, &media_info);
	}
	else
	{
		rtp_build_reset_time();
	}

	return 0;
}

extern  unsigned int audio_atds_hander_parse(unsigned  char *pdata);

/*输出aac给stream output模块*/
int app_stream_audio_output(unsigned char *buff, int len, APP_AUDIO_DATA_INFO *info)
{
	//PRINTF("buf=%p,len=%d\n",buff,len);
	int recreate = info->recreate;
	int samplerate = info->samplerate;
	int channel = info->channel;
	unsigned int timestamp = info->timestamp;
	unsigned int rtp_time = info->rtp_time;
	int mtu = 0;
	int rtsp_mtu = 0;

	APP_MEDIA_INFO media_info ;
	memset(&media_info, 0, sizeof(APP_MEDIA_INFO));
	media_info.sample = info->samplerate;

	if(len > 1300) {
		PRINTF("Warnning,the len = %d\n", len);
	}

	//printf("audio capture time = %u.\n", timestamp);
	
//	audio_atds_hander_parse(buff);

	/*if samplerate is change ,i must update audio sdp info.*/
	if(recreate == 1) {
		PRINTF("the audio changed ,update audio sdp.\n");
		rtsp_porting_audio_filter_sdp_info(buff, channel, samplerate, 0);
	}

	/*if ts is need,i must build ts.*/
	if(mult_ts_get_active() == 1 || mult_rtpts_get_active() == 1) {
		app_build_ts_audio(buff, len, timestamp, &media_info);
	}

	/*if rtp is need,i must build rtp ,maybe rtsp also need rtp*/
	if(mult_rtp_get_active() == 1 || stream_rtsp_get_active() == 1) {
		mtu = mult_get_direct_rtp_mtu();
		rtsp_mtu = stream_get_rtsp_mtu();

		if(rtsp_mtu < mtu) {
			mtu = rtsp_mtu;
		}

		//	printf("audio len=%d\n",len);
		//	printf("audio header =%02x,%02x,%02x,%02x,%02x,%02x,%02x,",buff[0],buff[1],
		//		buff[2],buff[3],buff[4],buff[5],buff[6]);
		//			printf("audio header =%02x,%02x,%02x,%02x,%02x,%02x,%02x,\n",buff[7],buff[8],
		//		buff[9],buff[10],buff[11],buff[12],buff[13]);
		//rtp_build_audio_data(len, buff, samplerate, mtu, timestamp, &media_info);
		rtp_build_audio_data(len, buff, samplerate, mtu, rtp_time, &media_info);
		
	}

	return 0;
}

#if 0
/*==============================================================================
	函数: app_build_reset_time
	作用: 用于同步时间之后的处理
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
void app_build_reset_time()
{
	if(g_ts_build_handle == NULL) {
		PRINTF("Error.\n");
		return ;
	}

	ts_build_reset_time(g_ts_build_handle);
	rtp_build_reset_time();
	rtpts_build_reset_time();
	return ;
}
#endif

