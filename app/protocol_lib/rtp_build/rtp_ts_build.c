/*****************************************************************************
*
* rtp_ts_build.c
*
*                                              add by zm
* 																										2012-04-23
*
*
*============================================================================
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
#include "rtp_ts_build.h"
#include "rtp_struct.h"

#include "../log/log_common.h"

static int rtpts_build_one(int len , char *buff, int frame_flag, unsigned int nowtime,void *info);


int rtpts_build_data(unsigned int nLen, unsigned char *pData, int rtp_mtu, int frame, unsigned int nowtime,void *info)
{
	int one_rtp_len = 0;
	//if(mtu >RTP_PAYLOAD_LEN)
	//	mtu = RTP_PAYLOAD_LEN;
	int mtu = rtp_mtu - 14;
	one_rtp_len = 188 * (mtu / 188);

	unsigned 	int sendlen = 0;

	while(sendlen < nLen) {
		if((nLen - sendlen) < one_rtp_len) {
			rtpts_build_one((nLen - sendlen), pData + sendlen, frame, nowtime,info);
			sendlen += (nLen - sendlen);
		} else {
			rtpts_build_one(one_rtp_len, pData + sendlen, frame, nowtime,info);
			sendlen += one_rtp_len;
		}

		frame --;
	}

	if(sendlen != nLen) {
		PRINTF("Error,sendlen =%d,nlen=%d\n", sendlen, nLen);
	}

	return 0;
}


/*默认只支持1NALU打包*/
static unsigned short  g_ts_seq_num = 0;
static unsigned int g_ts_current_time = 0;
static int rtpts_build_one(int len , char *buff, int frame_flag, unsigned int nowtime,void  *info)
{
	int cnt = 0;

	unsigned long  current_time = 0;
	unsigned long  rtptime = 0;
	char	          sendbuf[1500] = {0};
	RTP_FIXED_HEADER			*rtp_hdr;

	rtp_hdr 							= (RTP_FIXED_HEADER *)&sendbuf[0];
	rtp_hdr->payload	                = 33;
	rtp_hdr->version	                = 2;

	rtp_hdr->marker                     = 0;
	rtp_hdr->ssrc		                = htonl(12);


	if(g_ts_current_time == 0 || nowtime < g_ts_current_time) {
		g_ts_current_time = nowtime;
	}

	rtptime  = (nowtime - g_ts_current_time) * 90;
	if(rtptime >= 0xFFFFFFFF)
	{
		PRINTF("Warnning,the rtptime is too big =%lx\n",rtptime);
		g_ts_current_time = 0;
	}
	//g_ts_current_time = current_time;
	//	PRINTF("rtptime = %u \n",rtptime);
	rtp_hdr->timestamp	= htonl(rtptime);
	rtp_hdr->seq_no = htons((g_ts_seq_num)++);

	if(buff[0] != 0x47) {
		PRINTF("len = %d,buff = 0x%x,0x%x,0x%x,0x%x\n", len, buff[0], buff[1], buff[2], buff[3]);
	}

	memcpy(&sendbuf[12], buff, len);
	len += 12;
	//	SendRtspVideoData(sendbuf,len,1,0);
	rtpts_porting_senddata(sendbuf, len, 0,info);

	return 0;
}

void rtpts_build_reset_time()
{
	g_ts_current_time = 0;
}

