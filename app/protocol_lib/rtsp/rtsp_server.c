/******************************************************************
*
*rtsp.c
*============================================================================
* This file is used for formating h.264/aac  to rtsp flow
* Ver      alpha_1.0
* Author  jbx
* Shenzhen reach 2010.9.6
*============================================================================
*******************************************************************/
#ifdef USE_LINUX_PLATFORM
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>
#endif

#ifdef USE_WINDOWS_PLATFORM
#include <stdio.h>
#include   <afx.h>
#include <iostream>
#include <process.h>
#include <stdlib.h>
#include <math.h>
#include <winsock2.h>
#endif

#include "mid_platform.h"
#include "rtsp_client.h"
#include "rtsp_stream_deal.h"
#include "rtsp_server.h"
#include "rtsp_authentication.h"
#include "app_rtsp_center.h"

#ifdef USE_LINUX_PLATFORM
#include "../log/log_common.h"
#endif
/*VLC connect information*/
static RTSPCliParam           		 	gRTSPCliPara[MAX_RTSP_CLIENT_NUM];
//thread id
static mid_plat_pthread_t 			rtsp_threadid[MAX_RTSP_CLIENT_NUM]  ;
//have a rtsp client
static int                                   	g_rtsp_flag = 0;
static char 						g_local_ip[64]  = {0};

/*Resize Param table*/
typedef struct __THREAD_PARAM__ {
	int                 				nPos;
	mid_plat_socket                 	client_socket;
	mid_plat_sockaddr_in  		client_addr;
} Thread_Param;

enum{
	INVALID_CLIENT = -1,
	VLC_CLIENT = 0,
	QT_CLIENT,
	STB_TS_CLIENT,
	DEFAULT_CLIENT
};


static int SendAnnounce(int socket, mid_plat_sockaddr_in Listen_addr, char *localip , STR_CLIENT_MSG *strMsg);
static int ResponseTeardown(mid_plat_socket sock, STR_CLIENT_MSG *strMsg);
static void rtsp_change_video_resolve(int roomid);
static int rtsp_get_client_timeout(void);

//add by zhangmin
#ifdef GDM_RTSP_SERVER
static CString GetCurrectIP(void);
static CString GetCurrectIP()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	char name[255];
	CString ip;
	PHOSTENT hostinfo;
	wVersionRequested = MAKEWORD(2, 0);

	if(WSAStartup(wVersionRequested, &wsaData) == 0) {
		if(gethostname(name, sizeof(name)) == 0) {
			if((hostinfo = gethostbyname(name)) != NULL) {
				ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);
			}
		}

		WSACleanup();
	}

	return ip;
}

//get server local ip
int rtsp_get_local_ip(char *buff, int len)
{
	CString ip ;

	if(strlen(g_local_ip) == 0) {
		ip = GetCurrectIP();
		memcpy(g_local_ip, ip.GetBuffer(ip.GetLength()), ip.GetLength());
	}

	PRINTF("g_local_ip = #%s#\n", g_local_ip);

	if(len > strlen(g_local_ip)) {
		len = strlen(g_local_ip);
	}

	memcpy(buff, g_local_ip, len);
	return 0;

}

#else

int rtsp_set_local_ip(unsigned int localipaddr)
{
	struct in_addr          addr_ip;
	char local_ip[16] = {0};
	memcpy(&addr_ip, &localipaddr, 4);
	memcpy(local_ip, inet_ntoa(addr_ip), 16);

	strcpy(g_local_ip, local_ip);
	return 0;
}

//get server local ip
int rtsp_get_local_ip(char *buff, int len)
{
	strcpy(buff, g_local_ip);
	return 0;
}
#endif


static unsigned long getCurrentTimets(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned long ultime;

	gettimeofday(&tv , &tz);
	ultime = tv.tv_sec * 1000 + ((tv.tv_usec) / 1000);
	return (ultime);
}
static unsigned int getruntimets(void)
{
	unsigned int msec;
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	msec = tp.tv_sec;
	msec = msec * 1000 + tp.tv_nsec / 1000000;
	//printf("mid_clock ultime=%u==%u\n",msec,tp.tv_sec);
	return msec;
}

/**************************************************************************************************
                                        为NALU_t结构体分配内存空间
**************************************************************************************************/
static NALU_t *AllocNALU(int buffersize)
{
	NALU_t *n;

	if((n = (NALU_t *)calloc(1, sizeof(NALU_t))) == NULL) {
		exit(0);
	}

	n->max_size = buffersize;

	if((n->buf = (unsigned char *)calloc(buffersize, sizeof(char))) == NULL) {
		free(n);
		exit(0);
	}

	return n;
}



/*********************************************************************************************************
                                        为NALU_t结构体释放内存空间
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

static int GetMsgType(char *pDataFromClient, int usDataLen)
{
	char *p = pDataFromClient;
	int itype = -1;
	int m;

	for(m = 0; m < usDataLen - 7; m++) {
		if(p[m] == 0x4f && p[m + 1] == 0x50 && p[m + 2] == 0x54 && p[m + 3] == 0x49 \
		   && p[m + 4] == 0x4f && p[m + 5] == 0x4e) { //receive option
			itype = OPTION;
			break;
		} else if(p[m] == 0x44 && p[m + 1] == 0x45 && p[m + 2] == 0x53 && p[m + 3] == 0x43 \
		          && p[m + 4] == 0x52 && p[m + 5] == 0x49 && p[m + 6] == 0x42 && p[m + 7] == 0x45) {
			itype = DESCRIBE;
			break;
		} else if(p[m] == 0x53 && p[m + 1] == 0x45 && p[m + 2] == 0x54 && p[m + 3] == 0x55 \
		          && p[m + 4] == 0x50) {
			itype = SETUP;
			break;
		} else if(p[m] == 0x50 && p[m + 1] == 0x4c && p[m + 2] == 0x41 && p[m + 3] == 0x59) {
			itype = PLAY;
			break;
		} else if(p[m] == 0x50 && p[m + 1] == 0x41 && p[m + 2] == 0x55 && p[m + 3] == 0x53 \
		          && p[m + 4] == 0x45) {
			itype = PAUSE;
			break;
		} else if(p[m] == 0x54 && p[m + 1] == 0x45 && p[m + 2] == 0x41 && p[m + 3] == 0x52 \
		          && p[m + 4] == 0x44 && p[m + 5] == 0x4f && p[m + 6] == 0x57 && p[m + 7] == 0x4e) {
			itype = TEARDOWN;
			break;
		} //GET_PARAMETER
		else if(p[m] == 'G' && p[m + 1] == 'E' && p[m + 2] == 'T' && p[m + 3] == '_' \
		        && p[m + 4] == 'P' && p[m + 5] == 'A' && p[m + 6] == 'R' && p[m + 7] == 'A'\
		        && p[m + 8] == 'M' && p[m + 9] == 'E' && p[m + 10] == 'T' && p[m + 11] == 'E' && p[m + 12] == 'R') {
			itype = GET_PARAMETER;
			break;
		} //SET_PARAMETER
		else if(p[m] == 'S' && p[m + 1] == 'E' && p[m + 2] == 'T' && p[m + 3] == '_' \
		        && p[m + 4] == 'P' && p[m + 5] == 'A' && p[m + 6] == 'R' && p[m + 7] == 'A'\
		        && p[m + 8] == 'M' && p[m + 9] == 'E' && p[m + 10] == 'T' && p[m + 11] == 'E' && p[m + 12] == 'R') {
			itype = SET_PARAMETER;
			break;
		} else {
			;
		}
	}

	return itype;

}



static int GetCSeq(char *pDataFromClient, int usDataLen, STR_CLIENT_MSG *strMsg)
{
	char *p = pDataFromClient;
	int i = 0;
	char aucCSeq[10];
	unsigned int uiCSeq = 0;
	int m, j;

	for(m = 0; m < usDataLen; m++) {
		if(p[m] == 0x43 && p[m + 1] == 0x53 && p[m + 2] == 0x65 && p[m + 3] == 0x71 \
		   && p[m + 4] == 0x3a) { //CSeq
			while(((m + 6 + i) < usDataLen) && (p[m + 6 + i] != 0x0a)) {
				aucCSeq[i] = p[m + 6 + i] - 0x30;
				strMsg->aucCSeqSrc[i] = p[m + 6 + i];
				i++;
			}

			strMsg->ucCSeqSrcLen = i - 1;

			for(j = 0; j < i - 1; j++) {
				uiCSeq += aucCSeq[j] * pow(10, (i - 2 - j));
			}

			break;
		}
	}

	//	PRINTF("uicseq = %d\n",uiCSeq);
	return uiCSeq;
}


static int ConventNumberToString(char *pString, int uiNumber)
{
	int i = 0;
	int j = 0;
	char dat[5];

	for(i = 0; i < 5; i++) {
		dat[0] = uiNumber / 10000;
		dat[1] = (uiNumber % 10000) / 1000;
		dat[2] = (uiNumber % 1000) / 100;
		dat[3] = (uiNumber % 100) / 10;
		dat[4] = uiNumber % 10;
	}

	for(j = 0; j < 5; j++) {
		if(dat[j] == 0) {
			i--;
		} else {
			break;
		}
	}

	for(j = 0; j < i; j++) {

		pString[j] = dat[5 - i + j] + 0x30;
	}

	return i;
}


//**************************************************************************
/*RTSP 初始化*/
//**************************************************************************
static int RtspThreadInit(void)
{
	int cli;

	for(cli = 0; cli < MAX_RTSP_CLIENT_NUM; cli++) {
		SETRTSPUDPREADY(0, cli, 0);
		SETRTSPUSED(0, cli, FALSE);
		SETRTSPUDPAUDIOSOCK(0, cli, 0);
		SETRTSPUDPVIDEOSOCK(0, cli, 0);
		SETRTSPROOMID(0, cli, -1);
		SETRTSPCLIENTTYPE(0, cli, VLC_CLIENT);
	}

	return 0;
}
//**************************************************************************
/*RTSP server exit*/
//**************************************************************************
static int RtspExit()
{
	int cli;

	for(cli = 0; cli < MAX_RTSP_CLIENT_NUM; cli++) {
		SETRTSPUDPREADY(0, cli, 0);
		SETRTSPUSED(0, cli, FALSE);
		SETRTSPUDPAUDIOSOCK(0, cli, 0);
		SETRTSPUDPVIDEOSOCK(0, cli, 0);
		SETRTSPROOMID(0, cli, -1);
	}

	return 0;
}


//**************************************************************************
/*get null RTSPclient index*/
//**************************************************************************
static int RtspGetNullClientIndex()
{
	int cli ;

	for(cli = 0; cli < MAX_RTSP_CLIENT_NUM; cli++) {
		if((!ISRTSPUSED(0, cli))) {
			//need some new
			SETRTSPUSED(0, cli, TRUE);
			SETRTSPCLIENTTYPE(0, cli, VLC_CLIENT);
			SETRTSPUDPAUDIOSOCK(0, cli, -1);
			SETRTSPUDPVIDEOSOCK(0, cli, -1);
			return cli;
		}
	}

	return -1;
}

//**************************************************************************
/*get RTSP clients' number*/
//**************************************************************************
static int RtspGetClientNum()
{
	int cli ;
	int count = 0;

	for(cli = 0; cli < MAX_RTSP_CLIENT_NUM; cli++) {
		if((ISRTSPUSED(0, cli))) {
			count = count + 1;
		}
	}

	return count;
}

/*********************************************************************************************************
                                        获取连接用户信息
*********************************************************************************************************/
static int GetVlcMsgType(char *pDataFromClient, int usDataLen, STR_CLIENT_MSG *strMsg)
{
	int type;
	strMsg->iType = GetMsgType(pDataFromClient, usDataLen);
	strMsg->uiCseq = GetCSeq(pDataFromClient, usDataLen, strMsg);

	if(strMsg->iType >= MAX_OPERATE || strMsg->uiCseq > 65535) {
		return 0;
	}

	type = strMsg->iType;
	return type;
}

//*******************************************************************************************************************
/*设置建立信息信息*/
//*******************************************************************************************************************
static void PraseSETUPMsg(char *pDataFromClient, int usDataLen, STR_CLIENT_MSG *strMsg)
{
	int i = 0, j = 0;
	char *p = pDataFromClient;
	strMsg->ucTransNum = 0;

	for(i = 0; i < usDataLen; i++) {
		/*Transport*/
		if(p[i] == 0x54 && p[i + 1] == 0x72 && p[i + 2] == 0x61 && p[i + 3] == 0x6e && p[i + 4] == 0x73 && p[i + 5] == 0x70 &&
		   p[i + 6] == 0x6f && p[i + 7] == 0x72 && p[i + 8] == 0x74 && p[i + 9] == 0x3a) {
			for(j = 0; j < 100; j++) {
				if(p[i + 11 + j] != 0x0a) {
					strMsg->aucTrans[j] = p[i + 11 + j];
					strMsg->ucTransNum++;
				} else {
					strMsg->ucTransNum -= 1;
					return;
				}
			}

			strMsg->ucTransNum -= 1;
			//			PRINTF("g_strMsg.ucTransNum = %d,g_strMsg.aucTrans[0] =%c\n",strMsg->ucTransNum,strMsg->aucTrans[0]);

			return;
		}
	}

	strMsg->ucTransNum -= 1;
	return;
}

//*******************************************************************************************************************
/*获取客户端端口*/
//*******************************************************************************************************************
static int GetClientPort(char *pDataFromClient, int usDataLen, STR_CLIENT_MSG *strMsg, int trick_id)
{
	char *p = pDataFromClient;
	unsigned int i = 0;
	int m;
	strMsg->ucClientportLen[trick_id] = 0;

	for(m = 0; m < usDataLen; m++) {
		if(p[m] == 0x63 && p[m + 1] == 0x6c && p[m + 2] == 0x69 && p[m + 3] == 0x65 && p[m + 4] == 0x6e && p[m + 5] == 0x74 \
		   && p[m + 6] == 0x5f && p[m + 7] == 0x70 && p[m + 8] == 0x6f && p[m + 9] == 0x72 && p[m + 10] == 0x74 && p[m + 11] == 0x3d) { //client_port=
			while(p[m + 12 + i] != 0x3b && p[m + 12 + i] != 0x0d && p[m + 13 + i] != 0x0a) { //not  ";"  "nler"
				strMsg->aucClientport[trick_id][i] = p[m + 12 + i];
				strMsg->ucClientportLen[trick_id]++;
				i++;
			}
		}
	}

	//	PRINTF("g_strMsg.ucClientportLen=%d,g_strMsg.aucClientport=%c\n",strMsg->ucClientportLen,strMsg->aucClientport[strMsg->timeused][0]);
	if(i < MIN_CLIENT_PORT_BIT) {
		return ERR_CLIENT_MSG_PORT_TOO_SHORT;
	} else {
		return TRUE;
	}
}


//*******************************************************************************************************************
/*响应rtsp客户端*/
//*******************************************************************************************************************
static int ResponseOption(mid_plat_socket sock, STR_CLIENT_MSG *strMsg)
{
	int iSendDataLen = 0;
	int i = 0;
	char  pSend[TCP_BUFF_LEN] = {0};
	sprintf(pSend, "RTSP/1.0 200 OK\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\nPublic: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE,GET_PARAMETER,SET_PARAMETER\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	PRINTF("\n%s\n", pSend);
	return iSendDataLen;
}

static int ResponseGet_parameter(mid_plat_socket sock, STR_CLIENT_MSG *strMsg)
{
	int iSendDataLen = 0;
	int i = 0;
	char  pSend[TCP_BUFF_LEN] = {0};
	sprintf(pSend, "RTSP/1.0 200 OK\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	PRINTF("\n%s\n", pSend);
	return iSendDataLen;
}

static int ResponseSet_parameter(mid_plat_socket sock, STR_CLIENT_MSG *strMsg)
{
	int iSendDataLen = 0;
	int i = 0;
	char  pSend[TCP_BUFF_LEN] = {0};
	sprintf(pSend, "RTSP/1.0 200 OK\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	PRINTF("\n%s\n", pSend);
	return iSendDataLen;
}


//describe authentication
static int rtsp_describe_authentication(char *buff, int len, char *localip, int *qtflag, int *roomid, char *filename, int *need_timeout)
{


	if(buff == NULL || len < strlen("DESCRIBE RTSP/1.0")) {
		return -1;
	}

	char user[128] = {0};
	//	char passwd[128] = {0};
	char url[1024] = {0};
	int url_len = 0;
	char *temp = NULL;

	char local_ip[16] = {0};
	char url2[1024] = {0};
	int id = -1;
	char room[1024] = {0};
	int ret = 0;

	if(strstr(buff, "QuickTime") != NULL) {
		*qtflag = QT_CLIENT;
	} else if(strstr(buff, "VLC") != NULL) {
		*qtflag = VLC_CLIENT;
	} else {
		*qtflag = VLC_CLIENT;
	}

	//just for the stb
	if(strstr(buff, ".ts") != NULL) {
		*qtflag = STB_TS_CLIENT;
	}

	//just for client timeout
	if(strstr(buff, "timeout") != NULL) {
		*need_timeout = 1;
	}

	PRINTF("*need_timeout = %d\n", *need_timeout);

	if(strncmp(buff, "DESCRIBE ", 9) != 0) {
		PRINTF("ERROR!!!rtsp_describe_authentication 1.\n");
		return -1;
	}

	//关闭授权
#ifndef GDM_RTSP_SERVER

	if(1) {
		*roomid = 0;
		return 0;
	}

#endif
	temp = strstr(buff + 9, " RTSP/");

	if(temp == NULL) {
		PRINTF("ERROR!!!rtsp_describe_authentication 2.\n");
		return -1;
	}

	url_len = temp - (buff + 9) ;

	if(url_len < strlen("rtsp://0.0.0.0")) {
		PRINTF("ERROR!!!rtsp_describe_authentication 3.\n");
		return -1;
	}

	//	PRINTF("url_len = %d\n",url_len);
	strncpy(url, buff + 9, url_len);

#if 1
	strcpy(local_ip, localip);
	sprintf(url2, "rtsp://%s:554/card", local_ip);
#else
	strcpy(url2, url);
	temp = strstr(url2, "/card");

	if(temp == NULL) {
		ERR_PRN("\n");
		return -1;
	}

	*(temp + strlen("/card")) = '\0';
#endif

	//	PRINTF("client describe url is %s,the must url is %s.\n",url,url2);
	if(strncmp(url, url2, strlen(url2)) != 0) {
		PRINTF("ERROR!!!rtsp_describe_authentication 4.\n");
		return -1;
	}

	if(*(url + strlen(url2) + 1) != '?') {
		PRINTF("ERROR!!!rtsp_describe_authentication 5.\n");
		return -1;
	}

	//snprintf(room,2,"%s",url+strlen(url2) );
	memcpy(room, (url + strlen(url2)), 1);
	id = atoi(room);

	PRINTF("=%s=,room=#%s#,roomid = %d\n", url + strlen(url2) , room, id);

	if(id < 0 || id > rtsp_stream_get_room_num() - 1) {
		PRINTF("ERROR!!!rtsp_describe_authentication 6.\n");
		return -1;
	}

	*roomid = id;


	//	if((temp =strchr(url+strlen(url2) + 2,' ')) == NULL)
	//	{
	//		return -1;
	//	}
	//	strcpy(passwd,temp+1);
	//	*temp = '\0';
	strcpy(user, url + strlen(url2) + 2);
	PRINTF("user =$%s$\n", user);

#ifdef GDM_RTSP_SERVER
	ret =  app_rtsp_authentication_begin(user);

	if(ret  == 0) {
		strcpy(filename, user);
	}

#endif

	return ret ;
}


//*******************************************************************************************************************
/*向rtsp客户端发送流信息*/
//*******************************************************************************************************************
#define VIDEO_TRACKID 1
#define AUDIO_TRACKID 2
static int ResponseDescribe(mid_plat_socket socket, mid_plat_sockaddr_in Listen_addr, char *localip , int authencation, STR_CLIENT_MSG *strMsg)
{
	int                     	iSendDataLen = 0, iSendDataLen2 = 0, i = 0;
	int                     	usStringLen = 0;
	int                     	listen_port;
	int                     	audio_sample = 48000;
	char                    	pSend2[1000];
	char                    	local_ip[16] = {0};
	char                    	client_ip[16];
	char                  	pSend[TCP_BUFF_LEN] = {0};
	int 				roomid = strMsg->roomid;
	int 				sdpinfo_len = 0;
	char 			sdpinfo[1024] = {0};
	int				ret = 0;
	char				audio_config[16] = {0};
	int 				a_ret = 0;
	PRINTF("\n");
	a_ret = rtsp_stream_get_audio_sinfo(0, audio_config, &audio_sample);

	if(a_ret < 0) {
		PRINTF("Error\n");
	}

	int mult = 0;
	char ip[32] = {0};
	int vport = 0;
	int aport = 0;
	rtsp_porting_get_ginfo(&mult, ip, &vport, &aport);

	if(mult == 0 || strlen(ip) == 0) {
		strcpy(ip, localip);
	}

	sdpinfo_len =  rtsp_stream_get_video_sdp_info(sdpinfo, roomid);
#ifndef GDM_RTSP_SERVER

	if(rtsp_porting_server_need_stop() == 1) {
		sdpinfo_len = -1;
	}

#endif

	if(authencation == -1 || sdpinfo_len == -1) {
		PRINTF("authencation=%d,sdpinfo_len=%d\n", authencation, sdpinfo_len);
		sprintf(pSend, "RTSP/1.0   404   Stream Not Found\r\nCSeq: %d\r\n\r\n", strMsg->uiCseq);
		iSendDataLen = strlen(pSend);
		send(socket, pSend, iSendDataLen, 0);
		PRINTF("\n%s\n", pSend);
		return -1;
	}

	sprintf(local_ip, "%s", localip);
	memcpy(client_ip, inet_ntoa(Listen_addr.sin_addr), 16);
	listen_port = RTSP_LISTEN_PORT;
	sprintf(pSend2, "v=0\r\no=- 2890844256 2890842807 IN IP4 %s\r\nc=IN IP4 %s\r\nt=0 0\r\n", local_ip, ip); //inet_ntoa(Listen_addr.sin_addr));
	iSendDataLen2 = strlen(pSend2);

	sprintf(&pSend2[iSendDataLen2], "m=video %u RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\n", vport);
	iSendDataLen2 = strlen(pSend2);

	if(sdpinfo_len != -1) {
		sprintf(&pSend2[iSendDataLen2], "a=fmtp:96 packetization-mode=1;profile-level-id=428028;sprop-parameter-sets=%s\r\n", sdpinfo);
		iSendDataLen2 = strlen(pSend2);
	}

	sprintf(&pSend2[iSendDataLen2], "a=control:rtsp://%s:%d/trackID=%d\r\n", local_ip, listen_port, VIDEO_TRACKID);
	iSendDataLen2 = strlen(pSend2);

	//only the card0 have audio
	if(roomid == 0) {
		sprintf(&pSend2[iSendDataLen2], "m=audio %u RTP/AVP 97\r\na=rtpmap:97 mpeg4-generic/%d/2\r\n", aport, audio_sample);
		iSendDataLen2 = strlen(pSend2);

		//ret = rtsp_stream_get_audio_sdp_info(audio_config);
#if 1

		if(a_ret < 0) {
			PRINTF("ERROR,get audio sdp failed .\n");
		} else {
			sprintf(&pSend2[iSendDataLen2], "a=fmtp:97 streamtype=5; mode=AAC-hbr; config=%s; SizeLength=13; IndexLength=3; IndexDeltaLength=3\r\na=control:rtsp://%s:%d/trackID=%d\r\n", audio_config, local_ip, listen_port, AUDIO_TRACKID);
			iSendDataLen2 = strlen(pSend2);
		}

#endif
	}

	strcpy(pSend, "RTSP/1.0 200 OK\r\nContent-type: application/sdp\r\nServer: RRS 0.1\r\nContent-Length: ");
	iSendDataLen = strlen(pSend);
	usStringLen = ConventNumberToString(&pSend[iSendDataLen], iSendDataLen2);
	strcpy(&pSend[iSendDataLen + usStringLen], "\r\nCache-Control: no-cache\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(socket, pSend, iSendDataLen, 0);
	PRINTF("\n%s", pSend);
	send(socket, pSend2, iSendDataLen2, 0);
	PRINTF("\n%s\n", pSend2);
	return (iSendDataLen + iSendDataLen2);

}


static int ResponseDescribeTs(mid_plat_socket socket, mid_plat_sockaddr_in Listen_addr, char *localip , int authencation, STR_CLIENT_MSG *strMsg)
{
	int                     	iSendDataLen = 0, iSendDataLen2 = 0, i = 0;
	int                     	usStringLen = 0;
	int                     	listen_port;
	//int                     	audio_sample = rtsp_stream_get_audio_samplerate();
	char                    	pSend2[1000];
	char                    	local_ip[16] = {0};
	char                    	client_ip[16];
	char                  	pSend[TCP_BUFF_LEN] = {0};
	int 				roomid = strMsg->roomid;
	int 				sdpinfo_len = 0;
	char 			sdpinfo[1024] = {0};
	//	int				ret = 0;

	sdpinfo_len =  rtsp_stream_get_video_sdp_info(sdpinfo, roomid);
#ifndef GDM_RTSP_SERVER

	if(rtsp_porting_server_need_stop() == 1) {
		sdpinfo_len = -1;
	}

#endif

	if(authencation == -1 || sdpinfo_len == -1) {
		sprintf(pSend, "RTSP/1.0   404   Stream Not Found\r\nCSeq: %d\r\n\r\n", strMsg->uiCseq);
		iSendDataLen = strlen(pSend);
		send(socket, pSend, iSendDataLen, 0);
		PRINTF("\n%s\n", pSend);
		return -1;
	}

	sprintf(local_ip, "%s", localip);
	memcpy(client_ip, inet_ntoa(Listen_addr.sin_addr), 16);
	listen_port = RTSP_LISTEN_PORT;
	sprintf(pSend2, "v=0\r\no=- %u %u IN IP4 %s\r\n", getCurrentTimets(), getCurrentTimets(), local_ip);
	iSendDataLen2 = strlen(pSend2);

	sprintf(&pSend2[iSendDataLen2], "s=MPEG Transport Stream, streamed by the LIVE555 Media Server\r\ni=3.ts\r\nt=0 0\r\na=tool:LIVE555 Streaming Media v2011.01.19\r\n");
	iSendDataLen2 = strlen(pSend2);
	sprintf(&pSend2[iSendDataLen2], "a=type:broadcast\r\na=control:* \r\na=range:npt=0-\r\n");
	iSendDataLen2 = strlen(pSend2);

	sprintf(&pSend2[iSendDataLen2], "a=x-qt-text-nam:MPEG Transport Stream, streamed by the LIVE555 Media Server\r\na=x-qt-text-inf:3.ts\r\nm=video 0 RTP/AVP 33\r\nc=IN IP4 0.0.0.0\r\nb=AS:5000\r\na=control:track%d\r\n", VIDEO_TRACKID);
	//	sprintf(&pSend2[iSendDataLen2],"a=x-qt-text-nam:MPEG Transport Stream, streamed by the LIVE555 Media Server \r\na=x-qt-text-inf:3.ts \r\nm=video 0 RTP/AVP 33 \r\nc=IN IP4 0.0.0.0 \r\nb=AS:5000 \r\na=control:rtsp://%s:%d/trackID=%d \r\n",local_ip,listen_port,VIDEO_TRACKID);
	iSendDataLen2 = strlen(pSend2);





	strcpy(pSend, "RTSP/1.0 200 OK\r\nCseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	iSendDataLen = strlen(pSend);
	strcpy(&pSend[iSendDataLen], "\r\nContent-Type: application/sdp\r\nServer: RRS 0.1\r\nCache-Control: no-cache\r\nContent-Length: ");
	iSendDataLen = strlen(pSend);
	usStringLen = ConventNumberToString(&pSend[iSendDataLen], iSendDataLen2);
	strcpy(&pSend[iSendDataLen + usStringLen], "\r\n");
	iSendDataLen = strlen(pSend);
	strcpy(&pSend[iSendDataLen], "\r\n");
	iSendDataLen = strlen(pSend);
	send(socket, pSend, iSendDataLen, 0);
	PRINTF("\n%s", pSend);
	send(socket, pSend2, iSendDataLen2, 0);
	PRINTF("\n%s\n", pSend2);
	return (iSendDataLen + iSendDataLen2);

}




//*******************************************************************************************************************
/*建立响应*/
//*******************************************************************************************************************
static int rtsp_find_trackid(char *buff, int len)
{
	if(buff  == NULL) {
		return -1;
	}

	char *p = NULL;
	char tempbuff[1024] = {0};
	int id = 0;
	sprintf(tempbuff, "%s", buff);
	p = strstr(tempbuff, "/trackID=");
	PRINTF("\n");

	if(p == NULL) {
		return -1;
	}

	p = p + strlen("/trackID=") ;
	*(p + 1) = '\0';
	id = atoi(p);
	PRINTF("i find the trackid = %d\n", id);
	return id;
}

static int ResponseSetup(char *pReceiveBuf, int ReceiveBufLen, mid_plat_socket sock, STR_CLIENT_MSG *strMsg, int timeout)
{
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TCP_BUFF_LEN] = {0};
	int is_video = 1;
	int id = 0;
	id = rtsp_find_trackid(pReceiveBuf, ReceiveBufLen);

	if(id == VIDEO_TRACKID) {
		is_video = 1;
	} else if(id == AUDIO_TRACKID) {
		is_video = 2;
	} else {
		is_video = 1;
	}

	PraseSETUPMsg(pReceiveBuf, ReceiveBufLen, strMsg);
	GetClientPort(pReceiveBuf, ReceiveBufLen, strMsg, is_video - 1);

	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\nTransport: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucTransNum; i++) {
		pSend[iSendDataLen + i] = strMsg->aucTrans[i];
	}

	for(i = 0; i < strMsg->ucClientportLen[strMsg->timeused]; i++) {
		pSend[iSendDataLen + strMsg->ucTransNum + i ] = strMsg->aucClientport[strMsg->timeused][i];
	}

	//need client timeout
	if(timeout != 0) {
		sprintf(&pSend[iSendDataLen + strMsg->ucTransNum + strMsg->ucClientportLen[strMsg->timeused]], "\r\nContent-Length: 0\r\nCache-Control: no-cache\r\nSession: 11478; timeout=%d\r\nCseq: ", timeout);
	} else {
		strcpy(&pSend[iSendDataLen + strMsg->ucTransNum + strMsg->ucClientportLen[strMsg->timeused]], "\r\nContent-Length: 0\r\nCache-Control: no-cache\r\nSession: 11478\r\nCseq: ");
	}

	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	PRINTF("\n%s\n", pSend);
	strMsg->timeused = strMsg->timeused + 1;

	if(strMsg->timeused == 2) {
		strMsg->timeused = 0;
	}

	return iSendDataLen;
}


//*******************************************************************************************************************
/*响应播放*/
//*******************************************************************************************************************
static int ResponsePlay(mid_plat_socket sock, STR_CLIENT_MSG *strMsg)
{
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TCP_BUFF_LEN] = {0};
	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\nSession: 11478\r\nRange: npt=0.000- \r\n");
	iSendDataLen = strlen(pSend);

	sprintf(&pSend[iSendDataLen], "Cseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	PRINTF("\n%s\n", pSend);

	return iSendDataLen;
}



//*******************************************************************************************************************
/*响应 pause*/
//*******************************************************************************************************************
//pause
static int RtspPauseClient(int nPos)
{
	mid_plat_socket socket = -1;
	SETRTSPUDPREADY(0, nPos, 0);
	//	SETRTSPROOMID(0, nPos, -1);
	//	SETRTSPUSED(0, nPos, FALSE);
	socket = GETRTSPUDPAUDIOSOCK(0, nPos);
	mid_plat_close_socket(socket);
	SETRTSPUDPAUDIOSOCK(0, nPos, -1);
	socket = GETRTSPUDPVIDEOSOCK(0, nPos);
	mid_plat_close_socket(socket);

	SETRTSPUDPVIDEOSOCK(0, nPos, -1);

	char *client = NULL;
	client = GETRTSPCLIENT(0, nPos);
	rtsp_porting_close_client(nPos,client);
	client = NULL;
	SETRTSPCLIENT(0, nPos, client);
	//	SETRTSPCLIENTTYPE(0,nPos,VLC_CLIENT);
	return 0;
}

static int ResponsePause(mid_plat_socket sock, STR_CLIENT_MSG *strMsg)
{
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TCP_BUFF_LEN] = {0};
	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\nSession: 11478\r\n");
	iSendDataLen = strlen(pSend);

	sprintf(&pSend[iSendDataLen], "Cseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}

	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	PRINTF("\n%s\n", pSend);
	return iSendDataLen;
}





//*************************************************************************************************
/*退出*/
//*************************************************************************************************/
static int ResponseTeardown(mid_plat_socket sock, STR_CLIENT_MSG *strMsg)
{
	int iSendDataLen = 0;
	unsigned int i = 0;
	char  pSend[TCP_BUFF_LEN] = {0};
	strcpy(pSend, "RTSP/1.0 200 OK\r\nServer: RRS 0.1\r\n");
	iSendDataLen = strlen(pSend);


	sprintf(&pSend[iSendDataLen], "Cseq: ");
	iSendDataLen = strlen(pSend);

	for(i = 0; i < strMsg->ucCSeqSrcLen; i++) {
		pSend[iSendDataLen + i] = strMsg->aucCSeqSrc[i];
	}




	strcpy(&pSend[iSendDataLen + i], "\r\n\r\n");
	iSendDataLen = strlen(pSend);
	send(sock, pSend, iSendDataLen, 0);
	PRINTF("\n%s\n", pSend);
	return iSendDataLen;
}


/**************************************************************************************************
                    获取rtsp port
**************************************************************************************************/
static unsigned short g_rtsp_port = RTSP_LISTEN_PORT;
int RtspSetPort(unsigned short port)
{
	g_rtsp_port = port;
	return 0;
}
static int RtspGetPort()
{
	int RtspPort;
	RtspPort = g_rtsp_port;
	return (RtspPort);
}

static unsigned int g_keepactive_time = 60; //s
int rtsp_set_keepactive_time(int s)
{
	if(s < 5) {
		s = 5;
	}

	if(s > 900000) {
		s = 900000;
	}

	g_keepactive_time = s;
	return 0;
}

static unsigned int rtsp_get_keepactive_time()
{
	return g_keepactive_time;
}

//**************************************************************************
/*delete rtsp client*/
//**************************************************************************
static int RtspDelClient(int nPos)
{
	mid_plat_socket socket = -1;
	mid_plat_sockaddr_in temp_addr ;
	char *client = NULL;
	memset(&temp_addr, 0, sizeof(mid_plat_sockaddr_in));

	SETRTSPUDPREADY(0, nPos, 0);
	SETRTSPROOMID(0, nPos, -1);
	SETRTSPUSED(0, nPos, FALSE);
	socket = GETRTSPUDPAUDIOSOCK(0, nPos);
	mid_plat_close_socket(socket);
	SETRTSPUDPAUDIOSOCK(0, nPos, -1);
	socket = GETRTSPUDPVIDEOSOCK(0, nPos);
	mid_plat_close_socket(socket);

	SETRTSPUDPAUDIOADDR(0, nPos, temp_addr);
	SETRTSPUDPVIDEOADDR(0, nPos, temp_addr);
	SETRTSPADDR(0, nPos, temp_addr);

	SETRTSPUDPVIDEOSOCK(0, nPos, -1);
	SETRTSPCLIENTTYPE(0, nPos, VLC_CLIENT);

	client = GETRTSPCLIENT(0, nPos);
	rtsp_porting_close_client(nPos,client);
	client = NULL;
	SETRTSPCLIENT(0, nPos, client);

	return 0;
}

/*
*切换长宽后，需要关闭QT
*/
static void rtsp_change_video_resolve(int roomid)
{
	int cli  = 0;
	char *client = NULL;
	//	PRINTF("g_rtsp_flag=%d\n",g_rtsp_flag);
	if(g_rtsp_flag != 1) {
		PRINTF("warnning ,the rtsp have no client .\n");
		return ;
	}

	for(cli = 0; cli < MAX_RTSP_CLIENT_NUM; cli++) {
		//vlc 也关闭
		if((roomid == GETRTSPROOMID(0,cli)) && (GETRTSPCLIENTTYPE(0, cli) == QT_CLIENT))
		//if(roomid == GETRTSPROOMID(0, cli)) 
		{
			SETRTSPCLIENTTYPE(0, cli, INVALID_CLIENT);

			client = GETRTSPCLIENT(0, cli);
			rtsp_porting_close_client(cli,client);
			client = NULL;
			SETRTSPCLIENT(0, cli, client);
		
			PRINTF("the video resolve is change ,the %d client is invalid,the room is %d.\n", cli, roomid);
		}
	}

	return ;
}


//*******************************************************************************************************************
/*add rtsp udp client*/
//*******************************************************************************************************************

static int RtspAddClient(mid_plat_sockaddr_in client_addr, int nPos, STR_CLIENT_MSG *strMsg)
{
	//int      UdpRtspVideoSocket = 0;
	//int      UdpRtspAudioSocket = 0;
	int 	   have_audio_socket = 0;
	int      buf_num;
#ifndef GDM_RTSP_SERVER
	struct timeval tv;
#endif
	mid_plat_sockaddr_in temp_client_addr;
	mid_plat_sockaddr_in temp_video_addr;
	mid_plat_sockaddr_in temp_audio_addr;

	mid_plat_sockaddr_in video_addr;
	mid_plat_sockaddr_in audio_addr;
	unsigned int socklen;
	int m, n;

	int video_port = 0;

	int audio_port = 0;

	int roomid = strMsg->roomid;
	char *client = NULL;
	int mult = 0;
	char ip[32] = {0};
	int vport = 0;
	int aport = 0;



	socklen = sizeof(mid_plat_sockaddr_in);
	//	memset(&peeraddr, 0, socklen);
	memset(&video_addr, 0, socklen);
	memset(&audio_addr, 0, socklen);

	for(m = 0; m < strMsg->ucClientportLen[0]; m++) {
		if(strMsg->aucClientport[0][m] == 0x2d) {
			break;
		}
	}

	for(n = 0; n < m; n++) {
		video_port += (strMsg->aucClientport[0][n] - 0x30) * pow(10, (m - n - 1));
	}

	PRINTF("video socket m=%d,port=%d,audio_port = %d\n", m, video_port);

	//must have video port else will return -1;
	if(m == 0 || video_port == 0) {
		PRINTF("ERROR,the video socket port is error,m=%d,video_port= %d\n", m, video_port);
		return -1;
	}

	// bzero(&peeraddr,socklen);
	//memset(&peeraddr,0,socklen);

	rtsp_porting_get_ginfo(&mult, ip, &vport, &aport);

	if(mult == 1) {
		inet_pton(AF_INET, ip, &video_addr.sin_addr);
	} else {
		video_addr.sin_addr.s_addr = client_addr.sin_addr.s_addr;
	}

	//   	video_addr.sin_addr.s_addr = client_addr.sin_addr.s_addr;
	video_addr.sin_port = htons(video_port);
	video_addr.sin_family = AF_INET;


	for(m = 0; m < strMsg->ucClientportLen[1]; m++) {
		if(strMsg->aucClientport[1][m] == 0x2d) {
			break;
		}
	}

	for(n = 0; n < m; n++) {
		audio_port += (strMsg->aucClientport[1][n] - 0x30) * pow(10, (m - n - 1));
	}

	PRINTF("aduio socket m=%d,port=%d\n", m, audio_port);

	if(m == 0 || audio_port == 0) {
		PRINTF("WARNNING,the audio socket port is error,m=%d,audio_port= %d\n", m, audio_port);
		have_audio_socket = -1;
	} else {
		//bzero(&peeraddr,socklen);
		memset(&audio_addr, 0, socklen);

		if(mult == 1) {
			inet_pton(AF_INET, ip, &audio_addr.sin_addr);
		} else {
			audio_addr.sin_addr.s_addr = client_addr.sin_addr.s_addr;
		}

		audio_addr.sin_port = htons(audio_port);
		audio_addr.sin_family = AF_INET;
	}


	//compare the all client socket
	//if have repeat
	client = rtsp_porting_create_client(nPos,&video_addr, &audio_addr);

	if(client == NULL) {
		PRINTF("create client is ERROR\n");
		return 0;
	}

	PRINTF("create client succes ,client=%p\n", client);

	SETRTSPROOMID(0, nPos, roomid);
	
	SETRTSPCLIENT(0, nPos, client);

	SETRTSPUDPREADY(0, nPos, 1);

	//i will save the socket addr int  static struct;
	SETRTSPADDR(0, nPos, client_addr);


	PRINTF("create rtsp client sock success\n");
	return 0;
}


//*******************************************************************************************
/*
                    开启rtsp线程，接收用户
*/
//*******************************************************************************************
//rtsp clinet num just for ts
static int g_ts_client_num = 0;
static int rtsp_ts_set_client_num(int flag)
{
	g_ts_client_num += flag;

	if(g_ts_client_num < 0) {
		g_ts_client_num = 0;
	}

	return 0;
}
int rtsp_ts_get_client_num()
{
	return g_ts_client_num ;
}

//add by zhangmin
#ifdef GDM_RTSP_SERVER
static unsigned __stdcall  RtspVlcContact(void *pParams)
#else
static int  RtspVlcContact(void *pParams)
#endif
{
	char pRecv_temp[TCP_BUFF_LEN * 2] = {0};
	char                            pRecv[TCP_BUFF_LEN * 2] = {0};
	int                              cnt = 0;
	int                             nPos = 0;
	int					tnum = 0;
	int                             			msg_type = 0;
	Thread_Param                   		*rtsp_thread = (Thread_Param *)pParams;
	mid_plat_sockaddr_in              	Listen_addr;
	char                    				local_ip[64] = {0};
	mid_plat_socket                  	sSocket = rtsp_thread->client_socket;
	nPos = rtsp_thread->nPos;
	int                             resvlen = 0;
	int resvlen_temp = 0;
	int 					recv_errno = 0;
	STR_CLIENT_MSG 		strMsg;
	int 					client_flag = VLC_CLIENT;
	int ret = 0;
	long last_time = 0;
	long current_time = 0;

	int timeout = rtsp_get_client_timeout();
	int need_timeout = 0;

	char filename[256] = {0};

	memcpy(&Listen_addr, &rtsp_thread->client_addr, sizeof(mid_plat_sockaddr_in));
	PRINTF("----------have client--------num=%d-----------------------------\n", RtspGetClientNum());
	PRINTF("\n------------- the %d th vlc connecting -----------------\n\n", (nPos + 1));
	memset(pRecv, 0, sizeof(pRecv));
	rtsp_get_local_ip(local_ip, sizeof(local_ip));


	/*set timeout 3s*/
	mid_plat_set_socket_stimeout(sSocket, 3000);
	mid_plat_set_socket_rtimeout(sSocket, 5000);

	memset(&strMsg, 0, sizeof(STR_CLIENT_MSG));
	strMsg.roomid = -1;

	while(1) {
		if(need_timeout == 1) {
			if(last_time == 0) {
				last_time = getruntimets();
			}

			current_time = getruntimets();

			//PRINTF("current_time =%u,last_time=%u\n",current_time,last_time);
			if(current_time - last_time > rtsp_get_client_timeout() * 2000) {
				PRINTF("WARNNING,the %d client is timeout ,the time is %d,i will close it.\n", nPos, current_time - last_time);
				goto ExitClientMsg;
			}
		}

		if(sSocket <= 0) {
			PRINTF("ERROR ,the file = %s,the line =%d\n", __FILE__, __LINE__);
			goto ExitClientMsg;
		}

		if(GETRTSPCLIENTTYPE(0, nPos) == INVALID_CLIENT) {
			PRINTF("invailid client\n");
			tnum++;

			if(tnum > 3) {
				PRINTF("The %d client is invaild\n", nPos);
				goto ExitClientMsg;
			}
		}

		memset(pRecv_temp, 0, sizeof(pRecv_temp));
		resvlen_temp = recv(sSocket, pRecv_temp, TCP_BUFF_LEN, 0);

		if(resvlen_temp == 0) {
			PRINTF("ERROR ,the file = %s,the line =%d\n", __FILE__, __LINE__);
			PRINTF("the client is close the socket \n");
			goto ExitClientMsg;
		}

		if(resvlen_temp < 0) {
#ifdef GDM_RTSP_SERVER
			recv_errno = WSAGetLastError();
#endif

			if(errno == 11 || recv_errno == 10060) {
				continue;
			} else {
				PRINTF("resvlen = %d,errno=%d,=%d\n", resvlen_temp, errno, recv_errno);
				PRINTF("will goto ,the file = %s,the line =%d\n", __FILE__, __LINE__);
				goto ExitClientMsg;
			}
		}

		if(need_timeout == 1) {
			last_time = getruntimets();
		}

		if(strstr(pRecv_temp, "\r\n\r\n") == NULL) {
			//	PRINTF("Warnning,the msg is not complete,the msg =%s=\n",pRecv_temp);
			memset(pRecv, 0, sizeof(pRecv));
			memcpy(pRecv, pRecv_temp, resvlen_temp);
			continue;
		}

		strcat(pRecv, pRecv_temp);
		resvlen = strlen(pRecv);
		PRINTF("resvlen=%d,\n%s\n", resvlen, pRecv);
		msg_type = GetVlcMsgType(pRecv, resvlen, &strMsg);

		if(msg_type) {
			switch(msg_type) {
				case OPTION:
					ResponseOption(sSocket, &strMsg);
					break;

				case DESCRIBE:

					if(rtsp_describe_authentication(pRecv, resvlen, local_ip, &client_flag, &(strMsg.roomid), filename, &need_timeout) == 0) {
						if(client_flag == STB_TS_CLIENT) {
							ret = ResponseDescribeTs(sSocket, Listen_addr, local_ip, 0, &strMsg);
						} else {
							ret = ResponseDescribe(sSocket, Listen_addr, local_ip, 0, &strMsg);
						}
					} else {
						ret = ResponseDescribe(sSocket, Listen_addr, local_ip, -1, &strMsg);
						PRINTF("ERROR,the file = %s,the line =%d\n", __FILE__, __LINE__);
						goto ExitClientMsg;

					}

					if(ret == -1) {
						goto ExitClientMsg;
					}
					PRINTF("rtsp client is %d\n",client_flag);
					//设置客户端
					SETRTSPCLIENTTYPE(0, nPos, client_flag);

					if(client_flag == STB_TS_CLIENT) {
						rtsp_ts_set_client_num(1);
					}

					break;

				case SETUP:
					if(need_timeout == 1) {
						timeout =  rtsp_get_client_timeout();
					} else {
						timeout = 0;
					}

					ResponseSetup(pRecv, resvlen, sSocket, &strMsg, timeout);
					break;

				case PLAY:
					ret = RtspAddClient(Listen_addr, nPos, &strMsg);

					if(ret == -1) {
						goto ExitClientMsg;
					}

					ResponsePlay(sSocket, &strMsg);
					//force iframe
					rtsp_porting_force_Iframe(strMsg.roomid);

					if(0 == g_rtsp_flag) { //when fist vlc connect
						g_rtsp_flag = 1;
					}


					PRINTF("I success create the %d client ,the client type is %d\n", nPos, GETRTSPCLIENTTYPE(0, nPos));
					break;

				case ANNOUNCE:
					//SendAnnounce(sSocket,Listen_addr,local_ip,&strMsg);
					break;

				case PAUSE:
					RtspPauseClient(nPos);
					ResponsePause(sSocket, &strMsg);
					//					PRINTF("the service is not support PAUSE\n");
					break;

				case TEARDOWN:
					ResponseTeardown(sSocket, &strMsg);
					goto ExitClientMsg;

				case SET_PARAMETER:
					ResponseSet_parameter(sSocket, &strMsg);
					break;

				case GET_PARAMETER:
					ResponseGet_parameter(sSocket, &strMsg);
					break;

				default:
					PRINTF("ERROR ,goto exitclientmsg.the file = %s,the line =%d\n", __FILE__, __LINE__);
					goto ExitClientMsg;
					//break;
			}
		}

		memset(pRecv, 0, sizeof(pRecv));
	}

ExitClientMsg:
	PRINTF("Exit RtspCastComMsg nPos = %d sSocket = %d\n", nPos, sSocket);

#ifdef GDM_RTSP_SERVER
	app_rtsp_authentication_stop(filename);
#endif

	RtspDelClient(nPos);

	if(client_flag == STB_TS_CLIENT) {
		rtsp_ts_set_client_num(-1);;
	}

	//if there is no vlc to connect ,then stop rtp package
	cnt = RtspGetClientNum();

	if(cnt) {
		g_rtsp_flag = 1;
	} else {
		g_rtsp_flag = 0;
		//PRINTF("rtsp client byebye!\n");
	}

	PRINTF(" Exit rtsp %d client Pthread!!client num =%d!!ts client num = %d\n", nPos, cnt, g_ts_client_num);
	rtsp_threadid[nPos] = 0;
	close(sSocket);
	sSocket = -1;
	pthread_detach(pthread_self());
	pthread_exit(NULL);
	
	return 0;

}



//---------------------------------------RTSP TASK--------------------------------------------------
//add by zhangmin
#ifdef GDM_RTSP_SERVER
static unsigned __stdcall  RtspTask(void *pParams)
#else
static int  RtspTask(void *pParams)
#endif

{
	mid_plat_sockaddr_in              SrvAddr, ClientAddr;
	int                             nLen;
	int                             RtspCliSock = 0;
	int                             RtspSerSock = 0;
	Thread_Param                    rtspthread0;
	Thread_Param                    *rtspthread = &rtspthread0;
	int                             nPos = 0;
	int                             opt = 1;
	int ret = 0;
	unsigned int activetime = 7200;
	//	PRINTF("Rtsp Task........GetPid():%d\n",getpid());
	RtspThreadInit();
RTSPSTARTRUN:
	PRINTF("RtspTask start...\n");
	//bzero( &SrvAddr, sizeof(mid_plat_sockaddr_in) );
	memset(&SrvAddr, 0 , sizeof(mid_plat_sockaddr_in));
	SrvAddr.sin_family = AF_INET;
	SrvAddr.sin_port = htons(RtspGetPort());
	SrvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

#ifdef GDM_RTSP_SERVER
	mid_plat_pthread_t threadHandle;
#endif

	if(RtspSerSock > 0) {
		mid_plat_close_socket(RtspSerSock);
	}

	RtspSerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(RtspSerSock < 0) {
		PRINTF("ListenTask create error:%d,error msg: = %s\n", errno, strerror(errno));
		return -1;
	}

	setsockopt(RtspSerSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

	if(bind(RtspSerSock, (struct sockaddr *)&SrvAddr, sizeof(SrvAddr)) < 0) {
		PRINTF("in RtspTask:bind error:%d,error msg: = %s\n", errno, strerror(errno));
		return -1;
	}

	if(listen(RtspSerSock, 2) < 0) {
		PRINTF("listen error:%d,error msg: = %s", errno, strerror(errno));
		return -1;
	}

	while(1) {
#ifndef GDM_RTSP_SERVER
	if(rtsp_porting_server_need_stop() == 1)
	{
			usleep(400000);
			continue;
		}
#endif


		memset(&ClientAddr, 0, sizeof(mid_plat_sockaddr_in));
		nLen = sizeof(mid_plat_sockaddr_in);
		RtspCliSock = accept(RtspSerSock, (struct sockaddr *)&ClientAddr, (socklen_t *) &nLen);

		if(rtsp_porting_server_need_stop() == 1)
		{
			PRINTF("ERROR,rtsp need stop\n");
			mid_plat_close_socket(RtspCliSock);
			mid_plat_sleep(500);
			continue;			
		}
		
		if(RtspCliSock > 0   ) {
			nPos = RtspGetNullClientIndex();
			PRINTF("new rtsp client number: %d \n", nPos);


			if(-1 == nPos ) {
				fprintf(stderr, "ERROR: max client error  nPos=%d\n",nPos);
				mid_plat_close_socket(RtspCliSock);
				mid_plat_sleep(500);
				continue;
			} else {
				int nSize = 0;
				int result;
				nSize = 1;

				activetime = rtsp_get_keepactive_time();
				/*modify by zm  2012.04.26  */
				mid_socket_set_active(RtspCliSock, activetime, 3, 30);


				if((setsockopt(RtspCliSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&nSize, sizeof(nSize))) == -1) {
					perror("setsockopt failed");
				}

				nSize = 0;
				nLen = sizeof(nLen);
				result = getsockopt(RtspCliSock, SOL_SOCKET, SO_SNDBUF, (char *)&nSize , (socklen_t *)&nLen);

				if(result) {
					fprintf(stderr, "getsockopt() errno:%d socket:%d  result:%d\n", errno, RtspCliSock, result);
				}

				nSize = 1;

				if(setsockopt(RtspCliSock, IPPROTO_TCP, TCP_NODELAY, (const char *)&nSize , sizeof(nSize))) {
					fprintf(stderr, "Setsockopt error%d\n", errno);
				}

				rtspthread->nPos = nPos;
				rtspthread->client_socket = RtspCliSock;
				memcpy(&rtspthread->client_addr, &ClientAddr, sizeof(mid_plat_sockaddr_in));

				//add by zhangmin
#ifdef GDM_RTSP_SERVER
				threadHandle =  _beginthreadex(NULL, 0, RtspVlcContact, (void *)rtspthread, 0, &rtsp_threadid[nPos]);
				PRINTF("threadHandle == %d\n", threadHandle);

				//threadHandle will return 0,if it is failed
				if(threadHandle == 0) {
					RtspDelClient(nPos);
					mid_plat_close_socket(RtspCliSock);
					fprintf(stderr, "creat pthread ClientMsg error  = %d!\n" , errno);
					continue;
				}
			}

#else
				ret = pthread_create(&rtsp_threadid[nPos], NULL, (void *)RtspVlcContact, (void *)rtspthread);
				mid_plat_sleep(500);
				if(ret) {
					RtspDelClient(nPos);
					mid_plat_close_socket(RtspCliSock);
					fprintf(stderr, "creat pthread ClientMsg error  = %d!\n" , errno);
					continue;
				}
			}

#endif
		}
		else {
			//	if(errno == ECONNABORTED||errno == EAGAIN)  //软件原因中断
			if(errno == EAGAIN) {
				PRINTF("rtsp errno =%d program again start!!!\n", errno);
				mid_plat_sleep(100);
				continue;
			}

			if(RtspSerSock > 0) {
				mid_plat_close_socket(RtspSerSock);
			}

			goto RTSPSTARTRUN;
		}
	}
	RtspExit();
	PRINTF("close the gServSock server\n");

	if(RtspSerSock > 0)
	{
		PRINTF("close gserv socket \n");
		mid_plat_close_socket(RtspSerSock);
	}

	RtspSerSock = 0;
	return 0;
}

static int g_init_flag = 0;
//add by zhangmin
void rtsp_module_init(void)
{
	if(g_init_flag != 0) {
		return ;
	}

	PRINTF("now i init the rtsp module,build by zhangmin\n");
	g_init_flag = 1;
	//	app_rtsp_authentication_clear();
	//rtsp_stream_init(1,0);
	//	app_center_init("COM1");
	//	 _beginthreadex(NULL, 0, RtspTask, NULL, 0, NULL);
	pthread_t threadid = 0;
	pthread_create(&threadid, NULL, (void *)RtspTask, NULL);
	return ;
}




// SDP改变，需要关闭QT
void rtsp_change_sdp_info(int roomid)
{
	rtsp_change_video_resolve(roomid);
	return ;
}

void rtsp_close_all_client(void)
{
	int cli  = 0;
	char *client = NULL;
	for(cli = 0; cli < MAX_RTSP_CLIENT_NUM; cli++) {
		SETRTSPCLIENTTYPE(0, cli, INVALID_CLIENT);

		client = GETRTSPCLIENT(0, cli);
		rtsp_porting_close_client(cli,client);
		client = NULL;
		SETRTSPCLIENT(0, cli, client);
	}

	return ;
}



//is have client
int RtspClientIsNull(void)
{
	return g_rtsp_flag ;
}



//#define DEFAULT_CLINET_TIMEOUT 60
static int rtsp_get_client_timeout()
{
	return rtsp_get_keepactive_time();
}

int rtsp_get_sdp_describe(char *pSend,int len)
{
	int                     	iSendDataLen = 0, iSendDataLen2 = 0, i = 0;
	int                     	usStringLen = 0;
	int                     	listen_port;
	int                     	audio_sample = 48000;
	char                    	pSend2[1000];
	char                    	local_ip[16] = {0};
	char                    	client_ip[16];

	int roomid = 0;
	int 				sdpinfo_len = 0;
	char 			sdpinfo[1024] = {0};
	int				ret = 0;
	char				audio_config[16] = {0};
	int 				a_ret = 0;
	
	char localip[64]= {0};
	
	rtsp_get_local_ip(localip,sizeof(localip));
	
	
	PRINTF("\n");
	a_ret = rtsp_stream_get_audio_sinfo(0, audio_config, &audio_sample);

	if(a_ret < 0) {
		PRINTF("Error\n");
	}

	int mult = 0;
	char ip[32] = {0};
	int vport = 0;
	int aport = 0;
	rtsp_porting_get_ginfo(&mult, ip, &vport, &aport);

	if(mult == 0 || strlen(ip) == 0) {
		strcpy(ip, localip);
	}

	sdpinfo_len =  rtsp_stream_get_video_sdp_info(sdpinfo, roomid);

	if(  sdpinfo_len == -1) {
	//	PRINTF("authencation=%d,sdpinfo_len=%d\n", authencation, sdpinfo_len);
		sprintf(pSend, "%s","RTSP/1.0   404   Stream Not Found\r\n \r\n");
		iSendDataLen = strlen(pSend);
	//	send(socket, pSend, iSendDataLen, 0);
		PRINTF("\n%s\n", pSend);
		return -1;
	}

	sprintf(local_ip, "%s", localip);
	listen_port = RTSP_LISTEN_PORT;
//	sprintf(pSend2, "c=IN IP4 %s\r\n", ip); //inet_ntoa(Listen_addr.sin_addr));
//	iSendDataLen2 = strlen(pSend2);

	sprintf(&pSend2[iSendDataLen2], "m=video %u RTP/AVP 96\r\na=rtpmap:96 H264/90000\r\n", vport);
	iSendDataLen2 = strlen(pSend2);

	if(sdpinfo_len != -1) {
		sprintf(&pSend2[iSendDataLen2], "a=fmtp:96 packetization-mode=1;profile-level-id=428028;sprop-parameter-sets=%s\r\n", sdpinfo);
		iSendDataLen2 = strlen(pSend2);
	}

	sprintf(&pSend2[iSendDataLen2], "c=IN IP4 %s\r\n", ip); //inet_ntoa(Listen_addr.sin_addr));
	iSendDataLen2 = strlen(pSend2);
//	sprintf(&pSend2[iSendDataLen2], "a=control:rtsp://%s:%d/trackID=%d\r\n", local_ip, listen_port, VIDEO_TRACKID);
//	iSendDataLen2 = strlen(pSend2);

	//only the card0 have audio
	{
		sprintf(&pSend2[iSendDataLen2], "m=audio %u RTP/AVP 97\r\na=rtpmap:97 mpeg4-generic/%d/2\r\n", aport, audio_sample);
		iSendDataLen2 = strlen(pSend2);

		//ret = rtsp_stream_get_audio_sdp_info(audio_config);
#if 1

		if(a_ret < 0) {
			PRINTF("ERROR,get audio sdp failed .\n");
		} else {
			sprintf(&pSend2[iSendDataLen2], "a=fmtp:97 streamtype=5; mode=AAC-hbr; config=%s; SizeLength=13; IndexLength=3; IndexDeltaLength=3\r\n", audio_config, local_ip, listen_port, AUDIO_TRACKID);
			iSendDataLen2 = strlen(pSend2);
			sprintf(&pSend2[iSendDataLen2], "c=IN IP4 %s\r\n", ip); //inet_ntoa(Listen_addr.sin_addr));
			iSendDataLen2 = strlen(pSend2);
		}

#endif
	}
	snprintf(pSend,len,"%s",pSend2);
	//send(socket, pSend2, iSendDataLen2, 0);
	PRINTF("\n%s\n", pSend2);
	return 0;

}



