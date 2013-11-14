/******************************************************************
*
* ts_build.c
*============================================================================
*  作用：主要实现转化H264裸流数据到TS数据
*         不涉及到具体的流传输等等
*
*============================================================================
*******************************************************************/
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




#include "ts_build.h"
#include "../version.h"
#include "../log/log_common.h"

#define H264_STREAM_ID 0xE0
#define AAC_STREAM_ID 0xC0


#define VIDEO_BUFFLEN_MAX 1024*1024*1
typedef enum bool {
    FALSE_ = 0, TRUE_
} BOOL;

typedef struct stTsBuf {
	unsigned char *pucAddr;
	unsigned long ulNum;
} ST_TS_BUF;

typedef struct stSystemTime {
	unsigned long ulPCRTime;
	unsigned long ulPSTTime;
	unsigned long ulDSTTime;
	double dSoftTime;
} ST_SYS_TIME;


typedef struct TS_BUILD_HANDLE_T {
	//video
	unsigned int 	video_pid;
	unsigned char v_usPATContiCount;
	unsigned char	v_usPMTContiCount;
	unsigned char	v_ucContiCnt;
	unsigned int v_lasttime ;


	ST_TS_BUF  	v_pWtBuf;
	ST_TS_BUF  	v_pPESTmpBuf;
	ST_SYS_TIME 	v_dSysTime;

	//audio
	unsigned int 	audio_pid;
	unsigned char                    a_ucContiCnt;
	unsigned int               a_lasttime;

	ST_TS_BUF                        a_pWtBuf;
	ST_TS_BUF                        a_pPESTmpBuf;
	ST_SYS_TIME                      a_dSysTime;//ms


} TS_BUILD_HANDLE;


/********************function de*******************************************/
static void V_EStoTS(TS_BUILD_HANDLE *handle, unsigned char *pBuf, int uiBufLen);
static void V_SetPAT(TS_BUILD_HANDLE *handle, unsigned char *pTSPacket, unsigned short usStreamId, unsigned char ucVerNum);
static void V_SetPMT(TS_BUILD_HANDLE *handle, unsigned char *pTSPacket, unsigned int  uiPCRPID);
static unsigned int V_SetPESHead(TS_BUILD_HANDLE *handle, unsigned char *pucOut , unsigned char *pucIn, unsigned int usPESLen, BOOL bFrameMoreThanMAXPES);
static unsigned char V_ContinousCount(TS_BUILD_HANDLE *handle);
static unsigned int V_PacketTS(TS_BUILD_HANDLE *handle, unsigned char *pInBuf, int usInbufLen, unsigned char *pOutBuf, unsigned int  uiPID);


static void A_EStoTS(TS_BUILD_HANDLE *handle, unsigned char *pBuf, int uiBufLen);
static unsigned int A_SetPESHead(double softtime, unsigned char *pucOut , unsigned char *pucIn, unsigned short usPESLen, BOOL bFrameMoreThanMAXPES);
static unsigned char A_ContinousCount(TS_BUILD_HANDLE *handle);
static unsigned int A_PacketTS(TS_BUILD_HANDLE *handle, unsigned char *pInBuf, unsigned int usInbufLen, unsigned char *pOutBuf, unsigned int uiPID);
static int SendTsDataToVlc(unsigned char *buff, unsigned int len, int type, unsigned int nowtime,void *info);
static unsigned int ts_get_video_pid(void);
static unsigned int ts_get_audio_pid(void);


/******************************public varible*********************************/


/******************************public varible*********************************/
static const unsigned int  dvbpsi_crc32_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
	0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
	0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
	0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
	0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
	0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
	0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
	0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
	0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
	0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
	0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
	0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
	0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
	0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
	0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
	0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
	0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
	0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
	0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
	0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
	0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
	0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};




static int ts_build_handle_delete(TS_BUILD_HANDLE **dhandle)
{
	if(*dhandle == NULL) {
		return 0;
	}

	TS_BUILD_HANDLE *handle = *dhandle;

	if(handle->a_pPESTmpBuf.pucAddr != NULL) {
		free(handle->a_pPESTmpBuf.pucAddr);
	}

	handle->a_pPESTmpBuf.pucAddr = NULL;

	if(handle->a_pWtBuf.pucAddr != NULL) {
		free(handle->a_pWtBuf.pucAddr);
	}

	handle->a_pWtBuf.pucAddr = NULL;

	if(handle->v_pPESTmpBuf.pucAddr != NULL) {
		free(handle->v_pPESTmpBuf.pucAddr);
	}

	handle->v_pPESTmpBuf.pucAddr = NULL;

	if(handle->v_pWtBuf.pucAddr  != NULL) {
		free(handle->v_pWtBuf.pucAddr);
	}

	handle->v_pWtBuf.pucAddr  = NULL;

	free(*dhandle);
	dhandle = NULL;
	return 0;

}



static void V_SetPAT(TS_BUILD_HANDLE *handle, unsigned char *pTSPacket, unsigned short usStreamId, unsigned char ucVerNum)
{
	unsigned int i_crc;
	unsigned char i;
	unsigned char *pIn = pTSPacket;
	pIn[0] = 0x47; //sync
	pIn[1] = 0x40; //payload start indicator
	pIn[2] = 0x00; //PID
	pIn[3] = (0x30 | handle->v_usPATContiCount++); //adaptation & counter
	handle->v_usPATContiCount = handle->v_usPATContiCount > 0xf ? 0 : handle->v_usPATContiCount;
	pIn[4] = 0xa6; //length of fill in data
	pIn[5] = 0x00; //various flag

	for(i = 1; i < 0xa6; i++) {
		pIn[5 + i] = 0xff;
	}

	pIn[5 + 0xa6] = 0; //point field
	pIn[5 + 0xa6 + 1] = 0; //table id
	pIn[5 + 0xa6 + 2] = 0xb0;
	pIn[5 + 0xa6 + 3] = 0x0d; //section length
	pIn[5 + 0xa6 + 4] = (usStreamId >> 8);
	pIn[5 + 0xa6 + 5] = usStreamId & 0xff; //streamid
	pIn[5 + 0xa6 + 6] = ucVerNum; //verson num.here always be 0xed,maybe sometime should be changed 0xdf;

	/*because only one channel,and pat in one section*/
	pIn[5 + 0xa6 + 7] = 0x00;
	pIn[5 + 0xa6 + 8] = 0x00;
	pIn[5 + 0xa6 + 9] = 0x00;
	pIn[5 + 0xa6 + 0xa] = 0x01;
	pIn[5 + 0xa6 + 0xb] = 0xe0;
	pIn[5 + 0xa6 + 0xc] = 0x42; //pmt

	/* CRC_32 */
	i_crc = 0xffffffff;

	for(i = 0; i < 12; i++) {
		i_crc = (i_crc << 8)
		        ^ dvbpsi_crc32_table[(i_crc >> 24) ^ pIn[6 + 0xa6 + i]];
	}

	pIn[5 + 0xa6 + 0xd] = (i_crc >> 24) & 0xff;
	pIn[5 + 0xa6 + 0xe] = (i_crc >> 16) & 0xff;
	pIn[5 + 0xa6 + 0xf] = (i_crc >> 8) & 0xff;
	pIn[5 + 0xa6 + 0x10] = i_crc & 0xff;

	return;
}



/*==============================================================================
	函数: V_SetPMT
	作用:  生成pmt
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
static void V_SetPMT(TS_BUILD_HANDLE *handle, unsigned char *pTSPacket, unsigned int uiPCRPID) //pcrpid& reserv=e044
{

	int have_audio  = 1;
	have_audio = 1;
	unsigned char i = 0;
	unsigned int i_crc = 0;
	unsigned char *pIn = pTSPacket;
	int plen = 0;
	pIn[0] = 0x47; //sync
	pIn[1] = 0x40; //payload start indicator
	pIn[2] = 0x42; //pmd's PID,indicated by pat
	pIn[3] = (0x10 | handle->v_usPMTContiCount); //adaptation & counter
	handle->v_usPMTContiCount++;
	handle->v_usPMTContiCount = handle->v_usPMTContiCount > 0xf ? 0 : handle->v_usPMTContiCount;

	pIn[4] = 0;  //point field

	plen = 4 + 1;
	pIn[plen + 0] = 2; //table id
	pIn[plen + 1] = 0xb0;

	//setion len
	if(have_audio == 1) {
		pIn[plen + 2] = 0x12 + 0xf + 5; //len
	} else {
		pIn[plen + 2] = 0x12 + 0xf + 5 - 5; //len
	}

	pIn[plen + 3] = 0; //program number
	pIn[plen + 4] = 0x01; //program number

	//pIn[5+0x8d+6]=0xef;//0xd7; //reserv & verson num
	pIn[plen + 5] = 0xc5; //section version

	pIn[plen + 6] = 0; //section number
	pIn[plen + 7] = 0; //last section number
	pIn[plen + 8] = (0xe << 4) | ((uiPCRPID & 0x1f00) >> 8); //0x7

	pIn[plen + 0x9] = uiPCRPID & 0xff; //pcrpid

	pIn[plen + 0xa] = 0xf0;
	//==================
	pIn[plen + 0xb] = 0xf; //reserv & program_info_len

	plen = plen + 0xb + 1;
	//descriptor
	pIn[plen + 0x0] = 0x1d;
	pIn[plen + 0x1] = 0x0d;
	pIn[plen + 0x2] = 0x11;
	pIn[plen + 0x3] = 0x01;
	pIn[plen + 0x4] = 0x02;
	pIn[plen + 0x5] = 0x80;
	pIn[plen + 0x6] = 0x80;
	pIn[plen + 0x7] = 0x07;
	pIn[plen + 0x8] = 0;
	pIn[plen + 0x9] = 0x4f;
	pIn[plen + 0xa] = 0xff;
	pIn[plen + 0xb] = 0xff;
	pIn[plen + 0xc] = 0xfe;
	pIn[plen + 0xd] = 0xfe;
	pIn[plen + 0xe] = 0xff;

	//==========================
	//audio
	plen = plen + 0xe + 1;

	if(have_audio == 1) {
		pIn[plen + 0x0] = 0x0f;
		pIn[plen + 0x1] = 0xe0;
		pIn[plen + 0x2] = ts_get_audio_pid(); //audio pid
		pIn[plen + 0x3] = 0xf0;
		pIn[plen + 0x4] = 0;
		plen = plen + 0x4 + 1;
	}

	//=============video=============

	pIn[plen + 0] = 0x1b;
	pIn[plen + 1] = (0xe << 4) | ((uiPCRPID & 0x1f00) >> 8); //0x7
	pIn[plen + 2] = uiPCRPID & 0xff; //pcrpid
	pIn[plen + 3] = 0xf0;
	pIn[plen + 4] = 0; //reserv & es_info_len

	plen = plen + 4 + 1;
	/* CRC_32 */
	i_crc = 0xffffffff;

	for(i = 0; i < plen - 5 ; i++) {
		i_crc = (i_crc << 8)
		        ^ dvbpsi_crc32_table[(i_crc >> 24) ^ pTSPacket[5 + i ]];
	}

	pIn[plen + 0] = (i_crc >> 24) & 0xff;
	pIn[plen + 1] = (i_crc >> 16) & 0xff;
	pIn[plen + 2] = (i_crc >> 8) & 0xff;
	pIn[plen + 3] = i_crc & 0xff;

	plen = plen + 3 + 1;

	//printf("plen  =%d\n",plen);
	for(i = 0; i < (188 - plen); i++) {
		pIn[plen + i] = 0xff;
	}


	return;
}



/*******************************************************************
*Function name:V_EStoTS()
*Inparam  : pBuf(UINT8*),the point of input data
		    uiBufLen(UINT64),input data len
*Outparam:null
*Function:transform es data to ts data.
********************************************************************/
#if 0
static void V_EStoTS(TS_BUILD_HANDLE *handle, unsigned char *pBuf, int uiBufLen)
{
	unsigned int j = 0;
	unsigned short usPESNumOfOneFrame = 0, usPESDatLeft = 0;
	BOOL bFrameMoreThanMAXPES = FALSE_;
	int uiPESHeadAndDatLen = 0;

	/*ES->PES*/
	usPESNumOfOneFrame = (uiBufLen - 1) / (65000 - 0xd); //maybe one frame should divided into some PESs.

	if (uiBufLen >= 0xFDE8)
	{
		printf("warnning,usHeadAndDatLen=%x\n",uiBufLen);
     		//usHeadAndDatLen = 0;
	}
	
	/*frame in one PES*/
	if(usPESNumOfOneFrame < 1) {
		uiPESHeadAndDatLen =
		    V_SetPESHead(handle, handle->v_pPESTmpBuf.pucAddr, pBuf, uiBufLen, bFrameMoreThanMAXPES);
		V_SetPAT(handle, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), 0x3694, 0xdf); //0x69d3,0xed);
		handle->v_pWtBuf.ulNum += 188;
		V_SetPMT(handle, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), ts_get_video_pid());
		handle->v_pWtBuf.ulNum += 188;
		handle->v_pWtBuf.ulNum += V_PacketTS(handle, handle->v_pPESTmpBuf.pucAddr, uiPESHeadAndDatLen, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), ts_get_video_pid());
	}
	/*frame in some PESs*/
	else {
		for(j = 0; j < usPESNumOfOneFrame; j++) {
			uiPESHeadAndDatLen =
			    V_SetPESHead(handle, handle->v_pPESTmpBuf.pucAddr, &pBuf[(65000 - 0xd) * j], (65000 - 0xd), bFrameMoreThanMAXPES);
			bFrameMoreThanMAXPES = TRUE_;
			V_SetPAT(handle, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), 0x3694, 0xdf); //0x69d3,0xed);
			handle->v_pWtBuf.ulNum += 188;
			V_SetPMT(handle, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), ts_get_video_pid());
			handle->v_pWtBuf.ulNum += 188;
			handle->v_pWtBuf.ulNum += V_PacketTS(handle, handle->v_pPESTmpBuf.pucAddr, uiPESHeadAndDatLen, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), ts_get_video_pid());
		}

		usPESDatLeft = uiBufLen - (65000 - 0xd) * j;
		uiPESHeadAndDatLen =
		    V_SetPESHead(handle, handle->v_pPESTmpBuf.pucAddr, &pBuf[(65000 - 0xd) * j], usPESDatLeft, bFrameMoreThanMAXPES);
		handle->v_pWtBuf.ulNum += V_PacketTS(handle, handle->v_pPESTmpBuf.pucAddr, uiPESHeadAndDatLen, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), ts_get_video_pid());
		bFrameMoreThanMAXPES = FALSE_;
	}

	return;
}
#else

static void V_EStoTS(TS_BUILD_HANDLE *handle, unsigned char *pBuf, int uiBufLen)
{
	unsigned int j = 0;
	unsigned short usPESNumOfOneFrame = 0, usPESDatLeft = 0;
	BOOL bFrameMoreThanMAXPES = FALSE_;
	int uiPESHeadAndDatLen = 0;
	if (uiBufLen >= 0xFDE8)
	{
		PRINTF("warnning,usHeadAndDatLen2=%x\n",uiBufLen);
     		//usHeadAndDatLen = 0;
	}
	//printf("uiBufLen == 0x%x\n",uiBufLen);
	/*ES->PES*/
	//usPESNumOfOneFrame = (uiBufLen - 1) / (65000 - 0xd); //maybe one frame should divided into some PESs.

	/*frame in one PES*/

		uiPESHeadAndDatLen =
		    V_SetPESHead(handle, handle->v_pPESTmpBuf.pucAddr, pBuf, uiBufLen, bFrameMoreThanMAXPES);
		V_SetPAT(handle, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), 0x3694, 0xdf); //0x69d3,0xed);
		handle->v_pWtBuf.ulNum += 188;
		V_SetPMT(handle, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), ts_get_video_pid());
		handle->v_pWtBuf.ulNum += 188;
		handle->v_pWtBuf.ulNum += V_PacketTS(handle, handle->v_pPESTmpBuf.pucAddr, uiPESHeadAndDatLen, (handle->v_pWtBuf.pucAddr + handle->v_pWtBuf.ulNum), ts_get_video_pid());


	return;
}
#endif



/*******************************************************************
*Function name:V_SetPESHead()
*Inparam  : pBuf(UINT8*),the point of output data
		    pucIn(UINT8*),the point of input data
		    usPESLen(unsigned short),single PES len,max value is 65000
		    bFrameMoreThanMAXPES(BOOL),indicator whether frame in one or more PES
*Outparam:null
*return value:the len of pes data
*Function:Set PES HEAD
********************************************************************/
static unsigned int V_SetPESHead(TS_BUILD_HANDLE *handle, unsigned char *pucOut , unsigned char *pucIn, unsigned int usPESLen, BOOL bFrameMoreThanMAXPES)
{
	unsigned int usHeadAndDatLen = 0;
	unsigned int pes_len = 0;
	unsigned short usTmp;
	unsigned char *pOutDat = pucOut;
	unsigned long long uiTime;
	double dTimetmp;
	pOutDat[0] = 0;
	pOutDat[1] = 0;
	pOutDat[2] = 0x01; //sys
	pOutDat[3] = H264_STREAM_ID;

	/*different length in different condition bacause of head*/
	if(bFrameMoreThanMAXPES != FALSE_) {
		usHeadAndDatLen = usPESLen + 0x03;
	} else {
		usHeadAndDatLen = usPESLen + 0x0d;
	}


	pes_len = usHeadAndDatLen;
	if (pes_len >= 0xFDE8)
	{
		PRINTF("warnning,usHeadAndDatLen=%x=%d\n",pes_len,bFrameMoreThanMAXPES);
     		pes_len = 0;
	}
	
	pes_len = pes_len&0xffff;
	
	pOutDat[4] = (pes_len >> 8) & 0xff;
	pOutDat[5] = pes_len & 0xff;
	pOutDat[6] = 0x80;               //custom define ,maybe this should be changed

	if(bFrameMoreThanMAXPES != FALSE_) {
		pOutDat[7] = 0;
		pOutDat[8] = 0;
		memcpy(&pOutDat[9], pucIn, usPESLen);
		return (usHeadAndDatLen + 6);
	}

	pOutDat[7] = 0xc0;              //PST DST flag
	pOutDat[8] = 0x0a;              //PES head len

	if(bFrameMoreThanMAXPES == FALSE_) {
		dTimetmp = handle->v_dSysTime.dSoftTime;
	}

	/*PTS & DTS*/
	uiTime = (dTimetmp * 27000000) / 300;
	uiTime = uiTime & 0x3ffffffffLL;
	pOutDat[9] = (3 << 4) | ((uiTime >> 30) << 1) | 1;
	usTmp = (uiTime & 0x3fff8000) >> 15;
	pOutDat[10] = usTmp >> 7;
	pOutDat[11] = ((usTmp & 0xff) << 1) | 1;

	usTmp = (uiTime & 0x7fff);
	pOutDat[12] = usTmp >> 7;
	pOutDat[13] = ((usTmp & 0xff) << 1) | 1;

	pOutDat[14] = (1 << 4) | ((uiTime >> 30) << 1) | 1;
	usTmp = (uiTime & 0x3fff8000) >> 15;
	pOutDat[15] = usTmp >> 7;
	pOutDat[16] = ((usTmp & 0xff) << 1) | 1;

	usTmp = (uiTime & 0x7fff);
	pOutDat[17] = usTmp >> 7;
	pOutDat[18] = ((usTmp & 0xff) << 1) | 1;
	memcpy(&pOutDat[19], pucIn, usPESLen);
	return (usHeadAndDatLen + 6); //bacause data in the front of the pes should be added
}


/*******************************************************************
*Function name:V_PacketTS()
*Inparam  : pOutBuf(UINT8*),the point of output data
		    pInBuf(UINT8*),the point of input data
		    usInbufLen(unsigned short),input data length
		    uiPID(UINT32),PID
*Outparam:null
*return value:the whole length of TS pack.data & head
*Function:pack PES data to TS
********************************************************************/
static void  test_test(int tmp, int i)
{
	return ;
	static int old = 0;
	int x = 0;
	x = tmp & 0x0f;

	if(old != 0xf) {
		if((x - old != 1)) {
			PRINTF("tmp=%d,x=%d,i=%d,old=%d\n", tmp, x, i, old);
		}
	}

	old = x;
	return ;
}
static unsigned int V_PacketTS(TS_BUILD_HANDLE *handle, unsigned char *pInBuf, int usInbufLen, unsigned char *pOutBuf, unsigned int uiPID)
{
	int i = 0, val = 0, uiTSPakLen = 0, uiInbufLen = 0;
	unsigned long long ulTimebase = 0, uiTimeExt = 0;
	unsigned char ucAdpationFiledControl = 0, ucContinusCount = 0, ucLeftDatLen = 0;
	unsigned char *pAdd = NULL;
	unsigned char *pOut = pOutBuf;
	unsigned char *pIn = pInBuf;

	val = uiPID & 0x1fff;
	uiInbufLen = usInbufLen;

	if(uiInbufLen < 184) {
		pOut[0] = 0x47;
		pOut[1] = 0x40 | (val >> 8);
		pOut[2] = val & 0xff;
		ucContinusCount = V_ContinousCount(handle);
		pOut[3] = 0x30 | ucContinusCount; // adapation filed = 11,data include PSI & payload
		//PRINTF("%d\n",pOut[3] &0x0f);
		test_test(pOut[3] & 0x0f , __LINE__);
		/*be care that len is also one dat here*/
		pOut[4] = 184 - uiInbufLen - 1;

		if(pOut[4] > 0) {
			pOut[5] = 0;     //various flag all clear,just 0xff word
			memset(&pOut[6], 0xff, (pOut[4] - 1));
			pAdd = &pOut[6] + (pOut[4] - 1);
			memcpy(pAdd, pIn, (uiInbufLen)); //!!!!!!!!!! -2
		} else {
			memcpy(&pOut[5], pIn, (uiInbufLen));
		}

		pOut += 188;
		return 188;
	}

	pOut[0] = 0x47;               // sys head
	pOut[1] = 0x40 | (val >> 8); // 4 indicates that this TS packet is 1st word of PES/PSI
	pOut[2] = val & 0xff;       //PID
	ucContinusCount = V_ContinousCount(handle);

	pOut[3] = 0x30 | ucContinusCount;              // adapation filed = 11,data include PSI & payload
	//PRINTF("%d\n",pOut[3] &0x0f);
	test_test(pOut[3] & 0x0f , __LINE__);
	pOut[4] = 0x07;              //PSI len
	pOut[5] = 0x10;              //flag declare PSI only has PCR data
	ulTimebase = (handle->v_dSysTime.dSoftTime * 27000000) / 300;
	ulTimebase = ulTimebase & 0x3ffffffffLL;
	uiTimeExt = handle->v_dSysTime.dSoftTime * 27000000;
	uiTimeExt = uiTimeExt % 300;
	pOut[6] = (ulTimebase >> 25) & 0xff;
	pOut[7] = (ulTimebase >> 17) & 0xff;
	pOut[8] = (ulTimebase >> 9) & 0xff;
	pOut[9] = (ulTimebase >> 1) & 0xff;
	pOut[0xa] = ((ulTimebase & 0x1) << 7) | 0x7E | ((uiTimeExt >> 8) & 0x1);
	pOut[0xb] = ulTimebase & 0xff;

	handle->v_dSysTime.ulPCRTime += 2700; //time add	//3600?

	memcpy(&pOut[0xc], pIn, (188 - 12));
	pOut += 188;
	pIn += (188 - 12);
	uiInbufLen -= (188 - 12);
	//	v_ucContiCnt=1;

	for(i = 0; i <= uiInbufLen - 184; i += 184) {

		pOut[0] = 0x47;
		pOut[1] = 0 | (val >> 8);     //don't have PSI/PES 1st word
		pOut[2] = val & 0xff;                 //pid
		ucAdpationFiledControl = 1;
		ucContinusCount = V_ContinousCount(handle);
		pOut[3] = (ucAdpationFiledControl << 4) | ucContinusCount;
		//PRINTF("%d\n",pOut[3] &0x0f);
		test_test(pOut[3] & 0x0f , __LINE__);
		memcpy(&pOut[4], pIn, 184);
		pOut += 188;
		pIn += 184;
	}

	ucLeftDatLen = uiInbufLen % 184;

	if(ucLeftDatLen != 0) {
		pOut[0] = 0x47;
		pOut[1] = 0 | (val >> 8);
		pOut[2] = val & 0xff;
		ucContinusCount = V_ContinousCount(handle);
		pOut[3] = 0x30 | ucContinusCount; // adapation filed = 11,data include PSI & payload
		//PRINTF("%d\n",pOut[3] &0x0f);
		test_test(pOut[3] & 0x0f , __LINE__);
		/*be care that len is also one dat here*/
		pOut[4] = 184 - ucLeftDatLen - 1;

		if(pOut[4] > 0) {
			pOut[5] = 0;     //various flag all clear,just 0xff word
			memset(&pOut[6], 0xff, (pOut[4] - 1));
			pAdd = &pOut[6] + (pOut[4] - 1);
			memcpy(pAdd, pIn, (ucLeftDatLen)); //!!!!!!!!!! -2
		} else {
			memcpy(&pOut[5], pIn, (ucLeftDatLen));
		}

		pOut += 188;
	}

	uiTSPakLen = pOut - pOutBuf;
	return uiTSPakLen;
}

/*******************************************************************
*Function name:V_ContinousCount()
*Inparam  : null
*Outparam:v_ucContiCnt
*return value:val,the value of v_ucContiCnt
*Function:a simple counter
********************************************************************/
static unsigned char V_ContinousCount(TS_BUILD_HANDLE *handle)
{
	unsigned char val = 0;
	val = handle->v_ucContiCnt & 0xf;
	handle->v_ucContiCnt++;
	handle->v_ucContiCnt = handle->v_ucContiCnt < 0x10 ? handle->v_ucContiCnt : 0;
	return val;
}





static int SendTsDataToVlc(unsigned char *buff, unsigned int len, int type, unsigned int nowtime,void *info)
{
	//	printf("type = %d,buff= %p,len=%d\n",type,buff,len);
	if(type == 1) {
		ts_porting_senddata(type, buff, len, 0,nowtime,info);
	} else if(type == 2) {
		ts_porting_senddata(type, buff, len, 0,nowtime,info);
	}

	return 0;
}

static unsigned int  A_SetPESHead(double dsofttime, unsigned char *pucOut , unsigned char *pucIn, unsigned short usPESLen, BOOL bFrameMoreThanMAXPES)
{
	unsigned short usTmp, usHeadAndDatLen;
	unsigned char *pOutDat = pucOut;
	unsigned long long uiTime;
	double dTimetmp;
	pOutDat[0] = 0;
	pOutDat[1] = 0;
	pOutDat[2] = 0x01; //sys
	pOutDat[3] = AAC_STREAM_ID;

	if(bFrameMoreThanMAXPES != FALSE_) {
		usHeadAndDatLen = usPESLen + 0x03;
	} else {
		usHeadAndDatLen = usPESLen + 0x0d - 5;
	}

	pOutDat[4] = (usHeadAndDatLen >> 8) & 0xff;
	pOutDat[5] = usHeadAndDatLen & 0xff;
	pOutDat[6] = 0x80;               //custom define ,maybe this should be changed

	if(bFrameMoreThanMAXPES != FALSE_) {
		pOutDat[7] = 0;
		pOutDat[8] = 0;
		memcpy(&pOutDat[9], pucIn, usPESLen);
		return (usHeadAndDatLen + 6);
	}

	pOutDat[7] = 0x80;              //PST DST flag
	pOutDat[8] = 0x05;              //PES head len

	dTimetmp = dsofttime;

	uiTime = (dTimetmp * 27000000) / 300;

	//PTS&PDS
	//|0011 | PTS3 | 1 | PTS15 | 1 | PTS15 | 1 |
	//|0001 | PDS3 | 1 | PDS15 | 1 | PDS15 | 1 |
	uiTime = uiTime & 0x3ffffffffLL;
	//pOutDat[9]=(3<<4)|((uiTime>>30)<<1)|1;
	pOutDat[9] = (0x2 << 4) | ((uiTime >> 30) << 1) | 1;
	usTmp = (uiTime & 0x3fff8000) >> 15;
	pOutDat[10] = usTmp >> 7;
	pOutDat[11] = ((usTmp & 0xff) << 1) | 1;

	usTmp = (uiTime & 0x7fff);
	pOutDat[12] = usTmp >> 7;
	pOutDat[13] = ((usTmp & 0xff) << 1) | 1;

	memcpy(&pOutDat[14], pucIn, usPESLen);
	return (usHeadAndDatLen + 6); //bacause data in the front of the pes should be added
}


static unsigned int A_PacketTS(TS_BUILD_HANDLE *handle, unsigned char *pInBuf, unsigned int usInbufLen, unsigned char *pOutBuf, unsigned int uiPID)
{
	int i = 0, val = 0, uiTSPakLen = 0, uiInbufLen = 0;
	unsigned long long ulTimebase = 0, uiTimeExt = 0;
	unsigned char ucAdpationFiledControl = 0, ucContinusCount = 0, ucLeftDatLen = 0;
	unsigned char *pAdd;
	unsigned char *pOut = pOutBuf;
	unsigned char *pIn = pInBuf;
	val = uiPID & 0x1fff;

	uiInbufLen = usInbufLen;

	if(uiInbufLen < 188)
		PRINTF("uiInbufLen = %d\n",uiInbufLen);
#if 1
	if(uiInbufLen < 184) {
		pOut[0] = 0x47;
		pOut[1] = 0x40 | (val >> 8);
		pOut[2] = val & 0xff;
		ucContinusCount = V_ContinousCount(handle);
		pOut[3] = 0x30 | ucContinusCount; // adapation filed = 11,data include PSI & payload
		//PRINTF("%d\n",pOut[3] &0x0f);
		test_test(pOut[3] & 0x0f , __LINE__);
		/*be care that len is also one dat here*/
		pOut[4] = 184 - uiInbufLen - 1;

		if(pOut[4] > 0) {
			pOut[5] = 0;     //various flag all clear,just 0xff word
			memset(&pOut[6], 0xff, (pOut[4] - 1));
			pAdd = &pOut[6] + (pOut[4] - 1);
			memcpy(pAdd, pIn, (uiInbufLen)); //!!!!!!!!!! -2
		} else {
			memcpy(&pOut[5], pIn, (uiInbufLen));
		}

		pOut += 188;
		return 188;
	}

#endif	

	
	pOut[0] = 0x47;               // sys head
	pOut[1] = 0x40 | (val >> 8); // 4 indicates that this TS packet is 1st word of PES/PSI
	pOut[2] = val & 0xff;       //PID
	ucContinusCount = A_ContinousCount(handle);
	pOut[3] = 0x30 | ucContinusCount;            // adapation filed = 11,data include PSI & payload
	pOut[4] = 0x07;              //PSI len
	pOut[5] = 0x10;              //flag declare PSI only has PCR data

	ulTimebase = (handle->a_dSysTime.dSoftTime * 27000000) / 300;
	ulTimebase = ulTimebase & 0x3ffffffffLL;
	uiTimeExt = handle->a_dSysTime.dSoftTime * 27000000;
	uiTimeExt = uiTimeExt % 300;
	pOut[6] = (ulTimebase >> 25) & 0xff;
	pOut[7] = (ulTimebase >> 17) & 0xff;
	pOut[8] = (ulTimebase >> 9) & 0xff;
	pOut[9] = (ulTimebase >> 1) & 0xff;
	pOut[0xa] = ((ulTimebase & 0x1) << 7) | 0x7E | ((uiTimeExt >> 8) & 0x1);
	pOut[0xb] = ulTimebase & 0xff;

	handle->a_dSysTime.ulPCRTime += 2700; //time add	//3600?

	memcpy(&pOut[0xc], pIn, (188 - 12));
	pOut += 188;
	pIn += (188 - 12);
	uiInbufLen -= (188 - 12);
	//a_ucContiCnt=1;


	for(i = 0; i <= uiInbufLen - 184; i += 184) {
		pOut[0] = 0x47;
		pOut[1] = 0 | (val >> 8);     //don't have PSI/PES 1st word
		pOut[2] = val & 0xff;                 //pid
		ucAdpationFiledControl = 1;
		ucContinusCount = A_ContinousCount(handle);
		pOut[3] = (ucAdpationFiledControl << 4) | ucContinusCount;
		memcpy(&pOut[4], pIn, 184);
		pOut += 188;
		pIn += 184;
	}

	ucLeftDatLen = uiInbufLen % 184;

	if(ucLeftDatLen != 0) {
		pOut[0] = 0x47;
		pOut[1] = 0 | (val >> 8);
		pOut[2] = val & 0xff;
		ucContinusCount = A_ContinousCount(handle);
		pOut[3] = 0x30 | ucContinusCount; // adapation filed = 11,data include PSI & payload
		/*be care that len is also one dat here*/
	
		pOut[4] = 184 - ucLeftDatLen - 1;

		if(pOut[4] > 0) {
			pOut[5] = 0;     //various flag all clear,just 0xff word
			memset(&pOut[6], 0xff, (pOut[4] - 1));
			pAdd = &pOut[6] + (pOut[4] - 1);
			memcpy(pAdd, pIn, (ucLeftDatLen)); //!!!!!!!!!! -2
		} else {
			memcpy(&pOut[5], pIn, (ucLeftDatLen));
		}

		pOut += 188;
	}

	uiTSPakLen = pOut - pOutBuf;
	return uiTSPakLen;
}


static unsigned char A_ContinousCount(TS_BUILD_HANDLE *handle)
{
	unsigned char val = 0;
	val = handle->a_ucContiCnt & 0xf;
	handle->a_ucContiCnt++;
	handle->a_ucContiCnt = handle->a_ucContiCnt < 0x10 ? handle->a_ucContiCnt : 0;
	return val;
}

static void A_EStoTS(TS_BUILD_HANDLE *handle, unsigned char *pBuf, int uiBufLen)
{
	unsigned int j = 0;
	unsigned short usPESNumOfOneFrame = 0, usPESDatLeft = 0;
	BOOL bFrameMoreThanMAXPES = FALSE_;
	int uiPESHeadAndDatLen = 0;

	usPESNumOfOneFrame = (uiBufLen - 1) / (65000 - 0xd);

	if(usPESNumOfOneFrame < 1) {
		//build audio pes head
		uiPESHeadAndDatLen = A_SetPESHead(handle->a_dSysTime.dSoftTime, handle->a_pPESTmpBuf.pucAddr, pBuf, uiBufLen, bFrameMoreThanMAXPES);
		//build pes to ts
		handle->a_pWtBuf.ulNum += A_PacketTS(handle, handle->a_pPESTmpBuf.pucAddr,          \
		                                     uiPESHeadAndDatLen, (handle->a_pWtBuf.pucAddr + handle->a_pWtBuf.ulNum), ts_get_audio_pid());
	} else {

		for(j = 0; j < usPESNumOfOneFrame; j++) {
			uiPESHeadAndDatLen =
			    A_SetPESHead(handle->a_dSysTime.dSoftTime, handle->a_pPESTmpBuf.pucAddr, &pBuf[(65000 - 0xd) * j],
			                 (65000 - 0xd), bFrameMoreThanMAXPES);
			bFrameMoreThanMAXPES = TRUE_;

			handle->a_pWtBuf.ulNum += A_PacketTS(handle, handle->a_pPESTmpBuf.pucAddr,          \
			                                     uiPESHeadAndDatLen, (handle->a_pWtBuf.pucAddr + handle->a_pWtBuf.ulNum), ts_get_audio_pid());
		}

		usPESDatLeft = uiBufLen - (65000 - 0xd) * j;
		uiPESHeadAndDatLen = A_SetPESHead(handle->a_dSysTime.dSoftTime, handle->a_pPESTmpBuf.pucAddr, &pBuf[(65000 - 0xd) * j], usPESDatLeft, bFrameMoreThanMAXPES);
		handle->a_pWtBuf.ulNum += A_PacketTS(handle, handle->a_pPESTmpBuf.pucAddr, 		 \
		                                     uiPESHeadAndDatLen, (handle->a_pWtBuf.pucAddr + handle->a_pWtBuf.ulNum), ts_get_audio_pid());
		bFrameMoreThanMAXPES = FALSE_;
	}

	return;
}


static unsigned int ts_get_video_pid()
{
	return 0x44;
}

static unsigned int ts_get_audio_pid()
{
	return 0x54;
}


//转换aac frame  到ts流
/*==============================================================================
	函数: ts_build_audio_data
	作用:
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
void ts_build_audio_data(void *temp, unsigned char *pData, int nLen, unsigned int nowtime,void *info)
{
	TS_BUILD_HANDLE *handle = (TS_BUILD_HANDLE *)temp;
	char sendbuf[1500*4] = {0};
	double frametime;
	if(nLen > 1500*2)
	{
		PRINTF("Error,nLen =%d is too big\n",nLen);
		
		return ;
	}
	
	if(nowtime == 0 || nowtime < handle->a_lasttime) {
		ts_build_reset_time(handle);
	}

	if(handle->a_lasttime != 0) {
		frametime = (double)(nowtime - handle->a_lasttime) / 1000 ;
	}


	if(0  != handle->a_lasttime) {
		handle->a_dSysTime.dSoftTime += frametime;
	}

	//printf("audio dSoftTime=%f,%d,=%d\n", handle->a_dSysTime.dSoftTime, nowtime, handle->a_lasttime);
	A_EStoTS(handle, pData, nLen);

	handle->a_lasttime = nowtime;
	memset(sendbuf, 0, sizeof(sendbuf));
	memcpy(sendbuf, handle->a_pWtBuf.pucAddr, handle->a_pWtBuf.ulNum);
	//send
	SendTsDataToVlc(sendbuf, handle->a_pWtBuf.ulNum, 1, nowtime,info);
	handle->a_pWtBuf.ulNum = 0;
	handle->a_pPESTmpBuf.ulNum = 0;

	return ;

}


/*==============================================================================
	函数: ts_build_video_data
	作用:
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
void ts_build_video_data(void *temp, unsigned char *pData, int nLen, unsigned int nowtime,void *info)
{
	
	TS_BUILD_HANDLE *handle = (TS_BUILD_HANDLE *)temp;	
	int totallen = 0;

	float frametime;

	if(nLen > VIDEO_BUFFLEN_MAX)
	{
		PRINTF("Error,nLen =%d is too big\n",nLen);
		
		return ;
	}
		
	if(nowtime == 0 || nowtime < handle->v_lasttime) {
		ts_build_reset_time(handle);
	}

	if(handle->v_lasttime != 0) {
		frametime = (double)(nowtime - handle->v_lasttime) / 1000 ;
	}


	if(0  != handle->v_lasttime) {
		handle->v_dSysTime.dSoftTime += frametime;
	}


	//printf("video dSoftTime=%f,=%d=%d\n", handle->v_dSysTime.dSoftTime, nowtime, handle->v_lasttime);

	V_EStoTS(handle, pData, nLen);
	handle->v_lasttime = nowtime;


	SendTsDataToVlc(handle->v_pWtBuf.pucAddr, handle->v_pWtBuf.ulNum, 2, nowtime,info);

	handle->v_pWtBuf.ulNum = 0;
	handle->v_pPESTmpBuf.ulNum = 0;
	return ;
}


/*==============================================================================
	函数: ts_build_init
	作用:
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
void *ts_build_init(unsigned int vpid, unsigned int apid)
{
	int ret = 0;
	TS_BUILD_HANDLE *handle = NULL;
	handle = (TS_BUILD_HANDLE *)malloc(sizeof(TS_BUILD_HANDLE));

	if(handle == NULL) {
		PRINTF("error,the ts build init is error\n");
		ret = -1;
		return NULL;
	}

	memset(handle, 0, sizeof(TS_BUILD_HANDLE));
	handle->video_pid = vpid;
	handle->audio_pid = apid;

	//video init
	handle->v_dSysTime.ulPCRTime = 10;
	handle->v_dSysTime.ulPSTTime = 10;
	handle->v_dSysTime.ulDSTTime = 10;
	handle->v_dSysTime.dSoftTime = 10;
	handle->v_pWtBuf.pucAddr = (unsigned char *)malloc(VIDEO_BUFFLEN_MAX);

	if(handle->v_pWtBuf.pucAddr == NULL) {
		PRINTF("Error,malloc video wtbuf is error\n");
		ret = -1;
		goto EXIT;
	}

	handle->v_pWtBuf.ulNum = 0;                                               //output data len
	handle->v_pPESTmpBuf.pucAddr = (unsigned char *)malloc(VIDEO_BUFFLEN_MAX); //temp buff,just for ever pes pack

	if(handle->v_pPESTmpBuf.pucAddr == NULL) {
		PRINTF("Error,malloc video pesbuf is error\n");
		ret = -1;
		goto EXIT;
	}

	handle->v_pPESTmpBuf.ulNum = 0;


	/*audio init modify by zm  2012.04.26  */
	handle->a_dSysTime.ulPCRTime = 10; //ms
	handle->a_dSysTime.ulPSTTime = 10; //ms
	handle->a_dSysTime.ulDSTTime = 10;
	handle->a_dSysTime.dSoftTime = 10;
	handle->a_pWtBuf.pucAddr = (unsigned char *)malloc(700 * 1024);

	if(handle->a_pWtBuf.pucAddr == NULL) {
		PRINTF("Error,malloc a_pWtBuf is error\n");
		ret = -1;
		goto EXIT;
	}

	handle->a_pWtBuf.ulNum = 0;
	handle->a_pPESTmpBuf.pucAddr = (unsigned char *)malloc(700 * 1024);

	if(handle->a_pPESTmpBuf.pucAddr == NULL) {
		PRINTF("Error,malloc a_pWtBuf is error\n");
		ret = -1;
		goto EXIT;
	}

	handle->a_pPESTmpBuf.ulNum = 0;

EXIT:

	if(ret == 0) {
		return (void *)handle;
	}

	if(ret == -1) {
		if(handle == NULL) {
			return NULL;
		}

		ts_build_handle_delete(&handle);
		return NULL;
	}
}


/*==============================================================================
	函数: ts_build_uninit
	作用: 释放资源
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
void ts_build_uninit(void *temp)
{	
	TS_BUILD_HANDLE *handle = (TS_BUILD_HANDLE *)temp;
	ts_build_handle_delete(&handle);
	return ;
}

/*==============================================================================
	函数: ts_build_reset_time
	作用: 重置时间，用于同步编码器时间之后调用
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
void ts_build_reset_time(void *temp)
{
	TS_BUILD_HANDLE *handle = (TS_BUILD_HANDLE *)temp;
	handle->v_lasttime = 0;
	handle->a_lasttime = 0;

	handle->v_dSysTime.dSoftTime = 0; //ms
	handle->v_dSysTime.ulPCRTime = 0;
	handle->v_dSysTime.ulPSTTime = 0;
	handle->v_dSysTime.ulDSTTime = 0;


	handle->a_dSysTime.dSoftTime = 0; //ms
	handle->a_dSysTime.ulPCRTime = 0;
	handle->a_dSysTime.ulPSTTime = 0;
	handle->a_dSysTime.ulDSTTime = 0;


	return ;
}


/*==============================================================================
	函数: ts_get_version
	作用:  获取git的版本号
	输入:
	输出:
	作者:  modify by zm    2012.04.26
==============================================================================*/
int ts_build_get_version(char *version, int len)
{
	snprintf(version, len, "%s", _VERSION);
	return 0;
}

