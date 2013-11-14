#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/videodev2.h>

#include "tcpcom.h"
#include "common.h"
#include "demo.h"

#include "log_common.h"

//#include "app_quality.h"

typedef enum {
    LOCK_OUT = 0,
    LOCK_IN
} CurrentResolutionStatus;

/*音/视频参数表结构体*/
extern SavePTable 	gSysParaT;
extern DSPCliParam gDSPCliPara[PORT_NUM];
extern ResizeParam gLowRate;

extern CurrentResolutionStatus lock_status;

/*ICMP 报文发送socket*/
static int 	g_sockfd = 0;
/*max packet*/
static 	u_char outpack[MAXPACKET];
char 	rcvd_tbl[MAX_DUP_CHK / 8];
static 	sem_t	s_sem_con_lock;
static pthread_t client_threadid[MAX_CLIENT] = {0};
/*send data count*/
static unsigned int gnSentCount = 0;
/*Low Rate send data count*/
static unsigned int gnLowCount = 0;
/*send audio count*/
static unsigned int gnAudioCount = 0;
/*send data packet*/
unsigned char gszSendBuf[MAX_PACKET];
/*send data packet*/
unsigned char gszLowBuf[MAX_PACKET];
/*send audio data max packets*/
unsigned char gszAudioBuf[MAX_PACKET];
/*clents's ip */
char IPdotdec[6][20];
extern int app_rtsp_get_active();
extern int AppSetQualityResize(unsigned char data[], int len);
extern int LockCurrentResolution(unsigned char *data, int len);
/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
static int in_cksum(addr, len)
u_short *addr;
int len;
{
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while(nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if(nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}


/*建立ICMP 报文socket*/
void InitICMP(void)
{
	struct protoent *proto;
	int hold = 0;

	if(!(proto = getprotobyname("icmp"))) {
		DEBUG(DL_DEBUG, "unknown protocol icmp.\n");
		return ;
	}

	if((g_sockfd = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		perror("ping: socket");
		return ;
	}

	hold = 48 * 1024;
	setsockopt(g_sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&hold, sizeof(hold));
	return ;
}

/*close*/
int cleanICMP(void)
{
	close(g_sockfd);
	return 0;
}

/*发送ICMP 报文主要是解决网口插拔之后网络不通的bug*/
void SendICMPmessage(void)
{
	register struct icmp *icp;
	int ntransmitted = 0 , ident;
	int mx_dup_ck = MAX_DUP_CHK;
	int datalen = DEFDATALEN;
	register int cc;
	char ipaddr[20] = "192.168.22.22";
	struct sockaddr_in to;

	bzero((char *)&to, sizeof(struct sockaddr));
	to.sin_family = AF_INET;
	to.sin_addr.s_addr = inet_addr(ipaddr);
	ident = getpid() & 0xFFFF;

	icp = (struct icmp *)outpack;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = ntransmitted++;
	icp->icmp_id = ident;				/* ID */
	CLR(icp->icmp_seq % mx_dup_ck);
	gettimeofday((struct timeval *)&outpack[8], (struct timezone *)NULL);

	cc = datalen + 8;			/* skips ICMP portion */

	/* compute ICMP checksum here */
	icp->icmp_cksum = in_cksum((u_short *)icp, cc);
	sendto(g_sockfd, (char *)outpack, cc, 0, (struct sockaddr *)&to, sizeof(struct sockaddr_in));
	DEBUG(DL_DEBUG, "ICMP message!!!!!\n");
	return ;
}


/*设置网络ip地址*/
void SetEthConfigIP(unsigned int ipaddr, unsigned netmask)
{
	int ret;
	struct in_addr addr1, addr2;
	char temp[200];

	memset(temp, 0, sizeof(temp));
	memcpy(&addr1, &ipaddr, 4);
	memcpy(&addr2, &netmask, 4);
	strcpy(temp, "ifconfig eth0 ");
	strcat(temp, inet_ntoa(addr1));
	strcat(temp, " netmask ");
	strcat(temp, inet_ntoa(addr2));
	DEBUG(DL_DEBUG, "command:%s\n", temp);

	ret = system(temp);

	if(ret < 0) {
		DEBUG(DL_ERROR, "ifconfig %s up ERROR\n", inet_ntoa(addr1));
	}
}

/*设置网关*/
void SetEthConfigGW(unsigned int gw)
{
	char temp[200];
	struct in_addr addr1;
	int ret;

	memset(temp, 0, sizeof(temp));
	memcpy(&addr1, &gw, 4);
	strcpy(temp, "route add default gw ");
	strcat(temp, inet_ntoa(addr1));
	DEBUG(DL_DEBUG, "command:%s\n", temp);

	ret = system(temp);

	if(ret < 0) {
		DEBUG(DL_ERROR, "SetEthConfigGW() command:%s ERROR\n", temp);
	}

}


/*设置MAC地址*/
int SetMacAddr(int fd)
{
	char mac_addr[20];

	GetMACAddr(fd, (unsigned char *)mac_addr);
	DEBUG(DL_DEBUG, "MAC addr = %s\n", mac_addr);
	SplitMacAddr(mac_addr, gSysParaT.sysPara.szMacAddr, 6);
	PRINTF("MAC addr = %s\n",gSysParaT.sysPara.szMacAddr);
	return 0;
}


/*消息头打包*/
void PackHeaderMSG(BYTE *data,
                   BYTE type, WORD len)
{
	MSGHEAD  *p;
	p = (MSGHEAD *)data;
	memset(p, 0, HEAD_LEN);
	p->nLen = htons(len);
	p->nMsg = type;
	return ;
}

/*Initial client infomation*/
void InitClientData(unsigned char port_num)
{
	int cli;

	for(cli = 0; cli < MAX_CLIENT; cli++) 	{
		SETCLIUSED(port_num, cli, FALSE);
		SETSOCK(port_num, cli, INVALID_SOCKET);
		SETCLILOGIN(port_num, cli, FALSE);
		SETLOGINTYPE(port_num, cli, LOGIN_USER);
		SETLOWRATEFLAG(port_num, cli, STOP);
	}

	return;
}


void printf_tcp_socket_status(const char *func, int line)
{
	int cli ;
	int dsp = 0;

	for(cli = 0; cli < MAX_CLIENT; cli++) {
		PRINTF("[%s %d],socket = %d,,blogin=%d,bused=%d,nlogintype=%d\n", func, line, gDSPCliPara[dsp].cliDATA[cli].sSocket,
		       gDSPCliPara[dsp].cliDATA[cli].bLogIn,
		       gDSPCliPara[dsp].cliDATA[cli].bUsed,
		       gDSPCliPara[dsp].cliDATA[cli].nLogInType);
	}

}

void ClearLostClient(unsigned char dsp)
{
	int cli ;
	unsigned long currenttime = 0;
	currenttime = get_run_time();

	for(cli = 0; cli < MAX_CLIENT; cli++) {
		if(!ISUSED(dsp, cli) && ISSOCK(dsp, cli)) {
			PRINTF("[ClearLostClient] Socket = %d \n", GETSOCK(dsp, cli));
			shutdown(GETSOCK(dsp, cli), 2);
			close(GETSOCK(dsp, cli));
			SETSOCK(dsp, cli, INVALID_SOCKET);
			SETCLILOGIN(dsp, cli, FALSE) ;
		}

		if(ISUSED(dsp, cli) && (!ISLOGIN(dsp, cli))) {
			if((currenttime - GETCONNECTTIME(dsp, cli)) > 300) {
				PRINTF("\n\n\n\n ClearLostClient Socket = %d,client %d \n\n\n\n", GETSOCK(dsp, cli), cli);
				shutdown(GETSOCK(dsp, cli), 2);
				close(GETSOCK(dsp, cli));
				SETCLIUSED(dsp, cli, FALSE);
				SETSOCK(dsp, cli, INVALID_SOCKET);
				SETCLILOGIN(dsp, cli, FALSE) ;
			}
		}

	}
}

/*get null client index*/
int GetNullClientData(unsigned char port_num)
{
	int cli ;

	for(cli = 0; cli < MAX_CLIENT; cli++) {
		if((!ISUSED(port_num, cli)) && (!ISLOGIN(port_num, cli))) {
			DEBUG(DL_DEBUG, "[add Client] number = %d \n", cli);
			return cli;
		}
	}

	return -1;
}

/*broadcast message*/
void SendBroadCastMsg(int index, BYTE cmd, unsigned char *data, int len)
{
	unsigned char temp[200];
	int length = 0, ret = 0, cnt = 0;

	length = HEAD_LEN + len;
	PackHeaderMSG(temp, cmd, length);
	memcpy(temp + HEAD_LEN, data, len);
	DEBUG(DL_ERROR, "<><><><><><>SendBroadCastMsg()\n");

	for(cnt = 0; cnt < MAX_CLIENT; cnt++) {
		if(ISUSED(PORT_ONE, cnt) && ISLOGIN(PORT_ONE, cnt))	{
			ret = WriteData(GETSOCK(PORT_ONE, cnt), temp, length);

			if(ret < 0) {
				DEBUG(DL_ERROR, "SendBroadCastMsg()return:%d socket:%d ERROR:%d\n", ret, GETSOCK(PORT_ONE, cnt), errno);
				SETCLIUSED(PORT_ONE, cnt, FALSE);
				SETCLILOGIN(PORT_ONE, cnt, FALSE);
			}
		}
	}

	DEBUG(DL_DEBUG, "SendBroadCastMsg() succeed:%d framerate:%d\n", ret, gSysParaT.videoPara[PORT_ONE].nFrameRate);
}

/*broadcast message*/
void LowRateBroadCastMsg(int index, BYTE cmd, unsigned char *data, int len)
{
	unsigned char temp[200];
	int length = 0, ret = 0, cnt = 0;

	length = HEAD_LEN + len;
	PackHeaderMSG(temp, cmd, length);
	memcpy(temp + HEAD_LEN, data, len);

	for(cnt = 0; cnt < MAX_CLIENT; cnt++) {
		if(ISUSED(PORT_ONE, cnt) && ISLOGIN(PORT_ONE, cnt) && GETLOWRATEFLAG(PORT_ONE, cnt))	{
			ret = WriteData(GETSOCK(PORT_ONE, cnt), temp, length);

			if(ret < 0) {
				DEBUG(DL_ERROR, "SendBroadCastMsg()return:%d socket:%d ERROR:%d\n", ret, GETSOCK(PORT_ONE, cnt), errno);
				SETCLIUSED(PORT_ONE, cnt, FALSE);
				SETCLILOGIN(PORT_ONE, cnt, FALSE);
			}
		}
	}

	DEBUG(DL_DEBUG, "SendBroadCastMsg() succeed:%d framerate:%d\n", ret, gSysParaT.videoPara[PORT_ONE].nFrameRate);
}



/*set send timeout*/
int SetSendTimeOut(SOCKET sSocket, unsigned long time)
{
	struct timeval timeout ;
	int ret = 0;

	timeout.tv_sec = time ; //3
	timeout.tv_usec = 0;

	ret = setsockopt(sSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	if(ret == -1) {
		DEBUG(DL_ERROR, "setsockopt() Set Send Time Failed\n");
	}

	return ret;
}
void IISCloseLowRate()
{
	int cnt = 0;

	for(cnt = 0; cnt < MAX_CLIENT; cnt++) {
		if(GETLOWRATEFLAG(PORT_ONE, cnt)) {
			break;
		}
	}

	if(cnt == MAX_CLIENT) {
		SETLOWRATESTART(STOP);
	} else {
		SETLOWRATESTART(START);
	}
}

int gWriteFlag = 0;

int mid_recv(SOCKET s, unsigned char *buff, int len, int flags)
{
	int toplen = 0;
	int readlen = 0;

	while(len - toplen > 0) {
		readlen =  recv(s, buff + toplen, len - toplen, flags);

		if(readlen <= 0) {
			PRINTF("ERROR\n");
			return -1;
		}

		if(readlen != len - toplen) {
			PRINTF("WARNNING,i read the buff len = %d,i need len = %d\n", readlen, len);
		}

		toplen += readlen;
	}

	return toplen;
}
/*PORT_ONE TCP communication message*/
void OneTcpComMsg(void *pParams)
{
	unsigned char szData[256] = {0}, szPass[] = "123";
	int nLen;
	SOCKET sSocket;
	int nPos;
	WORD length;
	MSGHEAD header, *phead;
	int lowstream = 0;
	int *pnPos = (int *)pParams;
	//static char tmp_data[5] = {0};
	DEBUG(DL_DEBUG, "enter OneTcpComMsg() function!!\n");
	nPos = *pnPos;

	free(pParams);  //free memory
	pParams = NULL;
	sSocket = GETSOCK(PORT_ONE, nPos);

	/*set timeout 3s*/
	SetSendTimeOut(sSocket, 3);
	/*set recv time out*/

	bzero(szData, 256);
	phead = &header;
	memset(&header, 0, sizeof(MSGHEAD));
	sem_post(&s_sem_con_lock);
	DEBUG(DL_DEBUG, "OneTcpComMsg()     sem_post!!!!\n");

	while(!gblGetQuit()) {
		memset(szData, 0, sizeof(szData));

		if(sSocket <= 0) {
			goto ExitClientMsg;
		}

		nLen = recv(sSocket, szData, HEAD_LEN, 0);

		DEBUG(DL_DEBUG, "nlen = %d \n", nLen);
		memcpy(phead, szData, HEAD_LEN);

		if(nLen < HEAD_LEN || nLen == -1) {
			DEBUG(DL_ERROR, "nLen< %d !\n", HEAD_LEN);
			goto ExitClientMsg;
		}

		DEBUG(DL_DEBUG, "receive length:%d,HEAD_LEN:%d\n", nLen, HEAD_LEN);
		//message length
		phead->nLen = ntohs(phead->nLen);

		if(phead->nLen - HEAD_LEN > 0) {
			nLen = 0;
			nLen = mid_recv(sSocket, szData + HEAD_LEN, phead->nLen - HEAD_LEN, 0);
			DEBUG(DL_DEBUG, "nMsgLen = %d,len:%d!\n", phead->nLen, nLen);

			if(nLen == -1 || nLen < phead->nLen - HEAD_LEN) {
				DEBUG(DL_ERROR, "nLen < nMsgLen -HEAD_LEN\n");

				goto ExitClientMsg;
			}
		}

		switch(phead->nMsg) {
			case MSG_GET_AUDIOPARAM:
				DEBUG(DL_DEBUG, "PORT_ONE Get AudioParam \n");
				GetAudioParam(sSocket, PORT_ONE, (BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
				break;

			case MSG_GET_VIDEOPARAM:
				DEBUG(DL_DEBUG, "PORT_ONE Get VideoParam \n");
				GetVideoParam(sSocket, PORT_ONE, (BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
				break;

			case MSG_SET_AUDIOPARAM:
				DEBUG(DL_DEBUG, "PORT_ONE Set AudioParam \n");
				SetAudioParam(PORT_ONE, (BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
				break;

			case MSG_SET_VIDEOPARAM:
				DEBUG(DL_DEBUG, "PORT_ONE Set VideoParam \n");
				SetVideoParam(PORT_ONE, (BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
				break;
#if 0

			case MSG_VGA_ADJUST:
				DEBUG(DL_DEBUG, "PORT_ONE adjust vga picture!\n");
				break;
#endif

			case MSG_SET_DEFPARAM:
				DEBUG(DL_DEBUG, "DSP:%d Set Default Param !!!!!!!!\n", PORT_ONE);
				break;

			case MSG_ADD_TEXT:
#ifdef HAVE_QUALITY_MODULE
#endif
				{
					DEBUG(DL_DEBUG, "enter into MSG_ADD_TEXT \n");
					//AddOsdText(PORT_ONE, (BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
					DEBUG(DL_DEBUG, "PORT_ONE Add Text\n");
				}
				break;

			case MSG_FARCTRL:
				DEBUG(DL_DEBUG, "PORT_ONE Far Control\n");
				FarCtrlCamera(PORT_ONE, &szData[HEAD_LEN], phead->nLen - HEAD_LEN);
				break;

			case MSG_SET_SYSTIME: {
				gWriteFlag = 1;
				DEBUG(DL_DEBUG, "PORT_ONE Set system Time\n");
				CTime ctime;
				memcpy(&ctime, szData + HEAD_LEN, phead->nLen - HEAD_LEN);
				setRtcTime(ctime.tm_year, ctime.tm_mon + 1 ,
				           ctime.tm_mday, ctime.tm_hour, ctime.tm_min, ctime.tm_sec);
#ifdef TEST_MODE
				GreenScreenAjust();
				DEBUG(DL_DEBUG, "GreenScreenAjust ++++++++++++++++++++++  test ... \n");
#endif
			}
			break;

			case MSG_PASSWORD: {
				if(!(strcmp("sawyer", (char *)szData + HEAD_LEN))) {
					SETLOGINTYPE(PORT_ONE, nPos, LOGIN_ADMIN);
				} else if(szData[HEAD_LEN] == 'A' && !strcmp((char *)szPass, (char *)(szData + HEAD_LEN + 1))) {
					SETLOGINTYPE(PORT_ONE, nPos, LOGIN_ADMIN);
					DEBUG(DL_DEBUG, "logo Admin!\n");
				} else if(szData[HEAD_LEN] == 'U' && !strcmp((char *)szPass, (char *)(szData + HEAD_LEN + 1))) {

					SETLOGINTYPE(PORT_ONE, nPos, LOGIN_USER);
					DEBUG(DL_DEBUG, "logo User!\n");
				} else {
					PackHeaderMSG((BYTE *)szData, MSG_PASSWORD_ERR, HEAD_LEN);
					send(sSocket, szData, HEAD_LEN, 0);
					DEBUG(DL_DEBUG, "logo error!\n");
					SETLOGINTYPE(PORT_ONE, nPos, LOGIN_ADMIN);
					goto ExitClientMsg;   //
				}

				PackHeaderMSG((BYTE *)szData, MSG_CONNECTSUCC, HEAD_LEN);
				send(sSocket, szData, HEAD_LEN, 0);
				DEBUG(DL_DEBUG, "send MSG_CONNECTSUCC");
				/*length*/
				length = HEAD_LEN + sizeof(gSysParaT.sysPara);
				PackHeaderMSG((BYTE *)szData, MSG_SYSPARAMS, length);

				memcpy(szData + HEAD_LEN, &gSysParaT.sysPara, sizeof(gSysParaT.sysPara));
				send(sSocket, szData, length, 0);
				DEBUG(DL_DEBUG, "user log!  DSP:%d  client:%d \n", PORT_ONE, nPos);
				/*set client login succeed*/
				SETCLIUSED(PORT_ONE, nPos, TRUE);
				SETCLILOGIN(PORT_ONE, nPos, TRUE);
				DEBUG(DL_DEBUG, "DSP:%d	 socket:%d\n", PORT_ONE, GETSOCK(PORT_ONE, nPos));
				DEBUG(DL_DEBUG, "ISUSED=%d, ISLOGIN=%d\n", ISUSED(PORT_ONE, nPos), ISLOGIN(PORT_ONE, nPos));
				break;
			}

			case MSG_SYSPARAMS: {
				length = HEAD_LEN + sizeof(gSysParaT.sysPara);
				PackHeaderMSG((BYTE *)szData, MSG_SYSPARAMS, length);
				memcpy(szData + HEAD_LEN, &gSysParaT.sysPara, sizeof(gSysParaT.sysPara));
				send(sSocket, szData, length, 0);
				DEBUG(DL_DEBUG, "Get Sys Params ..........................\n");
			}
			break;

			case MSG_SETPARAMS:
				DEBUG(DL_DEBUG, "Set Params! gSysParams %d Bytes\n", sizeof(gSysParaT.sysPara));
				{
					unsigned char ParamBuf[200];
					SYSPARAMS *Newp, *Oldp;
					int ret = 0;

					memcpy(ParamBuf, &szData[HEAD_LEN], phead->nLen - HEAD_LEN);
					Oldp = &gSysParaT.sysPara;
					Newp = (SYSPARAMS *)&ParamBuf[0];
					ret = SetSysParams(Oldp, Newp);
				}
				break;

			case MSG_SAVEPARAMS:
				DEBUG(DL_DEBUG, "Save Params!\n");
				SaveParamsToFlash();
				SaveLowRateParamsToFlash();
				break;

			case MSG_QUALITYVALUE:
				DEBUG(DL_DEBUG, "Set quality value !!\n");
#ifdef HAVE_QUALITY_MODULE

				if(0 == lowstream && LOCK_OUT == lock_status) {
					AppSetQualityResize(szData + HEAD_LEN, phead->nLen - HEAD_LEN);
				}

#endif
				break;

			case MSG_RESTARTSYS:
				system("sync");
				sleep(4);
				DEBUG(DL_DEBUG, "Restart sys\n");
				system("reboot -f");
				break;

			case MSG_UpdataFile: {
				int ret = 0 ;
				ret = DoUpdateProgram(sSocket, szData, phead->nLen);

				if(ret < 0) {
					goto ExitClientMsg;
				}
			}
			break;

			case MSG_UPDATEFILE_ROOT:
				DEBUG(DL_DEBUG, "Updata Root file\n");
				break;

			case MSG_REQ_I:
				DEBUG(DL_DEBUG, "Request I Frame!\n");
				ForceIframe(GETLOWRATEFLAG(PORT_ONE, nPos));
				break;

			case MSG_CHANGE_INPUT: {
				inputtype stype;
				int oldindex = gblGetIDX();
				memcpy(&stype, szData + HEAD_LEN, 4);
				DEBUG(DL_DEBUG, "stype = %d \n", (int)stype);

				switch(stype) {
					case VGA: {
						DEBUG(DL_DEBUG, "Change to VGA \n");
						gblSetIDX(ADV7401_INDEX, 1);
					}
					break;

					case YCBCR: {
						DEBUG(DL_DEBUG, "Change to YPBPR \n");
						gblSetIDX(ADV7401_INDEX, 1);
					}
					break;

					case DVI: {
						DEBUG(DL_DEBUG, "Change to DVI \n");
						gblSetIDX(AD9880_INDEX, 1);
					}
					break;

					case SDI: {
						DEBUG(DL_DEBUG, "Change to SDI \n");
						gblSetIDX(SDI_INDEX, 1);
					}
					break;

					case AUTO: {
						DEBUG(DL_DEBUG, "Change to AUTO \n");
						gblSetIDX(AUTO_INDEX, 1);
					}
					break;
				}

				SendBroadCastMsg(PORT_ONE, MSG_SEND_INPUT, (unsigned char *)szData, 4);
				usleep(100000);

				if(oldindex != gblGetIDX()) {
					gblSetQuit();
				}
			}
			break;

			case MSG_SEND_INPUT: {
				DEBUG(DL_DEBUG, "send the input type message \n");
				/*传送输入设备类型*/
				MSGHEAD  msghead;
				int inputtype = 0;
				memset(&msghead, 0x00000000, HEAD_LEN);
				msghead.nLen = htons(HEAD_LEN + 4);
				msghead.nMsg = MSG_SEND_INPUT;
				send(sSocket, &msghead, HEAD_LEN, 0);

				switch(gblGetIDX()) {
					case	AD9880_INDEX:
						inputtype = DVI;
						break;

					case	ADV7401_INDEX:
						inputtype = VGA ;
						break;

					case	SDI_INDEX:
						inputtype = SDI ;
						break;

					case	AUTO_INDEX:
						inputtype = AUTO ;
						break;

					default:
						inputtype = AUTO ;
						break;
				}

				send(sSocket, &inputtype, 4, 0);
				DEBUG(DL_DEBUG, "send the input type message end \n");

			}
			break;

			case MSG_LOW_BITRATE: {
				RequestLowRate(PORT_ONE, (BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN, nPos);
				lowstream = 1;
			}
			break;

			case MSG_MUTE: {
				DEBUG(DL_DEBUG, " MSG_MUTE \n");
				SetMuteParams((BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
			}
			break;

			case MSG_SETVGAADJUST: {
				DEBUG(DL_DEBUG, "MSG_SETVGAADJUST\n");
				RevisePictureFrame((BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
			}
			break;

			case MSG_GSCREEN_CHECK: {
				DEBUG(DL_DEBUG, "MSG_SETVGAADJUST\n");
				GreenScreenAjust();
			}
			break;

			case MSG_CAMERACTRL_PROTOCOL: {
				int ret = 0;
				DEBUG(DL_DEBUG, "MSG_CAMERACTRL_PROTOCOL\n");
				ret = SaveRemoteCtrlIndex(sSocket, szData[HEAD_LEN], phead->nLen - HEAD_LEN);
				DEBUG(DL_DEBUG, "<<<<<<<ret>>>>>> = %d\n", ret);

				if(1 == ret) {
					system("sync");
					sleep(2);
					DEBUG(DL_DEBUG, "system reboot\n");
					system("reboot -f");
				}

				//                    SendBroadCastMsg(PORT_ONE,MSG_CAMERACTRL_PROTOCOL,&index,sizeof(index));
			}
			break;

			case MSG_LOCK_SCREEN:
				DEBUG(DL_DEBUG, "[DSP1TcpComMsg] ============= MSG_LOCK_SCREEN\n");
				LockCurrentResolution((BYTE *)&szData[HEAD_LEN], phead->nLen - HEAD_LEN);
				break;

			default:
				break;
		}

		DEBUG(DL_DEBUG, "Switch End!\n");
	}

ExitClientMsg:
	client_threadid[nPos] = 0;

	if(sSocket == GETSOCK(PORT_ONE, nPos)) {
		PRINTF("Exit Client Msg DSP:%d nPos = %d sSocket = %d\n", PORT_ONE, nPos, sSocket);
		SETCLIUSED(PORT_ONE, nPos, FALSE);
		SETSOCK(PORT_ONE, nPos, INVALID_SOCKET);
		SETLOGINTYPE(PORT_ONE, nPos, LOGIN_USER);
		SETCLILOGIN(PORT_ONE, nPos, FALSE);
		SETLOWRATEFLAG(PORT_ONE, nPos, STOP);
		IISCloseLowRate();
		close(sSocket);
	} else {
		printf_tcp_socket_status(__FUNCTION__, __LINE__);
		PRINTF("Warnning,this client is also cleared socket =%d,pos=%d,s=%d\n", sSocket, nPos, GETSOCK(PORT_ONE, nPos));
	}

	pthread_detach(pthread_self());
	pthread_exit(NULL);
}



/*TCP task mode*/
void TCPServerTask(void *pParam)
{

	PRINTF("get pid= %d\n", getpid());
	SOCKET sClientSocket;
	struct sockaddr_in SrvAddr, ClientAddr;
	pthread_t 		drawtimethread;
	SOCKET			ServSock = 0;
	int nLen;
	char newipconnect[20];
	short sPort = PORT_LISTEN_ONE;
	int clientsocket = 0;
	void *ret = 0;
	int ipos = 0;
	int fileflags;
	int opt = 1;

	while(gblGetPortStart()) {
		sleep(1);
	}

	gblSetStartOSD();

	if(CreateTextOsdBuffer() < 0) {
		DEBUG(DL_ERROR, "CreateTextOsdBuffer failed \n");
		gblSetStopOSD();
	}

	pthread_create(&drawtimethread, NULL, (void *)drawtimebufferThread, (void *)NULL);
	DEBUG(DL_DEBUG, "TCPServerTask()........GetPid():%d\n", getpid());
	ret = sem_init(&s_sem_con_lock, 0, 0);
	InitClientData(PORT_ONE);
DSP1STARTRUN:

	DEBUG(DL_DEBUG, "TCPServerTask Task start...\n");
	bzero(&SrvAddr, sizeof(struct sockaddr_in));
	SrvAddr.sin_family = AF_INET;
	SrvAddr.sin_port = htons(sPort);
	SrvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ServSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(ServSock < 0)  {
		DEBUG(DL_ERROR, "ListenTask create error:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		return;
	}

	setsockopt(ServSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(ServSock, (struct sockaddr *)&SrvAddr, sizeof(SrvAddr)) < 0)  {
		DEBUG(DL_ERROR, "bind terror:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		return;
	}

	if(listen(ServSock, 10) < 0) {
		DEBUG(DL_ERROR, "listen error:%d,error msg: = %s", errno, strerror(errno));
		gblSetQuit();
		return ;
	}

	if((fileflags = fcntl(ServSock, F_GETFL, 0)) == -1) {
		DEBUG(DL_ERROR, "fcntl F_GETFL error:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		return;
	}

	if(fcntl(ServSock, F_SETFL, fileflags | O_NONBLOCK) == -1) {
		DEBUG(DL_ERROR, "fcntl F_SETFL error:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		return;
	}

	DEBUG(DL_DEBUG, "Socket %d is OK!\n", PORT_ONE);
	nLen = sizeof(struct sockaddr_in);

	while(!gblGetQuit())  {
		memset(&ClientAddr, 0, sizeof(struct sockaddr_in));
		nLen = sizeof(struct sockaddr_in);
		sClientSocket = accept(ServSock, (void *)&ClientAddr, (DWORD *)&nLen);

		if(sClientSocket > 0) {
			//打印客户端ip//
			inet_ntop(AF_INET, (void *) & (ClientAddr.sin_addr), newipconnect, 16);
			DEBUG(DL_FLOW, "<><><><><>new client: socket = %d, ip = %s<><><><><>\n", sClientSocket, newipconnect);
			int nPos = 0;
			char chTemp[16], *pchTemp;

			pchTemp = &chTemp[0];
			ClearLostClient(PORT_ONE);
			nPos = GetNullClientData(PORT_ONE);

			if(-1 == nPos) 	{
				char chData[20];
				PackHeaderMSG((BYTE *)chData, MSG_MAXCLIENT_ERR, HEAD_LEN);
				DEBUG(DL_ERROR, "ERROR: max client error\n");
				send(sClientSocket, chData, HEAD_LEN, 0);
				close(sClientSocket);
			} else {
				int nSize = 0;
				int result;
				int *pnPos = malloc(sizeof(int));

				inet_ntop(AF_INET, (void *) & (ClientAddr.sin_addr), IPdotdec[nPos], 16);
				/* set client used */
				SETCLIUSED(PORT_ONE, nPos, TRUE);
				SETSOCK(PORT_ONE, nPos, sClientSocket);
				nSize = 1;

				if((setsockopt(sClientSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nSize,
				               sizeof(nSize))) == -1) {
					perror("setsockopt failed");
				}

				nSize = 0;
				nLen = sizeof(nLen);
				result = getsockopt(sClientSocket, SOL_SOCKET, SO_SNDBUF, &nSize , (DWORD *)&nLen);

				if(result) {
					DEBUG(DL_ERROR, "getsockopt() errno:%d socket:%d  result:%d\n", errno, sClientSocket, result);
				}

				nSize = 1;

				if(setsockopt(sClientSocket, IPPROTO_TCP, TCP_NODELAY, &nSize , sizeof(nSize))) {
					DEBUG(DL_ERROR, "Setsockopt error%d\n", errno);
				}

				memset(chTemp, 0, 16);
				pchTemp = inet_ntoa(ClientAddr.sin_addr);
				DEBUG(DL_DEBUG, "Clent:%s connected,nPos:%d socket:%d!\n", chTemp, nPos, sClientSocket);

				*pnPos = nPos;
				//Create ClientMsg task!
				result = pthread_create(&client_threadid[nPos], NULL, (void *)OneTcpComMsg, (void *)pnPos);
				usleep(1500000);

				if(result) {
					close(sClientSocket);   //
					DEBUG(DL_ERROR, "creat pthread ClientMsg error  = %d!\n" , errno);
					continue;
				}

				sem_wait(&s_sem_con_lock);
				DEBUG(DL_DEBUG, "sem_wait() semphone inval!!!  result = %d\n", result);
			}
		} else {
			if(errno == ECONNABORTED || errno == EAGAIN) { //软件原因中断
				//				DEBUG(DL_ERROR,"errno =%d program again start!!!\n",errno);
				usleep(100000);
				continue;
			}

			if(ServSock > 0)	{
				close(ServSock);
			}

			goto DSP1STARTRUN;
		}

	}

	if(drawtimethread) {
		if(pthread_join(drawtimethread, &ret) == 0) {
			if(ret == THREAD_FAILURE) {
				;
			}
		}

		drawtimethread = 0;
	}

	for(ipos = 0; ipos < MAX_CLIENT; ipos++) {
		if(client_threadid[ipos]) {
			clientsocket = GETSOCK(PORT_ONE, ipos);

			if(clientsocket != INVALID_SOCKET) {
				close(clientsocket);
				SETSOCK(PORT_ONE, ipos, INVALID_SOCKET);
			}

			if(pthread_join(client_threadid[ipos], &ret) == 0) {
				if(ret == THREAD_FAILURE) {
					DEBUG(DL_ERROR, "drawtimethread ret == THREAD_FAILURE \n");
				}
			}
		}
	}

	if(ServSock > 0)	{
		close(ServSock);
	}

	ServSock = 0;
	return;
}

#define USE_FOR_GDM
#ifdef USE_FOR_GDM
static unsigned long getCurrentTime2(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned long ultime;

	gettimeofday(&tv , &tz);
	ultime = tv.tv_sec * 1000 + ((tv.tv_usec) / 1000);
	return (ultime);
}

static unsigned long g_video_time = 0;
static unsigned long g_audio_time = 0;
//static unsigned int g_video_test_num = 0;
//static unsigned int g_audio_test_num = 0;
#endif
/*send encode data to every client*/
void SendDataToClient(int nLen, unsigned char *pData,
                      int nFlag, unsigned char index, int width, int height)
{
	int nRet, nSent, nSendLen, nPacketCount, nMaxDataLen;
	FRAMEHEAD  DataFrame;
	int cnt = 0;
	MSGHEAD	*p;
	SOCKET  sendsocket = 0;

	//	ParsePackageLock();
	bzero(&DataFrame, sizeof(FRAMEHEAD));
	nSent = 0;
	nSendLen = 0;
	nPacketCount = 0;
	nMaxDataLen = MAX_PACKET - sizeof(FRAMEHEAD) - HEAD_LEN;


	DataFrame.ID = 0x56534434;		// "4DSP"
	DataFrame.nFrameLength = nLen;

	DataFrame.nDataCodec = 0x34363248;		//"H264"
	//	DataFrame.nTimeTick = getVideoInterval();

#ifdef USE_FOR_GDM
	unsigned int timeTick = 0;

	if(g_video_time == 0) {
		timeTick = 0 ;
		g_video_time = get_run_time();
	} else {

		timeTick = get_run_time() - g_video_time;

	}

	DataFrame.nTimeTick = timeTick;
#else
	DataFrame.nTimeTick = getVideoInterval();
#endif
#if 1

	//	if(nFlag == 1) {
	//		rtsp_porting_video_filter_sdp_info(pData, width, height, 0);
	//	}

#endif
	DataFrame.nWidth = width;		//video width
	DataFrame.nHight = height;		//video height

	if(DataFrame.nHight == 1088) {
		DataFrame.nHight = 1080;
	}


	if(nFlag == 1)	{	//if I frame
		DataFrame.dwFlags = AVIIF_KEYFRAME;
	} else {
		DataFrame.dwFlags = 0;
	}


	while(nSent < nLen) {
		if(nLen - nSent > nMaxDataLen) {
			nSendLen = nMaxDataLen;

			if(nPacketCount == 0) {
				DataFrame.dwSegment = 2;    //start frame
			} else {
				DataFrame.dwSegment = 0;    //middle frame
			}

			nPacketCount++;
		} else {
			nSendLen = nLen - nSent;

			if(nPacketCount == 0) {
				DataFrame.dwSegment = 3;    //first frame and last frame
			} else {
				DataFrame.dwSegment = 1;    //last frame
			}

			nPacketCount++;
		}

		DataFrame.dwPacketNumber = gnSentCount++;
		memcpy(gszSendBuf + HEAD_LEN, &DataFrame, sizeof(FRAMEHEAD));
		memcpy(gszSendBuf + sizeof(FRAMEHEAD) + HEAD_LEN, pData + nSent, nSendLen);
		p = (MSGHEAD *)gszSendBuf;
		memset(p, 0, HEAD_LEN);
		p->nLen = htons((nSendLen + sizeof(FRAMEHEAD) + HEAD_LEN));
		p->nMsg = MSG_SCREENDATA;


		for(cnt = 0; cnt < MAX_CLIENT; cnt++) {
			if(ISUSED(index, cnt) && ISLOGIN(index, cnt) && (GETLOWRATEFLAG(index, cnt) == STOP)) {
				sendsocket = GETSOCK(index, cnt);

				if(sendsocket > 0)  {
					nRet = WriteData(sendsocket, gszSendBuf, nSendLen + sizeof(FRAMEHEAD) + HEAD_LEN);

					if(nRet < 0) {
						SETCLIUSED(index, cnt, FALSE);
						SETCLILOGIN(index, cnt, FALSE);
						writeWatchDog();
						DEBUG(DL_ERROR, "Error: SOCK = %d count = %d  errno = %d  ret = %d\n", sendsocket, cnt, errno, nRet);
					}
				}
			}
		}

		nSent += nSendLen;
	}

	//	ParsePackageunLock();
}

void write_local_file(char *file, char *data, int data_len)
{
	static FILE *fp_low = NULL;

	if(NULL == fp_low) {
		fp_low = fopen(file, "w+");
	}

	fwrite(data, data_len, 1, fp_low);
}

/*send encode data to every client*/
void SendLowRateToClient(int nLen, unsigned char *pData,
                         int nFlag, unsigned char index)
{

	int nRet, nSent, nSendLen, nPacketCount, nMaxDataLen;
	FRAMEHEAD  DataFrame;
	int cnt = 0;
	MSGHEAD	*p;
	SOCKET  sendsocket = 0;
	ResizeParam *resize = &gLowRate;
	//	ParsePackageLock();
	bzero(&DataFrame, sizeof(FRAMEHEAD));
	nSent = 0;
	nSendLen = 0;
	nPacketCount = 0;
	nMaxDataLen = MAX_PACKET - sizeof(FRAMEHEAD) - HEAD_LEN;

	DataFrame.ID = 0x56534434;		// "4DSP"
	DataFrame.nFrameLength = nLen;

	DataFrame.nDataCodec = 0x34363248;		//"H264"
	//	DataFrame.nTimeTick = getVideoInterval();

	DataFrame.nWidth = resize->dst_w; //352;		//video width
	DataFrame.nHight = resize->dst_h; //288;		//video height

	if(nFlag == 1)	{	//if I frame
		DataFrame.dwFlags = AVIIF_KEYFRAME;
	} else {
		DataFrame.dwFlags = 0;
	}

	while(nSent < nLen) {
		if(nLen - nSent > nMaxDataLen) {
			nSendLen = nMaxDataLen;

			if(nPacketCount == 0) {
				DataFrame.dwSegment = 2;    //start frame
			} else {
				DataFrame.dwSegment = 0;    //middle frame
			}

			nPacketCount++;
		} else {
			nSendLen = nLen - nSent;

			if(nPacketCount == 0) {
				DataFrame.dwSegment = 3;    //first frame and last frame
			} else {
				DataFrame.dwSegment = 1;    //last frame
			}

			nPacketCount++;
		}

		DataFrame.dwPacketNumber = gnLowCount++;
		memcpy(gszLowBuf + HEAD_LEN, &DataFrame, sizeof(FRAMEHEAD));
		memcpy(gszLowBuf + sizeof(FRAMEHEAD) + HEAD_LEN, pData + nSent, nSendLen);
		p = (MSGHEAD *)gszLowBuf;
		memset(p, 0, HEAD_LEN);
		p->nLen = htons((nSendLen + sizeof(FRAMEHEAD) + HEAD_LEN));
		p->nMsg = MSG_LOW_SCREENDATA;
		//p->nMsg=MSG_SCREENDATA;
		//send multi client

		for(cnt = 0; cnt < MAX_CLIENT; cnt++) {
			if(ISUSED(index, cnt) && ISLOGIN(index, cnt) && (START == GETLOWRATEFLAG(index, cnt))) {
				sendsocket = GETSOCK(index, cnt);

				if(sendsocket > 0)  {

					nRet = WriteData(sendsocket, gszLowBuf, nSendLen + sizeof(FRAMEHEAD) + HEAD_LEN);

					if(nRet < 0) {
						SETCLIUSED(index, cnt, FALSE);
						SETCLILOGIN(index, cnt, FALSE);
						writeWatchDog();
						DEBUG(DL_ERROR, "Error: SOCK = %d count = %d  errno = %d  ret = %d\n", sendsocket, cnt, errno, nRet);
					}
				}
			}
		}

		nSent += nSendLen;
	}

	//   ParsePackageunLock();
}


/*set audio header infomation*/
static void SetAudioHeader(FRAMEHEAD *pAudio, AudioParam *pSys)
{
	pAudio->nDataCodec = 0x53544441;					//"ADTS"
	pAudio->nFrameRate = pSys->SampleRate; 			//sample rate 1-----44.1KHz
	pAudio->nWidth = pSys->Channel;					//channel (default: 2)
	pAudio->nHight = pSys->SampleBit;				//sample bit (default: 16)
	pAudio->nColors = pSys->BitRate;					//bitrate  (default:64000)
}

/*send encode audio data to every client*/
void SendAudioToClient(int nLen, unsigned char *pData,
                       int nFlag, unsigned char index, unsigned int samplerate)
{
	int nRet, nSent, nSendLen, nPacketCount, nMaxDataLen;
	FRAMEHEAD  DataFrame;
	int cnt = 0;
	MSGHEAD  *p;

	//	ParsePackageLock();
	bzero(&DataFrame, sizeof(FRAMEHEAD));
	DataFrame.dwPacketNumber = gnAudioCount++;

	nSent = 0;
	nSendLen = 0;
	nPacketCount = 0;
	nMaxDataLen = MAX_PACKET - sizeof(FRAMEHEAD) - HEAD_LEN;

	DataFrame.ID = 0x56534434;		// " 4DSP"
	DataFrame.nFrameLength = nLen;


#ifdef USE_FOR_GDM
	unsigned int timeTick = 0;

	if(g_audio_time == 0) {
		timeTick = 0 ;
		g_audio_time = get_run_time() ;
	} else {
		timeTick = get_run_time() - g_audio_time;
	}

	DataFrame.nTimeTick = timeTick;
#else
	DataFrame.nTimeTick = getAudioInterval();
#endif

	SetAudioHeader(&DataFrame, &gSysParaT.audioPara[PORT_ONE]);
	DataFrame.nWidth = 2;					//sound counts (default:2)
	DataFrame.nHight = samplerate;					//samplebit (default:16)

	if(nFlag == 1) {
		DataFrame.dwFlags = AVIIF_KEYFRAME;
	} else {
		DataFrame.dwFlags = 0;
	}

	while(nSent < nLen) {

		if(nLen - nSent > nMaxDataLen) {
			nSendLen = nMaxDataLen;

			if(nPacketCount == 0) {
				DataFrame.dwSegment = 2;
			} else {
				DataFrame.dwSegment = 0;
			}

			nPacketCount++;
		} else {
			nSendLen = nLen - nSent;

			if(nPacketCount == 0) {
				DataFrame.dwSegment = 3;
			} else {
				DataFrame.dwSegment = 1;
			}

			nPacketCount++;
		}

		memcpy(gszAudioBuf + HEAD_LEN, &DataFrame, sizeof(FRAMEHEAD));
		memcpy(gszAudioBuf + sizeof(FRAMEHEAD) + HEAD_LEN, pData + nSent, nSendLen);
		//PackHeaderMSG(gszAudioBuf,MSG_AUDIODATA,(nSendLen+sizeof(FRAMEHEAD)+HEAD_LEN));
		p = (MSGHEAD *)gszAudioBuf;
		memset(p, 0, HEAD_LEN);
		p->nLen = htons((nSendLen + sizeof(FRAMEHEAD) + HEAD_LEN));
		p->nMsg = MSG_AUDIODATA;



		for(cnt = 0; cnt < MAX_CLIENT; cnt++) {
			if(ISUSED(index, cnt) && ISLOGIN(index, cnt)) {
				nRet = WriteData(GETSOCK(index, cnt), gszAudioBuf, nSendLen + sizeof(FRAMEHEAD) + HEAD_LEN);

				if(nRet < 0) {
					SETCLIUSED(index, cnt, FALSE);
					SETCLILOGIN(index, cnt, FALSE);
					DEBUG(DL_ERROR, "Audio Send error index:%d, cnt:%d %d nSendLen:%d  ,nRet:%d\n", index, cnt, errno, nSendLen, nRet);
				}
			}
		}

		nSent += nSendLen;
	}

	//	ParsePackageunLock();
}



