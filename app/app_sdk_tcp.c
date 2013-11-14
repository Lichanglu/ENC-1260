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

#include "common.h"
#include "demo.h"

#include "log_common.h"
#include "enc1260weblisten.h"
#include "middle_control.h"
#include "WEB_CGI/cgi-enc/weblib.h"
#include "WEB_CGI/cgi-enc/webTcpCom.h"
#include "app_video_output.h"
#include "./stream_output/stream_output_struct.h"
#include "app_sdk_tcp.h"
#include "tcpcom.h"

#define PORT_SDK			1

#define SDK_LOGIN_USER 	"admin"
#define SDK_LOGIN_PASSWD 	"szreach"
#define MAX_CLIENT_NUM 8


static int g_client_num = 0;
int app_sdk_login_check(char *in,char *out)
{
	char temp[256] = {0};
	snprintf(temp,sizeof(temp),"%s/%s",SDK_LOGIN_USER,SDK_LOGIN_PASSWD);
	if(strcmp(temp,in) == 0)
	{
		PRINTF("login check ok\n");
		return SERVER_RET_OK;
	}
	else
	{
		PRINTF("login check failed\n");
		return SERVER_RET_USER_INVALIED;		
	}
}







static int sdk_add_client_num( )
{
	g_client_num ++;
}
static int sdk_del_client_num()
{
	g_client_num--;
}

static int sdk_get_client_num()
{
	return g_client_num;
}




#if 0
static pthread_t sdk_client_thread_id[MAX_CLIENT] = {0};
static int 		 g_sdk_cli_sock = -1;
extern DSPCliParam gDSPCliPara[PORT_NUM];

int getSDKClientSockfd(void)
{
	return g_sdk_cli_sock;
}

static int ReadData(int s, void *pBuf, int nSize)
{
	int nWriteLen = 0;
	int nRet = 0;

	while(nWriteLen < nSize) {
		nRet = recv(s, (char *)pBuf + nWriteLen, nSize - nWriteLen, 0);
		nWriteLen += nRet;
	}

	return nWriteLen;
}



int sdkappCmdIntParse(int cmd, int invalue, int inlen, int *outvalue, int *outlen)
{
	webMsgInfo_1260  msg, outmsg;
	int Sockfd  =  0, ret = 0, retVal = 0;
	char intemp[100], outtmep[100];
	int valueLen = 0, cmdLen = 0;
	char retValue[16] = {0};
	int retcmd = 0;

	Sockfd = getSDKClientSockfd();

	msg.identifier = TCP_IDENTIFIER;
	valueLen = inlen;
	cmdLen	= sizeof(webparsetype);
	msg.type = INT_TYPE;
	msg.len = sizeof(webMsgInfo_1260) + valueLen + cmdLen;
	memcpy(intemp, &msg, sizeof(webMsgInfo_1260));
	memcpy(intemp + sizeof(webMsgInfo_1260), &cmd, cmdLen);
	memcpy(intemp + (sizeof(webMsgInfo_1260) + cmdLen), &invalue, valueLen);

	if(WriteData(Sockfd, (void *)intemp, msg.len) < 1)	{
		close(Sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPSEND;
	}

	if(ReadData(Sockfd, (void *)&outmsg, sizeof(webMsgInfo_1260)) < sizeof(webMsgInfo_1260)) {
		close(Sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPRECV;
	}

	if(ReadData(Sockfd, (void *)retValue, (outmsg.len - sizeof(webMsgInfo_1260))) < 0) {
		//printf("retValue[0]:%d,retValue[1]:%d\n",retValue[0],retValue[1]);
		close(Sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPRECV;
	}

	memcpy(&retcmd, &retValue, sizeof(webparsetype));
	memcpy(&retVal, retValue + 4, sizeof(int));
	memcpy(outvalue, retValue + 8, sizeof(int));
	return retVal;

}



int sdkappCmdStringParse(int cmd, char *invalue, int inlen, char  *outvalue, int *outlen)
{
	webMsgInfo_1260  msg, outmsg;
	int sockfd  =  0, retcmd = 0, retval = 0;
	char intemp[100] = {0};
	char outtmep[100] = {0};
	int valuelen = 0, cmdlen = 0;
	char retvalue[100] = {0};

	sockfd = getSDKClientSockfd();

	if(sockfd < 0) {
		return -1;
	}

	msg.identifier = TCP_IDENTIFIER;
	valuelen = inlen;
	cmdlen	= sizeof(webparsetype);
	msg.type = STRING_TYPE;

	msg.len = sizeof(webMsgInfo_1260) + valuelen + cmdlen;
	memcpy(intemp, &msg, sizeof(webMsgInfo_1260));
	memcpy(intemp + sizeof(webMsgInfo_1260), &cmd, cmdlen);

	if(invalue != NULL) {
		memcpy(intemp + (sizeof(webMsgInfo_1260) + cmdlen), invalue, valuelen);
	}

	if(WriteData(sockfd, (void *)intemp, msg.len) < 1)	{
		close(sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPSEND;
	}

	if(ReadData(sockfd, (void *)&outmsg, sizeof(webMsgInfo_1260)) < sizeof(webMsgInfo_1260)) {
		close(sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPRECV;
	}

	if(ReadData(sockfd, (void *)retvalue, (outmsg.len - sizeof(webMsgInfo_1260))) < 0) {
		close(sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPRECV;
	}

	memcpy(&retcmd, retvalue, cmdlen);
	memcpy(&retval, retvalue + 4, sizeof(int));
	memcpy(outvalue, retvalue + cmdlen + sizeof(int), (outmsg.len - sizeof(webMsgInfo_1260) - sizeof(webparsetype)));
	return retval;

}




int sdkappCmdStructParse(int cmd, void  *invalue, int inlen, void *outvalue, int *outlen)
{
	webMsgInfo_1260  inmsg, outmsg;
	int Sockfd  =  0, retCmd = 0, retval;
	char intemp[1024], outtmep[1024];
	int valueLen = 0, cmdLen = 0;
	char retValue[1024] = {0};
	int msgHeadLen = sizeof(webMsgInfo_1260);

	Sockfd = getSDKClientSockfd();

	inmsg.identifier = TCP_IDENTIFIER;
	valueLen = inlen;
	cmdLen	= sizeof(int);
	inmsg.type = STRUCT_TYPE;

	inmsg.len = msgHeadLen + valueLen + cmdLen;
	memcpy(intemp, &inmsg, msgHeadLen);
	memcpy(intemp + msgHeadLen, &cmd, cmdLen);

	if(invalue != NULL) {
		memcpy(intemp + (msgHeadLen + cmdLen), invalue, valueLen);
	}

	if(WriteData(Sockfd, (void *)intemp, inmsg.len) < 1)	{
		close(Sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPSEND;
	}

	if(ReadData(Sockfd, (void *)&outmsg, msgHeadLen) < msgHeadLen) {
		close(Sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPRECV;
	}

	if(ReadData(Sockfd, (void *)retValue, (outmsg.len - msgHeadLen)) < 0) {
		close(Sockfd);
		close(g_sdk_cli_sock);
		return ERR_TCPRECV;
	}

	//need some check
	memcpy(&retCmd, retValue, cmdLen);
	memcpy(&retval, retValue + cmdLen, sizeof(int));
	memcpy(outvalue, retValue + cmdLen + sizeof(int), outmsg.len - msgHeadLen - cmdLen - sizeof(int));

	return retval;

}


int SDKTcpConnect(char *ip)
{
	int client_socket = 0;
	char outdata[1024] = {0};
	int  sdkret = 0;
	int outlen = 0;
	struct sockaddr_in client_addr;

	client_socket = socket(AF_INET, SOCK_STREAM , 0);

	bzero(&client_addr , sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(SERVPORT);
	client_addr.sin_addr.s_addr = inet_addr(ip);

	int connet_num = connect(client_socket , (struct sockaddr *)&client_addr , sizeof(struct sockaddr));

	if(connet_num == -1) {
		PRINTF("connect is error!\n");
		close(client_socket);
		client_socket = -1;
		return -1;
	}

	g_sdk_cli_sock = client_socket;

	return client_socket;
}

int sdkSetLogin(char *username, char *password)
{
	char inbuf[128] = {0};
	char outbuf[1024] = {0};
	strcpy(inbuf, username);
	strcat(inbuf, "/");
	strcat(inbuf, password);
	int inlen = strlen(username) + strlen(password) + 1;
	int outlen = 0;
	int ret = 0;
	PRINTF("login = %s\n", inbuf);
	ret = sdkappCmdStringParse(MSG_LOGIN, inbuf, inlen, outbuf, &outlen);

	return ret;
}

int sdkGetVideoparam(OutputVideoInfo *outval)
{
	int inlen = sizeof(OutputVideoInfo);
	int outlen = 0, ret = 0;
	ret = sdkappCmdStructParse(MSG_GETOUTPUTVIDEOINFO, NULL, inlen, outval, &outlen);
	return ret;
}

int sdkSetVideoparam(OutputVideoInfo *inval, OutputVideoInfo *outval)
{
	int inlen = sizeof(OutputVideoInfo);
	int outlen = 0;
	int ret;

	ret = sdkappCmdStructParse(MSG_SETOUTPUTVIDEOINFO, inval, inlen, outval, &outlen);
	return ret;
}


/*获得音频参数表*/
int sdkGetAudioParam(AudioParam *audio)
{
	int inlen = sizeof(AudioParam);
	int outlen = 0, ret = 0;
	ret = sdkappCmdStructParse(MSG_GETAUDIOPARAM, (AudioParam *)NULL, inlen, (AudioParam *)audio, &outlen);
	return ret;
}

/*设置音频参数*/
int sdkSetAudioParam(AudioParam *inaudio, AudioParam *outaudio)
{
	int inlen = sizeof(AudioParam);
	int outlen = 0, ret = 0;
	ret = sdkappCmdStructParse(MSG_SETAUDIOPARAM, inaudio, inlen, (void *)outaudio, &outlen);
	return ret;
}

int divideString(char *srcdata, char *delim, char **dstdata)
{
	int i = 1;
	dstdata[0] = strtok(srcdata, delim);

	while((dstdata[i] = strtok(NULL, delim))) {
		i++;
	}

	PRINTF("dstdata = %s,%s\n", dstdata[0], dstdata[1]);
	return i;
}

int appsdkSetLogin(int sockfd, char *indata, char *outdata, int nPos)
{
	char *dstdata[2];
	int totallen  = 0;
	WEB_ERROR sdk_ret = RET_OK;
	char senddata[1024] = {0};
	divideString(indata, "/", dstdata);
	PRINTF("%s,%s\n", dstdata[0], dstdata[1]);
	strcpy(outdata, indata);
	totallen = MSGINFOHEAD + sizeof(int) + sizeof(int) + strlen(outdata);

	if((strcmp(SDK_USERNAME, dstdata[0]) == 0)
	   && (strcmp(SDK_PASSWORD, dstdata[1]) == 0)) {
		SETLOGINTYPE(PORT_SDK, nPos, LOGIN_ADMIN);
	} else {
		sdk_ret = RET_USER_INVALIED;
		msgPacket(senddata, STRING_TYPE, totallen, MSG_LOGIN, sdk_ret);
		memcpy(senddata + (totallen - strlen(outdata)), outdata, strlen(outdata));
		//PRINTF("the cmd =%04x,the out=%s,the ret=%04x\n", cmd, out, web_ret);
		send(sockfd, senddata, totallen, 0);
		goto EXIT;
	}

	msgPacket(senddata, STRING_TYPE, totallen, MSG_LOGIN, sdk_ret);
	memcpy(senddata + (totallen - strlen(outdata)), outdata, strlen(outdata));
	//PRINTF("the cmd =%04x,the out=%s,the ret=%04x\n", cmd, out, web_ret);
	send(sockfd, senddata, totallen, 0);
	PRINTF("send MSG_CONNECTSUCC\n");

	/*set client login succeed*/
	SETCLIUSED(PORT_SDK, nPos, TRUE);
	SETCLILOGIN(PORT_SDK, nPos, TRUE);
	PRINTF("DSP:%d	 socket:%d\n", PORT_SDK, GETSOCK(PORT_SDK, nPos));
	PRINTF("ISUSED=%d, ISLOGIN=%d\n", ISUSED(PORT_SDK, nPos), ISLOGIN(PORT_SDK, nPos));
	return 0;
EXIT:

	if(sockfd == GETSOCK(PORT_SDK, nPos)) {
		PRINTF("Exit Client Msg DSP:%d nPos = %d sSocket = %d\n", PORT_SDK, nPos, sockfd);
		SETCLIUSED(PORT_SDK, nPos, FALSE);
		SETSOCK(PORT_SDK, nPos, INVALID_SOCKET);
		SETLOGINTYPE(PORT_SDK, nPos, LOGIN_USER);
		SETCLILOGIN(PORT_SDK, nPos, FALSE);
		SETLOWRATEFLAG(PORT_SDK, nPos, STOP);
		close(sockfd);
		return -1;
	}
}



int sdkParseInt(int fd, char *data, int len)
{
	int recvlen;
	int cmd = 0;
	int actdata = 0;
	//char logoname[15] = "logo.png";
	int ret = 0;
	int sdk_ret  = RET_OK;

	int need_send = 0;


	char senddata[1024] = {0};
	int totallen = 0;

	recvlen = recv(fd, data, len, 0);
	int out = 0;

	if(recvlen < 0) {
		sdk_ret = RET_RECV_FAILED;
		need_send = 1;
		goto EXIT;
	}

	if(len != sizeof(cmd) + sizeof(int)) {
		sdk_ret = RET_INVAID_PARM_LEN;
		need_send = 1;
		goto EXIT;
	}


	memcpy(&cmd, data, sizeof(int));
	memcpy(&actdata, data + sizeof(int), len - sizeof(int));

	PRINTF("cmd = %04x\n", cmd);

	switch(cmd) {
		default:
			PRINTF("unkonwn cmd = %04x\n", cmd);
			need_send = 1;
			sdk_ret = RET_UNKOWN_CMD;
			break;
	}

	if(ret < 0) {
		sdk_ret = RET_INVAID_PARM_VALUE;
	}

EXIT:

	if(need_send == 1) {
		totallen = MSGINFOHEAD + sizeof(cmd) + sizeof(sdk_ret) + sizeof(out);
		msgPacket(senddata, INT_TYPE, totallen, cmd, sdk_ret);
		memcpy(senddata + (totallen - sizeof(out)), &out, sizeof(out));
		PRINTF("the cmd =%04x,the value=%d,the ret=%04x\n", cmd, out, sdk_ret);
		send(fd, senddata, totallen, 0);
	}

	return 0;
}


int sdkParseString(int fd, char *data, int len, int npos)
{
	int recvlen;
	int cmd = 0;
	int status = 0;
	char actdata[4096] = {0};
	int vallen = 0;

	char senddata[1024] = {0};
	int totallen = 0;

	char  out[256] = "unknown cmd.";
	int sdk_ret  = RET_OK;
	int need_send = 0;

	recvlen = recv(fd, data, len, 0);

	vallen = len - sizeof(int);

	if(recvlen < 0 || vallen >= sizeof(actdata)) {
		PRINTF("Error.\n");
		sdk_ret = RET_INVAID_PARM_LEN;
		need_send = 1;
		status = -1;
		goto EXIT;
	}


	//actdata = (char *)malloc(vallen);
	memcpy(&cmd, data, sizeof(int));
	memcpy(actdata, data + sizeof(int), vallen);
	PRINTF("cmd = %04x\n", cmd);

	switch(cmd) {
		case MSG_LOGIN: {
			PRINTF("MSG_LOGIN fd =%d, data=%s\n", fd, actdata);
			appsdkSetLogin(fd, actdata, out, npos);
		}
		break;

		default:
			PRINTF("Warnning,the cmd %d is UNKOWN\n", cmd);
			need_send = 1;
			break;
	}

	//	free(actdata);


EXIT:

	if(need_send == 1) {
		totallen = MSGINFOHEAD + sizeof(cmd) + sizeof(sdk_ret) + sizeof(out);
		msgPacket(senddata, INT_TYPE, totallen, cmd, sdk_ret);
		memcpy(senddata + (totallen - sizeof(out)), &out, sizeof(out));
		PRINTF("the cmd =%04x,the value=%d,the ret=%04x\n", cmd, out, sdk_ret);
		send(fd, senddata, totallen, 0);
	}

	return status;
}


int sdkParseStruct(int fd, char *data, int len)
{
	int recvlen;
	int cmd = 0;
	//char *actualdata = NULL;
	char actualdata[1024] = {0};
	char out[2048] = {0};
	int  vallen = 0;
	int  status = 0;
	int  ret = 0;


	char senddata[1024] = {0};
	int totallen = 0;

	int sdk_ret = RET_OK;
	int need_send = 0;


	recvlen = recv(fd, data, len, 0);

	if(recvlen < 0) {
		PRINTF("recv failed,errno = %d,error message:%s \n", errno, strerror(errno));
		//return -1;
		sdk_ret = RET_RECV_FAILED;
		status = -1;
		need_send = 1;
		goto EXIT;
	}

	vallen = len - sizeof(int);
	//actualdata = (char *)malloc(vallen);
	memcpy(&cmd, data, sizeof(int));
	memcpy(actualdata, data + sizeof(int), len - sizeof(int));
	//PRINTF("cmd = 0x%04x,vallen=%d=len=%d\n", cmd, vallen, len);

	switch(cmd) {

		case MSG_SETOUTPUTVIDEOINFO:
			PRINTF("cmd = 0x%04x is MSG_SETOUTPUTVIDEOINFO.\n", cmd);
			setOutputVideoInfo(fd, cmd, actualdata, vallen);
			break;

		case MSG_GETOUTPUTVIDEOINFO:
			PRINTF("cmd = 0x%04x is MSG_GETOUTPUTVIDEOINFO.\n", cmd);
			getOutputVideoInfo(fd, cmd, actualdata, vallen);
			break;

		case MSG_GETAUDIOPARAM:
			PRINTF("cmd = 0x%04x is MSG_GETAUDIOPARAM.\n", cmd);
			GetAudioParam_1260(fd, cmd, actualdata, vallen);
			break;

		case MSG_SETAUDIOPARAM:
			PRINTF("cmd = 0x%04x is MSG_SETAUDIOPARAM.\n", cmd);
			setAudioParam_1260(fd, cmd, actualdata, vallen);
			break;
#if 0

		case MSG_RTSP_GET_GINFO:
		case MSG_RTSP_SET_GINFO:
		case MSG_RTSP_GET_CINFO:
		case MSG_RTSP_SET_CINFO:
		case MSG_RTSP_ADD_SERVER:
		case MSG_MULT_ADD_SERVER:
		case MSG_MULT_GET_CINFO:
		case MSG_MULT_SET_CINFO:
		case MSG_RTSP_SET_STATUS:
		case MSG_MULT_SET_STATUS:
		case MSG_MULT_DEL_SERVER:
			PRINTF("cmd = 0x%04x is MSG_STREAMOUTPUT\n", cmd);
			app_web_stream_output_process_struct(fd, cmd, actualdata, vallen);
			break;
#endif

		default:
			PRINTF("UNKOWN cmd %d\n", cmd);
			break;

	}

	if(ret < 0) {
		sdk_ret = RET_INVAID_PARM_VALUE;
	}

EXIT:

	if(need_send == 1) {
		totallen = MSGINFOHEAD + sizeof(cmd) + sizeof(sdk_ret) + vallen;
		msgPacket(senddata, STRING_TYPE, totallen, cmd, sdk_ret);
		memcpy(senddata + (totallen - vallen), out, vallen);
		PRINTF("the cmd =%04x,,the ret=%04x\n", cmd,  sdk_ret);
		send(fd, senddata, totallen, 0);
	}

	//	free(actualdata);
	return status;
}
#endif

/*WEB server Thead Function*/
void *sdk_clinet_thr(int *arg)
{
	PRINTF("get pid= %d\n", getpid());
	Void                   *status              = THREAD_SUCCESS;
	int nPos;
	int len, client_sock;
	webMsgInfo_1260		webinfo_1260;
	ssize_t			recvlen;
	char  sdk_data[2048] = {0};
	//int *pnPos = (int *)arg;
	int sdk_ret = SERVER_RET_OK;
	int out = 0;
	int totallen = MSGINFOHEAD + sizeof(int) + sizeof(sdk_ret) + sizeof(int);
	char sendBuf[1024] = {0};
	PRINTF("enter sdk_clinet_thr() function!!\n");
	//nPos = *pnPos;

	//free(arg);  //free memory
	//arg = NULL;
	//client_sock = GETSOCK(PORT_SDK, nPos);
	client_sock =(int) (*arg);
	PRINTF("client_sock = %d\n",client_sock);
	
	while(!gblGetQuit()) {
		if(client_sock <= 0) {
			goto ExitClientMsg;
		}

		memset(&webinfo_1260, 0, sizeof(webinfo_1260));
		recvlen = recv(client_sock, &webinfo_1260, sizeof(webinfo_1260), 0);
		PRINTF("recvlen = %d webinfo.len =%d webinfo.type = %d,id=%x\n",
		       recvlen, webinfo_1260.len, webinfo_1260.type, webinfo_1260.identifier);

		if(recvlen < 1)	{
			PRINTF("recv failed,errno = %d,error message:%s,client_sock = %d\n", errno, strerror(errno), client_sock);
			status  = THREAD_FAILURE;
			sdk_ret = SERVER_RET_RECV_FAILED;
			msgPacket(TCP_SDK_IDENTIFIER,sendBuf, INT_TYPE, totallen, 0xffff, sdk_ret);
			memcpy(sendBuf + (totallen - sizeof(out)), &out, sizeof(out));
			PRINTF("up to max client num : %x\n", sdk_ret);
			send(client_sock, sendBuf, totallen, 0);
			
			goto ExitClientMsg;

		}

		if(webinfo_1260.identifier != TCP_SDK_IDENTIFIER) {
			PRINTF("id  error,client_sock = %d\n", client_sock);
			status  = THREAD_FAILURE;
			sdk_ret = SERVER_RET_ID_ERROR;
			msgPacket(TCP_SDK_IDENTIFIER,sendBuf, INT_TYPE, totallen, 0xffff, sdk_ret);
			memcpy(sendBuf + (totallen - sizeof(out)), &out, sizeof(out));
			PRINTF("up to max client num : %x\n", sdk_ret);
			send(client_sock, sendBuf, totallen, 0);
			goto ExitClientMsg;
		}

		len = webinfo_1260.len - sizeof(webinfo_1260);

		PRINTF("web deal begin =%d\n", webinfo_1260.type);

		switch(webinfo_1260.type) {
			case INT_TYPE:
				midParseInt(TCP_SDK_IDENTIFIER,client_sock, sdk_data, len);
			//	sdkParseInt(client_sock, sdk_data, len);
				break;

			case STRING_TYPE:
				midParseString(TCP_SDK_IDENTIFIER,client_sock,sdk_data,len);
				//sdkParseString(client_sock, sdk_data, len, nPos);
				break;

			case STRUCT_TYPE:
				midParseStruct(TCP_SDK_IDENTIFIER,client_sock,sdk_data,len);
				//sdkParseStruct(client_sock, sdk_data, len);
				break;

			default:
				break;
		}

		PRINTF("web deal end =%d\n", webinfo_1260.type);
		//close(client_sock);
	}

	//ProtocolExit();

	PRINTF("Web listen Thread Function Exit!!\n");
ExitClientMsg:
//	sdk_client_thread_id[nPos] = 0;


		//IISCloseLowRate();
	close(client_sock);
	sdk_del_client_num();

	pthread_detach(pthread_self());
	pthread_exit(NULL);
	return status;
}

/*TCP task mode*/
void listenSdkTask(void *pParam)
{
	PRINTF("get pid= %d\n", getpid());
	SOCKET sClientSocket;
	struct sockaddr_in SrvAddr, ClientAddr;
	pthread_t 		drawtimethread;
	SOCKET			ServSock = 0;
	int nLen;
	char newipconnect[20] = {0};
	int clientsocket = 0;
	void *ret = 0;
	int ipos = 0;
	int fileflags;
	int opt = 1;
	pthread_t sdk_client_thread_id[MAX_CLIENT] = {0};
DSP1STARTRUN:

	PRINTF("listenSdkTask Task start...\n");
	bzero(&SrvAddr, sizeof(struct sockaddr_in));
	SrvAddr.sin_family = AF_INET;
	SrvAddr.sin_port = htons(SDK_LISTEN_PORT);
	SrvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	ServSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(ServSock < 0)  {
		PRINTF("ListenTask create error:%d,error msg: = %s\n", errno, strerror(errno));
		return;
	}

	setsockopt(ServSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(ServSock, (struct sockaddr *)&SrvAddr, sizeof(SrvAddr)) < 0)  {
		PRINTF("bind terror:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		return;
	}

	if(listen(ServSock, 10) < 0) {
		PRINTF("listen error:%d,error msg: = %s", errno, strerror(errno));
		gblSetQuit();
		return ;
	}

	if((fileflags = fcntl(ServSock, F_GETFL, 0)) == -1) {
		PRINTF("fcntl F_GETFL error:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		return;
	}

	if(fcntl(ServSock, F_SETFL, fileflags | O_NONBLOCK) == -1) {
		PRINTF("fcntl F_SETFL error:%d,error msg: = %s\n", errno, strerror(errno));
		gblSetQuit();
		return;
	}

	nLen = sizeof(struct sockaddr_in);

	while(!gblGetQuit())  {
		memset(&ClientAddr, 0, sizeof(struct sockaddr_in));
		nLen = sizeof(struct sockaddr_in);
		sClientSocket = accept(ServSock, (void *)&ClientAddr, (DWORD *)&nLen);

		if(sClientSocket > 0) {
			//打印客户端ip//
			inet_ntop(AF_INET, (void *) & (ClientAddr.sin_addr), newipconnect, 16);
			PRINTF("<><><><><>new client: socket = %d, ip = %s<><><><><>\n", sClientSocket, newipconnect);
			int nPos = 0;
			char chTemp[16], *pchTemp;

			pchTemp = &chTemp[0];
		//	ClearLostClient(PORT_SDK);
		//	nPos = GetNullClientData(PORT_SDK);

			if(sdk_get_client_num() >=  MAX_CLIENT_NUM)
			{
				unsigned char chData[512];
				int totallen = 0;
				int cmd = 0;
				int web_ret = SERVER_RET_SOCK_MAX_NUM;
				int out = 0;
				totallen = MSGINFOHEAD + sizeof(cmd) + sizeof(web_ret) + sizeof(out);
				msgPacket(TCP_SDK_IDENTIFIER,chData, INT_TYPE, totallen, cmd, web_ret);
				memcpy(chData + (totallen - sizeof(out)), &out, sizeof(out));
				PRINTF("up to max client num : %x\n", web_ret);
				send(sClientSocket, chData, totallen, 0);
			} else {
				int nSize = 0;
				int result;
				//int *pnPos = malloc(sizeof(int));

				//inet_ntop(AF_INET, (void *) & (ClientAddr.sin_addr), IPdotdec[nPos], 16);
				/* set client used */
				//SETCLIUSED(PORT_SDK, nPos, TRUE);
			//	SETSOCK(PORT_SDK, nPos, sClientSocket);
				nSize = 1;

				if((setsockopt(sClientSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&nSize,
				               sizeof(nSize))) == -1) {
					perror("setsockopt failed");
				}

				nSize = 0;
				nLen = sizeof(nLen);
				result = getsockopt(sClientSocket, SOL_SOCKET, SO_SNDBUF, &nSize , (DWORD *)&nLen);

				if(result) {
					PRINTF("getsockopt() errno:%d socket:%d  result:%d\n", errno, sClientSocket, result);
				}

				nSize = 1;

				if(setsockopt(sClientSocket, IPPROTO_TCP, TCP_NODELAY, &nSize , sizeof(nSize))) {
					PRINTF("Setsockopt error%d\n", errno);
				}

				memset(chTemp, 0, 16);
				pchTemp = inet_ntoa(ClientAddr.sin_addr);
				PRINTF("Clent:%s connected,nPos:%d socket:%d!\n", chTemp, nPos, sClientSocket);

			//	*pnPos = nPos;
				//Create ClientMsg task!
				sdk_add_client_num();
				result = pthread_create(&sdk_client_thread_id[nPos], NULL, (void *)sdk_clinet_thr, &sClientSocket);
				usleep(1500000);

				if(result) {
					close(sClientSocket);   //
					PRINTF("creat pthread ClientMsg error  = %d!\n" , errno);
					continue;
				}

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

	if(ServSock > 0)	{
		close(ServSock);
	}

	ServSock = 0;
	return;
}


