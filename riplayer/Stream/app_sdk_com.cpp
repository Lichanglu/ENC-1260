
#include  "stream.h"


//#define	MSG_GETOUTPUTVIDEOINFO			0x1117
//#define	MSG_SETOUTPUTVIDEOINFO			0x1118
//#define	MSG_GETAUDIOPARAM 					0x111A
//#define	MSG_SETAUDIOPARAM						0x111E



//#define PORT_SDK			1
//#define SDK_LISTEN_PORT		4000
//#define MSG_LOGIN			0x1157
//#define SDK_USERNAME 	"admin"
//#define SDK_PASSWORD	"admin"

//typedef enum __webparsetype
//{
//	INT_TYPE = 0,
//	STRING_TYPE,
//	STRUCT_TYPE,
//}webparsetype;

//typedef struct __webMsgInfo
//{
//	int 			identifier;  //0xaaaaaaaa 标识一个有效数据
//	int				len;      //总长
//	webparsetype	type;     
//}webMsgInfo; 



//static SOCKET 		 g_sdk_cli_sock = -1;

//static SOCKET getSDKClientSockfd(void)
//{
//	return g_sdk_cli_sock;
//}

static int ReadData(SOCKET s, void *pBuf, int nSize)
{
	int nWriteLen = 0;
	int nRet = 0;

	while(nWriteLen < nSize) {
		nRet = recv(s, (char *)pBuf+nWriteLen, nSize-nWriteLen, 0);
		nWriteLen += nRet;
	}
	
	return nWriteLen;
}



/*
##
## send date to socket runtime
##
##
*/
static int WriteData(SOCKET s, void *pBuf, int nSize)
{
	int nWriteLen = 0;
	int nRet = 0;
	int nCount = 0;


	while(nWriteLen < nSize) {

		nRet = send(s, (char *)pBuf + nWriteLen, nSize - nWriteLen, 0);

		if(nRet < 0) {
			if( (nCount < 10)) {

				Sleep(1);
				nCount++;
				continue;
			}
			return nRet;
		} else if(nRet == 0) {

			continue;
		}

		nWriteLen += nRet;
		nCount = 0;
	}

	return nWriteLen;
}


int sdkappCmdIntParse(SOCKET Sockfd,int cmd,int invalue,int inlen,int *outvalue,int *outlen)
{
	webMsgInfo_1260  msg,outmsg;
	//SOCKET Sockfd  =  0;
	int ret = 0,retVal=0;
	char intemp[100] ; //,outtmep[100];
	int valueLen = 0, cmdLen = 0;
	char retValue[16] = {0};
	int retcmd = 0;

	//Sockfd = getSDKClientSockfd();
	
	msg.identifier = TCP_SDK_IDENTIFIER;
	valueLen = inlen;
	cmdLen	= sizeof(webparsetype);
	msg.type = INT_TYPE;
	msg.len = sizeof(webMsgInfo_1260)+valueLen + cmdLen;
	memcpy(intemp,&msg,sizeof(webMsgInfo_1260));
	memcpy(intemp+sizeof(webMsgInfo_1260),&cmd,cmdLen);
	memcpy(intemp+(sizeof(webMsgInfo_1260) + cmdLen),&invalue,valueLen);
	if(WriteData(Sockfd, (void*)intemp,msg.len) < 1)	{
		//closesocket(Sockfd);
		return CLIENT_ERR_TCPSEND;
	}
	if(ReadData(Sockfd,(void *)&outmsg,sizeof(webMsgInfo_1260)) < sizeof(webMsgInfo_1260))
	{
		//closesocket(Sockfd);
		return CLIENT_ERR_TCPRECV;
	}
	if(ReadData(Sockfd, (void *)retValue,(outmsg.len - sizeof(webMsgInfo_1260))) < 0)
	{
		//closesocket(Sockfd);	
		return CLIENT_ERR_TCPRECV;
	}
	memcpy(&retcmd,&retValue,sizeof(webparsetype));
	memcpy(&retVal,retValue + 4, sizeof(int));
	memcpy(outvalue,retValue + 8,sizeof(int));
	return retVal;
}



static int sdkappCmdStringParse(SOCKET sockfd,int cmd,char *invalue,int inlen,char  *outvalue,int *outlen)
{
	webMsgInfo_1260  msg,outmsg;
	//SOCKET sockfd  =  0;
	int retcmd = 0,retval=0;
	char intemp[1024] = {0};
	char outtmep[1024] = {0};
	int valuelen = 0, cmdlen = 0;
	char retvalue[1024] = {0};

	//sockfd = getSDKClientSockfd();
//	if(sockfd < 0)
//		return -1;
	
	msg.identifier = TCP_SDK_IDENTIFIER;
	valuelen = inlen;
	cmdlen	= sizeof(webparsetype);
	msg.type = STRING_TYPE;
		
	msg.len = sizeof(webMsgInfo_1260)+valuelen + cmdlen;
	memcpy(intemp,&msg,sizeof(webMsgInfo_1260));
	memcpy(intemp+sizeof(webMsgInfo_1260),&cmd,cmdlen);
	if(invalue != NULL)
	{
		memcpy(intemp+(sizeof(webMsgInfo_1260) + cmdlen),invalue,valuelen);
	}
	if(WriteData(sockfd, (void*)intemp,msg.len) < 1)	{
		//closesocket(sockfd);
	
		return CLIENT_ERR_TCPSEND;
	}
	if(ReadData(sockfd,(void *)&outmsg,sizeof(webMsgInfo_1260)) < sizeof(webMsgInfo_1260))
	{
		//closesocket(sockfd);
		
		return CLIENT_ERR_TCPRECV;
	}
	
	if(ReadData(sockfd, (void *)retvalue,(outmsg.len - sizeof(webMsgInfo_1260))) < 0)
	{
		//closesocket(sockfd);	
	
		return CLIENT_ERR_TCPRECV;
	}
	memcpy(&retcmd,retvalue,cmdlen);
	memcpy(&retval,retvalue + 4,sizeof(int));
	memcpy(outvalue,retvalue+cmdlen+sizeof(int),(outmsg.len - sizeof(webMsgInfo_1260) - sizeof(webparsetype)));
	return retval;
	
}


static int sdkappCmdStructParse(SOCKET Sockfd,int cmd,void  *invalue,int inlen,void *outvalue,int *outlen)
{
	webMsgInfo_1260  inmsg,outmsg;
	//SOCKET Sockfd  =  0;
	int retCmd = 0,retval;
	char intemp[1024];//,outtmep[1024];
	int valueLen = 0, cmdLen = 0;
	char retValue[1024] = {0};
	int msgHeadLen = sizeof(webMsgInfo_1260);
	
	//Sockfd = getSDKClientSockfd();

	inmsg.identifier = TCP_SDK_IDENTIFIER;
	valueLen = inlen;
	cmdLen	= sizeof(int);
	inmsg.type = STRUCT_TYPE;
		
	inmsg.len = msgHeadLen+valueLen + cmdLen;
	memcpy(intemp,&inmsg,msgHeadLen);
	memcpy(intemp+msgHeadLen,&cmd,cmdLen);
	if(invalue !=NULL)
	{
		memcpy(intemp+(msgHeadLen + cmdLen),invalue,valueLen);
	}
	if(WriteData(Sockfd, (void*)intemp,inmsg.len) < 1)	{
		//closesocket(Sockfd);
		
		return CLIENT_ERR_TCPSEND;
	}
	if(ReadData(Sockfd,(void *)&outmsg,msgHeadLen) < msgHeadLen)
	{
		//closesocket(Sockfd);
	
		return CLIENT_ERR_TCPRECV;
	}
	
	if(ReadData(Sockfd, (void *)retValue,(outmsg.len - msgHeadLen)) < 0)
	{
		//closesocket(Sockfd);
	
		return CLIENT_ERR_TCPRECV;
	}
	//need some check
	memcpy(&retCmd,retValue,cmdLen);
	memcpy(&retval,retValue+cmdLen,sizeof(int));
	memcpy(outvalue,retValue+cmdLen+sizeof(int),outmsg.len-msgHeadLen-cmdLen-sizeof(int));
	
	return retval;
	
}

int ConnectServer(SOCKET s,struct sockaddr_in addr)
{

 //设置套接字为非阻塞模式
 	unsigned long ul = 1;
  ioctlsocket(s, FIONBIO, (unsigned long*)&ul);
  
 //建立连接（异步模式下直接返回-1）
 if (SOCKET_ERROR == connect(s, (struct sockaddr*)&addr, sizeof(struct sockaddr)))
 {
  //设置超时
  timeval tv;
  fd_set set;
  FD_ZERO(&set);
  FD_SET(s, &set);
  tv.tv_sec = 1;  //3s，默认数值较大
  tv.tv_usec = 0;
  //select函数功能：测试指定的fd可读？可写？有异常条件待处理？
  int ret = select(0, NULL, &set, 0, &tv);
  
  if (ret <= 0)  //发生错误或超时
  {
   closesocket(s);
   //AfxMessageBox("Cannot Connect the server！\n");
   return -1;
  }
 }
 
  ul = 0;  //设置成原来的阻塞模式
 ioctlsocket(s, FIONBIO, (unsigned long*)&ul);
 
  return 0;
  
}

int sdkTcpConnect(char *ip)
{
	SOCKET client_socket;				
	char outdata[1024] = {0};
	int  sdkret = 0;
	int outlen = 0;
	struct sockaddr_in client_addr;
	int ret =0;
	
	WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,1),&wsaData)) //调用Windows Sockets DLL
	    { 
	         printf("Winsock无法初始化!\n");
	         WSACleanup();
	         return NULL;
	    }
	
	client_socket =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(client_socket < 0 ){
		PRINTF("sdkTcpConnect:client_socket error!\n");
		return -1;
	}

	memset(&client_addr ,0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(SDK_LISTEN_PORT);
	client_addr.sin_addr.s_addr = inet_addr(ip);
	u_long mode = 1;
	ioctlsocket(client_socket,FIONBIO,&mode);

	ret = ConnectServer(client_socket,client_addr);
//	connet_num = connect(client_socket , (struct sockaddr *)&client_addr ,sizeof(struct sockaddr)); 
	if( ret < 0 )
	{
		PRINTF("sdkTcpConnect:connect is error!\n");
		closesocket(client_socket);
		WSACleanup();
		client_socket = -1;
		return -1;
	}
//	g_sdk_cli_sock = client_socket;
	
	return client_socket;
}

int sdkSetLogin(SOCKET fd,char *username, char *password)
{
	char inbuf[128] = {0};
	char outbuf[1024] = {0};
	int inlen = strlen(username) + strlen(password) + 1;
	int outlen = 0;
	int ret = 0;

	strcpy(inbuf,username);
	strcat(inbuf,"/");
	strcat(inbuf,password);

	PRINTF("login = %s\n",inbuf);
	ret = sdkappCmdStringParse(fd,MSG_SDK_LOGIN,inbuf,inlen,outbuf, &outlen);

	return ret;
}

int sdkGetVideoparam(SOCKET fd,OutputVideoInfo *outval)
{
	int inlen=sizeof(OutputVideoInfo);
	int outlen = 0,ret = 0;
	ret =sdkappCmdStructParse(fd,MSG_GETOUTPUTVIDEOINFO,NULL,inlen,outval,&outlen);
	return ret;
}

int sdkSetVideoparam(SOCKET fd,OutputVideoInfo *inval,OutputVideoInfo *outval)
{
	int inlen = sizeof(OutputVideoInfo);
 	int outlen = 0;
	int ret;
	
	ret = sdkappCmdStructParse(fd,MSG_SETOUTPUTVIDEOINFO,inval,inlen,outval,&outlen);
	return ret;
}


/*获得音频参数表*/
int sdkGetAudioParam(SOCKET fd,AudioParam *audio)
{
	int inlen=sizeof(AudioParam);
	int outlen = 0,ret = 0;
	ret =sdkappCmdStructParse(fd,MSG_GETAUDIOPARAM,(AudioParam*)NULL,inlen, (AudioParam*)audio,&outlen);
	return ret;
}

/*设置音频参数*/
int sdkSetAudioParam(SOCKET fd,AudioParam *inaudio,AudioParam *outaudio)
{
	int inlen=sizeof(AudioParam);
	int outlen = 0,ret = 0;
	ret =sdkappCmdStructParse(fd,MSG_SETAUDIOPARAM,inaudio,inlen, (void*)outaudio,&outlen);
	return ret;
}



int sdkCloseTCPConnet(SOCKET Sockfd)
{
	closesocket(Sockfd);
	return 0;
}
