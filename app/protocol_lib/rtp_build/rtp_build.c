/*****************************************************************************
*
*rtsp.c
*============================================================================
* This file is used for formating h.264/aac  to rtsp flow
* Ver      alpha_1.0
* Author  jbx
* Shenzhen reach 2010.9.6
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
#include "rtp_build.h"
#include "rtp_struct.h"

#include "../log/log_common.h"

static unsigned int            g_time_org = 0;
static NALU_t                  *g_audio_puiNalBuf = NULL;

/**************************************************************************************************
                    ��ȡϵͳ��ǰʱ��
**************************************************************************************************/

#if 0
static unsigned long getCurrentTime(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned long ultime;

	gettimeofday(&tv , &tz);
	ultime = tv.tv_sec * 1000 + ((tv.tv_usec) / 1000);
	return (ultime);
}
#endif

/**************************************************************************************************
                                        ΪNALU_t�ṹ������ڴ�ռ�
**************************************************************************************************/
static NALU_t *AllocNALU(int buffersize)
{
	NALU_t *n;

	if((n = (NALU_t *)calloc(1, sizeof(NALU_t))) == NULL) {
		exit(0);
	}

	n->max_size = buffersize;

	if((n->buf = (char *)calloc(buffersize, sizeof(char))) == NULL) {
		free(n);
		exit(0);
	}

	return n;
}



/*********************************************************************************************************
                                        ΪNALU_t�ṹ���ͷ��ڴ�ռ�
*********************************************************************************************************/
static void FreeNALU(NALU_t *n)
{
	if(n) {
		if(n->buf) {
			free(n->buf);
			n->buf = NULL;
		}

		free(n);
	}
}


/********************************************************************************************************************
RTP ���������ʼ��
1.��ʼ����
2.������Ƶ���������ڴ�ռ�
*********************************************************************************************************************/
void rtp_build_init()
{
	g_audio_puiNalBuf = AllocNALU(20000);
	return ;
}



/********************************************************************************************************************
�˳� RTP Э��
1.������
2.�ͷŷ�����ڴ�
*********************************************************************************************************************/
void rtb_build_uninit()
{

	FreeNALU(g_audio_puiNalBuf);
	return;
}

/********************************************************************************************************************
���� RTP video ����
*********************************************************************************************************************/
static int SendMultCastVideoData(char *pData, int length,void *info)
{
	rtp_porting_senddata(2, pData, length, 0,info);
	return 0;
}

/********************************************************************************************************************
���� RTP audio ����
*********************************************************************************************************************/
static int SendMultCastAudioData(char *pData, int length,void *info)
{
	rtp_porting_senddata(1, pData, length, 0,info);
	return 0;
}







static int SendRtpNalu(NALU_t *nalu , unsigned short *seq_num, unsigned long ts_current , int roomid, int end, int mtu,void *info)
{
	int 	rtp_mtu = mtu;
	char				 sendbuf[1500] = {0};
	RTP_FIXED_HEADER			*rtp_hdr;
	char                        *nalu_payload;
	FU_INDICATOR	            *fu_ind;
	FU_HEADER		            *fu_hdr;
	int                          bytes = 0;

	int total_len = 0;
	memset(sendbuf, 0, 20);
	//����RTP HEADER��
	rtp_hdr 							= (RTP_FIXED_HEADER *)&sendbuf[0];
	rtp_hdr->payload	                = H264;
	rtp_hdr->version	                = 2;
	rtp_hdr->marker                     = 0;
	rtp_hdr->ssrc		                = htonl(10);
	rtp_hdr->timestamp		            = htonl(ts_current);

	if((nalu->len - 1) <= rtp_mtu) {
		//����rtp M λ��

		rtp_hdr->marker = 1;
		rtp_hdr->seq_no 	= htons((*seq_num)++); //���кţ�ÿ����һ��RTP����1
		memcpy(&sendbuf[12], nalu->buf, nalu->len);
		bytes = nalu->len + 12 ; 					//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�
		//	SendRtspVideoData(sendbuf, bytes,nalu->nal_unit_type,roomid);
		SendMultCastVideoData(sendbuf, bytes,info);
		total_len = nalu->len;
	} else if((nalu->len - 1) > rtp_mtu) {
		//�õ���nalu��Ҫ�ö��ٳ���Ϊ1400�ֽڵ�RTP��������
		int k = 0, l = 0;
		int t = 0; //����ָʾ��ǰ���͵��ǵڼ�����ƬRTP��

		l = (nalu->len - 1) % rtp_mtu; //���һ��RTP������Ҫװ�ص��ֽ���

		if(l == 0) {
			k = (nalu->len - 1) / rtp_mtu - 1; //��Ҫk��1400�ֽڵ�RTP��
			l = rtp_mtu;
		} else {
			k = (nalu->len - 1) / rtp_mtu; //��Ҫk��1400�ֽڵ�RTP��
		}

		while(t <= k) {
			rtp_hdr->seq_no = htons((*seq_num)++); //���кţ�ÿ����һ��RTP����1

			if(!t) { //����һ����Ҫ��Ƭ��NALU�ĵ�һ����Ƭ����FU HEADER��Sλ
				//����rtp M λ��
				rtp_hdr->marker = 0;
				//����FU INDICATOR,�������HEADER����sendbuf[12]
				fu_ind = (FU_INDICATOR *)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
				fu_ind->F = nalu->forbidden_bit;
				fu_ind->NRI = nalu->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//����FU HEADER,�������HEADER����sendbuf[13]
				fu_hdr = (FU_HEADER *)&sendbuf[13];
				fu_hdr->E = 0;
				fu_hdr->R = 0;
				fu_hdr->S = 1;
				fu_hdr->TYPE = nalu->nal_unit_type;

				nalu_payload = &sendbuf[14]; //ͬ��sendbuf[14]����nalu_payload
				memcpy(nalu_payload, nalu->buf + 1, rtp_mtu); //ȥ��NALUͷ

				//	PRINTF("%x,ser_no=%d\n",nalu_payload[0],*seq_num);
				bytes = rtp_mtu + 14;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥ��ʼǰ׺��NALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�
				//	SendRtspVideoData(sendbuf, bytes,0,roomid);
				SendMultCastVideoData(sendbuf, bytes,info);
				total_len += rtp_mtu;
				t++;
			}
			//����һ����Ҫ��Ƭ��NALU�ķǵ�һ����Ƭ������FU HEADER��Sλ������÷�Ƭ�Ǹ�NALU�����һ����Ƭ����FU HEADER��Eλ
			else if(k == t) { //���͵������һ����Ƭ��ע�����һ����Ƭ�ĳ��ȿ��ܳ���1400�ֽڣ���l>1386ʱ����
				//����rtp M λ����ǰ����������һ����Ƭʱ��λ��1
				rtp_hdr->marker = 1;
				//����FU INDICATOR,�������HEADER����sendbuf[12]
				fu_ind = (FU_INDICATOR *)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
				fu_ind->F = nalu->forbidden_bit;
				fu_ind->NRI = nalu->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//����FU HEADER,�������HEADER����sendbuf[13]
				fu_hdr = (FU_HEADER *)&sendbuf[13];
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->TYPE = nalu->nal_unit_type;
				fu_hdr->E = 1;

				nalu_payload = &sendbuf[14]; //ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
				memcpy(nalu_payload, nalu->buf + t * rtp_mtu + 1, l); //��nalu���ʣ���l-1(ȥ����һ���ֽڵ�NALUͷ)�ֽ�����д��sendbuf[14]��ʼ���ַ�����
				bytes = l + 14;		//���sendbuf�ĳ���,Ϊʣ��nalu�ĳ���l-1����rtp_header��FU_INDICATOR,FU_HEADER������ͷ��14�ֽ�
				//	PRINTF("%x,ser_no=%d\n",nalu_payload[0],*seq_num);
				//		SendRtspVideoData(sendbuf, bytes,0,roomid);
				SendMultCastVideoData(sendbuf, bytes,info);
				total_len += l;
				t++;
			} else if(t < k && 0 != t) {
				//����rtp M λ��
				rtp_hdr->marker = 0;
				//����FU INDICATOR,�������HEADER����sendbuf[12]
				fu_ind = (FU_INDICATOR *)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
				fu_ind->F = nalu->forbidden_bit;
				fu_ind->NRI = nalu->nal_reference_idc >> 5;
				fu_ind->TYPE = 28;

				//����FU HEADER,�������HEADER����sendbuf[13]
				fu_hdr = (FU_HEADER *)&sendbuf[13];
				fu_hdr->R = 0;
				fu_hdr->S = 0;
				fu_hdr->E = 0;
				fu_hdr->TYPE = nalu->nal_unit_type;

				nalu_payload = &sendbuf[14]; //ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
				memcpy(nalu_payload, nalu->buf + t * rtp_mtu + 1, rtp_mtu); //ȥ����ʼǰ׺��naluʣ������д��sendbuf[14]��ʼ���ַ�����
				bytes = rtp_mtu + 14;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥԭNALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�

				//	SendRtspVideoData(sendbuf, bytes,0,roomid);
				SendMultCastVideoData(sendbuf, bytes,info);
				total_len += rtp_mtu;
				t++;
			}
		}
	}

	//if(total_len+1 != nalu->len )
	//{
	//	PRINTF("nalu send len =%d,nalu len= %d\n",total_len,nalu->len);
	//}
	return 0;
}

static unsigned short                           g_seq_num = 0;
int rtp_build_video_data(int nLen, unsigned char *pData, int nFlag, int rtp_mtu, unsigned int nowtime, void *info)
//(int nLen,  unsigned char *pData, int nFlag,unsigned int timetick ,int *seq,int roomid,int idr_flag)
{

	if(rtp_mtu < 228) {
		PRINTF("Error\n");
		return 0;
	}

	int roomid = 0;
	unsigned int timetick = 0;
	unsigned short *seq = &g_seq_num;
	//printf("nowtime =%u==%u\n",nowtime,g_time_org);
	if(g_time_org == 0 || nowtime < g_time_org) {
		g_time_org = nowtime;
	}

	timetick = nowtime - g_time_org;

	NALU_t		nalu;
	unsigned char *pstart;

	unsigned long ts_current  = 0;
	unsigned long mytime = 0;

	unsigned char *pos = pData;
	unsigned char *ptail = pData + nLen - 4;
	unsigned char *temp1, *temp2;

	unsigned char nalu_value = 0;
	temp1 = temp2 = NULL;
	int send_total_len = 0;

	//14 = rtp head len +2
	int mtu = rtp_mtu - 14;
	int cnt = 0;

	for(;;) {
		mytime = (unsigned long)timetick;
		//ts_current = mytime * 90;
	//	printf("mytiem=%x\n",mytime);
	//	mytime +=  (0xffffffff/44-1000*120);
		ts_current =mytime * 90 ;

		if(ts_current >= 0XFFFFFFFF)
		{
				PRINTF("Warnning,the ts_current is too big =%lx\n",ts_current);
				g_time_org = 0;
		}

					
	//	printf("video_time = %x\n",ts_current);
		pstart = pos;
		memset(&nalu, 0 , sizeof(nalu));

		nalu_value = 0;

		//�ж�ͷ�Ƿ�naluͷ 0001 ���� 001 ,�����˳�
		if(!((*pos == 0 && *(pos + 1) == 0 && *(pos + 2) == 0 && *(pos + 3) == 1))) {
			PRINTF("read nalu header failed!\n");
		} else {
			temp1 = pos;
			nalu_value = *(pos + 4);
		}

		if(((nalu_value & 0x1f) == 7) || ((nalu_value & 0x1f) == 8)) {
			//�ҵ���һ��naluͷ 0001 ���� 001 , ���ߵ�֡β
			do {
				pos++;
			} while((*pos != 0 || *(pos + 1) != 0 || *(pos + 2) != 0 || *(pos + 3) != 1)
			        && (pos < ptail));
		} else {
			pos = ptail;
		}

		if(pos >= ptail)

		{
			//����ǵ���֡β�� �������ʣ��������Ϊһ��nalu��Ԫ����
			nalu.buf = pstart + 4;
			nalu.len = pData - pstart + nLen  - 4 ;
			//PRINTF("nalu_len=%d,the len =%d,%d,%p,%p,%p\n",nalu.len,temp2-temp1 ,ptail-pstart,pstart,pos,ptail);
			send_total_len += (nalu.len + 4);
			nalu.forbidden_bit = nalu.buf[0] & 0x80;
			nalu.nal_reference_idc = nalu.buf[0] & 0x60;
			nalu.nal_unit_type = (nalu.buf[0]) & 0x1f;
			/*
			                        DEBUG(DL_FLOW,"send last nalu pkt! len = %d frame_len = %d flag = %d \
			                        pdata = 0x%x pos = 0x%x pstart = 0x%x\n", nalu.len, nLen ,nFlag\
			                        , pData , pos , pstart);
			*/
			SendRtpNalu(&nalu, (unsigned short *)seq, ts_current, roomid, 1, mtu,info);
			//PRINTF("seq =%d,time =%ld\n",seq,ts_current);
			break;
		} else {
			//����һ��nalu��Ԫ
			nalu.buf = pstart + 4;
			nalu.len = pos - pstart - 4;
			send_total_len += (nalu.len + 4);
			nalu.forbidden_bit = nalu.buf[0] & 0x80;
			nalu.nal_reference_idc = nalu.buf[0] & 0x60;
			nalu.nal_unit_type = (nalu.buf[0]) & 0x1f;
			/*
			                        DEBUG(DL_FLOW,"send nalu pkt! len = %d frame_len = %d flag = %d\
			                        pdata = 0x%x pos = 0x%x pstart = 0x%x\n", nalu.len, nLen ,nFlag\
			                        , pData , pos , pstart);
			 */
			SendRtpNalu(&nalu, (unsigned short *)seq, ts_current, roomid, 0, mtu,info);
			//PRINTF("seq =%d,time =%ld\n",seq,ts_current);
		}
	}

	if(send_total_len != nLen) {
		PRINTF("send_total_len = %d,nLen=%d\n", send_total_len, nLen);
	}

	return 0;
}


static unsigned short g_audio_seq_num = 0;
int rtp_build_audio_data(int nLen, unsigned char *pData, int samplerate, int rtp_mtu, unsigned int nowtime,void *info)
{
	int begin_len = 0;
	unsigned char                   sendbuf[1500] = {0};
	//unsigned char                           *sendbuf = gszAudioRtspBuf;
	int                             pLen;
	int                             offset[5] = {0};
	int                             ucFrameLenH = 0, ucFrameLenL = 0;
	int	                            bytes = 0;

	unsigned long                   mytime = 0;
	unsigned long                   ts_current_audio = 0;
	int                     			audio_sample = samplerate; //rtsp_stream_get_audio_samplerate();
	int audio_frame_len = 0;
	int temp_len = 0;

	int i = 0;
	int j = 0;
	//14 = rtp head len +2
	int mtu = rtp_mtu - 14;
	/*��ʱ�汾modify by zm  2012.04.27  */
	//int mtu = nLen + 12;
	int roomid = 0;
	unsigned int timetick = 0;
	int *seq = &g_seq_num;
//	printf("AUDIO nowtime =%u==%u\n",nowtime,g_time_org);
	//printf("nowtime=0x%x,g_time_org=0x%x\n",nowtime,g_time_org);
	if(g_time_org == 0 || nowtime < g_time_org) {
		g_time_org = nowtime;
	}

	timetick = nowtime - g_time_org;


	unsigned int framelen = 0;
	framelen = ((pData[3] & 0x03) << 9) | (pData[4] << 3) | ((pData[5] & 0xe0) >> 5);


	//PRINTF("pData[0] =%x,%x,%x,%x,len=%d\n",pData[0],pData[1],pData[2],pData[3],nLen)	;
	for(i = 0; i < nLen - 4; i++) {
		if((pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x58)
		   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x5c)
		   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x6c)
		   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x60)
		   || (pData[i] == 0xff && pData[i + 1] == 0xf1 && pData[i + 2] == 0x4c)) {

			offset[j] = i;
			j++;
		}
	}

	if(j > 1) {
		PRINTF("RtspAudioPack j=%d=%d\n", j, nLen);
		//printf("pData[0] =%x,%x,%x,%x,len=%d\n",pData[0],pData[1],pData[2],pData[3],nLen)	;
		//printf("pData[0] =%x,%x,%x,%x,len=%d\n",pData[4],pData[5],pData[6],pData[7],nLen)	;
	}

	if(framelen == nLen && j >= 1) {
		j = 1;
	}


	for(i = 0; i < j; i++) {

		pLen = offset[i + 1] - offset[i];

		if(i == j - 1) {
			pLen = nLen - offset[i];
		}

		temp_len = audio_frame_len = pLen - 7;
		//PRINTF("RtspAudioPack  temp_len = %d\n",temp_len);
		mytime = (unsigned long)timetick ;

		while(temp_len > 0) {
			if(temp_len >=  mtu) {
				g_audio_puiNalBuf->len = mtu;
				//PRINTF("temp_len = %d,mtu=%d\n", temp_len, mtu);
				//PRINTF("i=%d,the len =%d=0x%x\n", i, g_audio_puiNalBuf->len, g_audio_puiNalBuf->len);
			} else {
				g_audio_puiNalBuf->len = temp_len;
			}

			ucFrameLenH = audio_frame_len / 32;
			ucFrameLenL = (audio_frame_len % 32) * 8;
			
			g_audio_puiNalBuf->buf[0] = 0;
			g_audio_puiNalBuf->buf[1] = 0x10;
			g_audio_puiNalBuf->buf[2] = ucFrameLenH;
			g_audio_puiNalBuf->buf[3] = ucFrameLenL;
			begin_len =  offset[i] + 7 + audio_frame_len - temp_len;
			memcpy(&g_audio_puiNalBuf->buf[4], pData +begin_len , g_audio_puiNalBuf->len);

			//intf("begin_len=%d,the len=%d\n",begin_len,g_audio_puiNalBuf->len);
			
			temp_len -= 	g_audio_puiNalBuf->len;
			memset(sendbuf, 0, sizeof(sendbuf));

			//mytime += (0xffffffff/44.1-1000*120);
			
			if(audio_sample == 16000) {
				ts_current_audio = mytime * 16;
			} else if(audio_sample == 32000) {
				ts_current_audio = mytime * 32;
			} else if(audio_sample == 44100) {
				ts_current_audio = mytime * 44 +mytime/10;
			} else if(audio_sample == 96000) {
				ts_current_audio =  mytime * 96;
			} else {
				ts_current_audio = mytime * 48;
			}
			//printf("ts_current_audio =%lx,my=%x==%d\n",ts_current_audio,mytime,timetick);
			//if audio time is >0xffffffff,need reset 0;
			if(ts_current_audio >= 0XFFFFFFFF)
			{
				PRINTF("Warnning,the ts_current_audio is too big =%lx\n",ts_current_audio);
				g_time_org = 0;
			}
			//ts_current_audio = timetick;
			//	static int g_test_audio = 0;
			//	g_test_audio ++;
			//	if(g_test_audio %40 ==0)
			//	printf("audio timetick =0x%x,the ts_current_audio  =0x%x\n",timetick,ts_current_audio);
			bytes = g_audio_puiNalBuf->len + 16 ;

			RTP_FIXED_HEADER_AUDIO *p;
			p = (RTP_FIXED_HEADER_AUDIO *)sendbuf;
			p->byte1 = 0x80;

			if(temp_len != 0) {
				p->byte2 = 0x61;
			} else {
				p->byte2 = 0xe1;
			}

			p->seq_no = htons(g_audio_seq_num++);
			p->timestamp = htonl(ts_current_audio);
			p->ssrc = htonl(97);
			memcpy(&sendbuf[12], g_audio_puiNalBuf->buf, g_audio_puiNalBuf->len + 4);

			if(j == 1) {
				//		SendRtspAudioData(sendbuf,bytes,roomid);
				SendMultCastAudioData(sendbuf, bytes,info);
			} else {
				PRINTF("ERROR!!!!SendRtspAudioData not send.\n");
			}
		
		}

	}

	//printf("audio bytes =%d,the len=%d\n",bytes,nLen);
	return 0;
}

int rtp_build_reset_time()
{

	g_time_org = 0;
}
