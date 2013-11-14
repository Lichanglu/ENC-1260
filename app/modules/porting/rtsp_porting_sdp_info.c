/*******************************************************************
*  rtsp porting  for get sdp info
*  主要用于获取编码数据的PPS,SPS信息，做base 64编码，用于RTSP交互使用
*                               add by zm
* 												2011-5-31
********************************************************************/

/******************************************************************************************
*  获取SPS.PPS用来填充SDP信息
*
*
*
*******************************************************************************************/
#if 1
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

#include "mid_mutex.h"

#include "rtsp_server.h"

#include "rtsp_porting_sdp_info.h"
#include "log_common.h"
typedef struct RTSP_VIDEO_SDP_INFO_T {

	unsigned int 	roomid;     //区分第几路编码器 0/1/2/3
	unsigned int width;
	unsigned int heigh;

	unsigned int 	sdp_flag ;             //是否有sdp info
	unsigned char 	sps_info[MAX_SPS_INFO_LEN];
	unsigned int 	sps_info_len;
	unsigned char 	pps_info[MAX_SPS_INFO_LEN];
	unsigned int 	pps_info_len;
} RTSP_SDP_VINFO;

typedef struct audio_aac_adts_fixed_header_t {
	int syncword;
	unsigned char id;
	unsigned char layer;
	unsigned char profile;
	unsigned char sampling_frequency_index;
	unsigned char channel_configuration;
} audio_aac_adts_fixed_header_s;

typedef struct adts_fixed_header{
	unsigned  syncword:12;
	unsigned  id:1;
	unsigned  layer:2;	
	unsigned  protecction_absent:1;
	unsigned  profile:2;
	unsigned  sampling_frequency_index:4;
	unsigned  private_bit:1;
	unsigned  channel_configuration:3;
	unsigned  original:1;
	unsigned  home:1;
}Adts_Fixed_Header;

typedef struct adts_variable_header{
	unsigned  copyright_identification_bit:1;
	unsigned  copyright_identification_start:1;
	unsigned  frame_length:13;
	unsigned  adts_buffer_fullness:11;
	unsigned  number_of_raw_data_blocks_in_fames:2;
}Adts_Variable_Header;


//static audio_aac_adts_fixed_header_s g_audio_adts_header;


typedef struct RTSP_AUDIO_SDP_INFO_T {

	unsigned int 	roomid;     //区分第几路编码器 0/1/2/3
	unsigned int  channel;  //默认为2
	unsigned int  samplerate; //音频采样率

	unsigned int  audio_config ; // 是否有audio config
} RTSP_SDP_AINFO;

#define MAX_VIDEO_SDP_NUM 2
static mid_mutex_t 	g_rtsp_video_sdp_mutex = NULL;
//static mid_mutex_t 	g_rtsp_audio_sdp_mutex = NULL;
static RTSP_SDP_VINFO g_video_sinfo[MAX_VIDEO_SDP_NUM] ;
static RTSP_SDP_AINFO g_audio_sinfo[MAX_VIDEO_SDP_NUM];


static int rtsp_porting_get_sps_info(char *string, int *len, int id);
static int rtsp_porting_set_sps_info(unsigned char *string, int len, int id);
static int rtsp_porting_set_pps_info(unsigned char *string, int len, int id);
static int rtsp_porting_get_pps_info(char *string, int *len, int id);
extern int FindH264StartNAL(unsigned char *pp);
extern int app_b64_encode(void const *src, size_t srcSize, char *dest, int  destLen);


static int rtsp_porting_set_sdp_flag(int flag, int id);
static int rtsp_porting_set_width_height(int width, int height, int id);
static int rtsp_porting_get_width_height(int *width, int *height, int id);
static int rtsp_porting_get_nal_unit_type(unsigned char nal_code);
static unsigned int rtsp_stream_audio_filter_sdp_info(unsigned char *pdata);
static int rtsp_porting_get_sps_info(char *string, int *len, int id)
{
	int ret = 0;
	RTSP_SDP_VINFO *info = NULL;
	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	info = &(g_video_sinfo[id]);

	if(info->sdp_flag == 0) {
		ret = -1;
	} else {
		memcpy(string, info->sps_info, info->sps_info_len);
		*len = info->sps_info_len;
	}

	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return ret;
}

static int rtsp_porting_set_sps_info(unsigned char *string, int len, int id)
{
	if(len > MAX_SPS_INFO_LEN) {
		PRINTF("Error,\n");
		return -1;
	}

	//int ret = 0;
	RTSP_SDP_VINFO *info = NULL;

	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	info = &(g_video_sinfo[id]);
	memcpy(info->sps_info, string, len);
	info->sps_info_len = len;
	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return 0;
}

static int rtsp_porting_set_pps_info(unsigned char *string, int len, int id)
{
	if(len > MAX_SPS_INFO_LEN) {
		PRINTF("Error,\n");
		return -1;
	}

	//int ret = 0;
	RTSP_SDP_VINFO *info = NULL;

	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	info = &(g_video_sinfo[id]);
	memcpy(info->pps_info, string, len);
	info->pps_info_len = len;
	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return 0;
}


static int rtsp_porting_get_pps_info(char *string, int *len, int id)
{
	int ret = 0;
	RTSP_SDP_VINFO *info = NULL;
	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	info = &(g_video_sinfo[id]);

	if(info->sdp_flag == 0) {
		ret = -1;
	} else {
		memcpy(string, info->pps_info, info->pps_info_len);
		*len = info->pps_info_len;
	}

	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return ret;
}

static int rtsp_porting_set_sdp_flag(int flag, int id)
{


	//int ret = 0;
	RTSP_SDP_VINFO *info = NULL;

	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	info = &(g_video_sinfo[id]);
	info->sdp_flag = flag;
	//PRINTF("info->sdp_flag  = %d\n", flag);
	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return 0;
}

static int rtsp_porting_set_width_height(int width, int height, int id)
{


	//	int ret = 0;
	RTSP_SDP_VINFO *info = NULL;

	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	info = &(g_video_sinfo[id]);
	info->width = width;
	info->heigh = height;
	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return 0;
}

static int rtsp_porting_get_width_height(int *width, int *height, int id)
{
	int ret = 0;
	RTSP_SDP_VINFO *info = NULL;
	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	info = &(g_video_sinfo[id]);
	*width  = info->width;
	*height = info->heigh;
	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return ret;
}

static int rtsp_porting_get_nal_unit_type(unsigned char nal_code)
{
	unsigned char nal_unit_type = 0;
	unsigned char nal_ref_idc = 0;
	nal_unit_type = nal_code & 0x1f;
	nal_ref_idc = nal_code & 0x60;

	if(nal_unit_type == 7 || nal_unit_type == 8) {
		if(nal_ref_idc == 0) {
			return -1;
		}

		return nal_unit_type;
	}

	return nal_unit_type;

}

//lichl
unsigned int audio_atds_hander_parse(unsigned  char *pdata)
{
	Adts_Variable_Header hdr;
	hdr.number_of_raw_data_blocks_in_fames=(pdata[6]&0x03);
	if(hdr.number_of_raw_data_blocks_in_fames != 0)
		PRINTF("-----------number_of_raw_data_blocks_in_fames=%d \n",hdr.number_of_raw_data_blocks_in_fames+1);

	return 0;
}


/*parse audio aac adts header length*/
static unsigned int rtsp_stream_audio_filter_sdp_info(unsigned char *pdata)
{
	if(pdata == NULL) {
		return -1;
	}

	PRINTF("the audio adts header [0]=0x%x,[1]=0x%x,[2]=0x%x,[3]=0x%x\n", pdata[0], pdata[1], pdata[2], pdata[3]);
	int syncword = 0;
	unsigned char id;
	unsigned char layer;
	unsigned char profile;
	unsigned char sampling_frequency_index;
	unsigned char channel_configuration;
	audio_aac_adts_fixed_header_s audio_adts_header;
	unsigned int config = 0;

	syncword = (pdata[0] << 4) | ((pdata[1] & 0xf0) >> 4) ;

	if(syncword != 0x0fff) {
		PRINTF("the syncword =0x%x\n", syncword);
		return -1;
	}

	id = (pdata[1] & 0x08) >> 3;
	layer = (pdata[1] & 0x06) >> 1;
	profile = (pdata[2] & 0xc0) >> 6;
	sampling_frequency_index = (pdata[2] & 0x3c) >> 2;
	channel_configuration = ((pdata[2] & 0x01) >> 2) | ((pdata[3] & 0xc0) >> 6);
	PRINTF("layer=%d,the profile=%d,the sampling=0x%x,channel_configura=%d\n",
	       layer, profile, sampling_frequency_index, channel_configuration);

	memset(&audio_adts_header, 0, sizeof(audio_adts_header));
	audio_adts_header.id = id;
	audio_adts_header.layer = layer;
	audio_adts_header.profile = profile;
	audio_adts_header.sampling_frequency_index = sampling_frequency_index;
	audio_adts_header.channel_configuration = channel_configuration;

	config = 0x1000 | (((audio_adts_header.sampling_frequency_index & 0x0f) >> 1) << 8)
	         | ((audio_adts_header.sampling_frequency_index & 0x01) << 7)
	         | ((audio_adts_header.channel_configuration) << 3) ;



	PRINTF("g_audio_adts_header.sampling_frequency_index =%d,channel=%d,config=%04x\n", audio_adts_header.sampling_frequency_index
	       , audio_adts_header.channel_configuration, config);


	return config;
}

/*parse IDR header length*/
int rtsp_porting_video_filter_sdp_info(unsigned char *pdata, int width, int height, int id)
{
	if(id >= MAX_VIDEO_SDP_NUM) {
		PRINTF("Error,id=%d\n", id);
		return -1;
	}

	int len = 0x17;
	int ret = 0, I_frame_header_length = 0;
	unsigned char *find = pdata;
	int temp_len = 0;
	unsigned char *nalu_ptr = NULL;
	int nalu_len = 0;
	char nal_unit_type = 0;

	int owidth = 0;
	int oheight = 0;
	rtsp_porting_get_width_height(&owidth, &oheight,   id);

	if(owidth != width || oheight != height) {
		PRINTF("the width and height is change,old = %d * %d,new=%d*%d\n", owidth, oheight, width, height);
		rtsp_change_sdp_info(id);
		rtsp_porting_set_width_height(width, height, id);
	}

	//else
	//	{
	//		return 0;
	//	}



	while(find) {
		ret = FindH264StartNAL(find);

		if(ret) {
			I_frame_header_length++ ;

			if(I_frame_header_length == 2) {
				temp_len = find - pdata;
			}

			if(I_frame_header_length >= 3) {
				break;
			} else {
				find += 3;    //find next NAL header
			}
		}

		find++;
	}

	len = (int)(find - pdata);

	//PRINTF("length = %d\n", len);

	if(len > H264_HEADER_LEN) {
		len = 0x18;
	}

	if(I_frame_header_length < 3) {
		return len;
	}

	nalu_ptr = pdata;
	nalu_len = temp_len;
	nal_unit_type = rtsp_porting_get_nal_unit_type(nalu_ptr[4]);
	//PRINTF("len = %d,temp_len = %d,nalu_ptr[4] =0x%02x,nal_unit_type = %d\n", len, temp_len, nalu_ptr[4], nal_unit_type);

	if(nal_unit_type == 7) {
		rtsp_porting_set_sps_info(&(nalu_ptr[4]), nalu_len - 4, id);
	} else	if(nal_unit_type == 8) {
		rtsp_porting_set_pps_info(&(nalu_ptr[4]), nalu_len - 4, id);
	}


	nalu_ptr = pdata + temp_len ;
	nalu_len = len - temp_len;
	nal_unit_type = rtsp_porting_get_nal_unit_type(nalu_ptr[4]);

	//PRINTF("len = %d,temp_len = %d,nalu_ptr[4] =0x%02x,nal_unit_type = %d\n", len, temp_len, nalu_ptr[4], nal_unit_type);

	if(nal_unit_type == 7) {
		rtsp_porting_set_sps_info(&(nalu_ptr[4]), nalu_len - 4, id);
	} else	if(nal_unit_type == 8) {
		rtsp_porting_set_pps_info(&(nalu_ptr[4]), nalu_len - 4, id);
	}

	rtsp_porting_set_sdp_flag(1, id);

	//

	return len;
}

int rtsp_porting_audio_filter_sdp_info(unsigned char *pdata, int channel, int samplerate, int id)
{
	if(id >= MAX_VIDEO_SDP_NUM) {
		PRINTF("Error,id=%d\n", id);
		return -1;
	}

	unsigned int config = 0;
	config = rtsp_stream_audio_filter_sdp_info(pdata);
	PRINTF("config = %d\n", config);
	mid_mutex_lock(g_rtsp_video_sdp_mutex);
	g_audio_sinfo[id].audio_config = config;
	g_audio_sinfo[id].channel = channel;
	g_audio_sinfo[id].samplerate = samplerate;
	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return 0;
}


/*
*	获取系统的SDP信息，主要在IDR包头里面查找SPS,PPS
*
*/
int rtsp_stream_get_video_sdp_info(char *buff, int id)
{
	if(id >= MAX_VIDEO_SDP_NUM) {
		PRINTF("Error,id =%d\n", id);
		return -1;
	}

	int inlen = 0;
	char in[128] = {0};
	int outlen = 256;
	char out[256] = {0};
	int b64_len = 0;
	int b64_len1 = 0;
	int ret = 0;

	ret = rtsp_porting_get_sps_info(in, &inlen, id);

	if(inlen == 0 || ret < 0) {
		PRINTF("Error\n");
		return -1;
	}

	PRINTF("sps info ,inlen = %d\n", inlen);

	b64_len = app_b64_encode(in, inlen, out, outlen);

	if(b64_len < inlen) {
		PRINTF("Error\n");
		return -1;
	}

	memcpy(buff, out, b64_len);

	sprintf(buff + b64_len, ",");

	memset(in, 0, sizeof(in));
	memset(out, 0, sizeof(out));
	ret = rtsp_porting_get_pps_info(in, &inlen, id);

	if(inlen == 0) {
		PRINTF("Error\n");
		return -1;
	}

	PRINTF("pps info ,inlen = %d\n", inlen);

	b64_len1 = app_b64_encode(in, inlen, out, outlen);

	if(b64_len1 < inlen) {
		PRINTF("Error\n");
		return -1;
	}

	memcpy(buff + b64_len + 1, out, b64_len1);

	PRINTF("BUFF = %s\n", buff);

	return 0;
}

unsigned int rtsp_stream_get_audio_sinfo(int id, char *buff, int *rate)
{
	mid_mutex_lock(g_rtsp_video_sdp_mutex);

	if(g_audio_sinfo[id].audio_config <= 0) {
		PRINTF("ERROR,Now i can't find audio sdp info\n");
		mid_mutex_unlock(g_rtsp_video_sdp_mutex);
		return -1;
	}

	sprintf(buff, "%04x", g_audio_sinfo[id].audio_config);
	*rate = g_audio_sinfo[id].samplerate;
	mid_mutex_unlock(g_rtsp_video_sdp_mutex);
	return 0;
}



/***/
void rtsp_porting_sdp_init()
{
	if(g_rtsp_video_sdp_mutex != NULL) {
		return ;
	}

	g_rtsp_video_sdp_mutex = mid_mutex_create();

	if(g_rtsp_video_sdp_mutex == NULL) {
		PRINTF("Error! rtsp sdp mutex init error\n");
	}

	int i = 0;

	for(i = 0; i < MAX_VIDEO_SDP_NUM; i++) {
		memset(&(g_video_sinfo[i]), 0, sizeof(RTSP_SDP_VINFO))	;
		memset(&(g_audio_sinfo[i]), 0, sizeof(RTSP_SDP_AINFO))	;
	}

	return ;
}

void rtsp_porting_sdp_destory()
{
	if(g_rtsp_video_sdp_mutex == NULL) {
		return ;
	}

	mid_mutex_destroy(&g_rtsp_video_sdp_mutex);
	g_rtsp_video_sdp_mutex = NULL;

}
#endif

