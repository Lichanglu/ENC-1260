#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "weblib.h"
#include "webTcpCom.h"

#define KERNEL_VERSION_LENGTH		16


/*网络发送数据*/
static int WriteData(int s, void *pBuf, int nSize)
{
	int nWriteLen = 0;
	int nRet = 0;
	int nCount = 0;

	while(nWriteLen < nSize) {
		nRet = send(s, (char *)pBuf+nWriteLen, nSize-nWriteLen, 0);
		if(nRet < 0 ) {
			if((errno == ENOBUFS) && (nCount < 10)) {
				fprintf(stderr,"network buffer have been full!\n");
				usleep(10000);
				nCount++;
				continue;
			}
			return nRet;
		}
		else if(nRet == 0) {
			fprintf(stderr,"Send Net Data Error nRet= %d\n",nRet);
			continue;
		}
		nWriteLen += nRet;
		nCount = 0;
	}
	return nWriteLen;
}


/*网络接收数据*/
static int ReadData(int s, void *pBuf, int nSize)
{
	int nWriteLen = 0;
	int nRet = 0;

	while(nWriteLen < nSize) {
		nRet = recv(s, (char *)pBuf+nWriteLen, nSize-nWriteLen, 0);
		nWriteLen += nRet;
	}
	
	return nWriteLen;
}

static int connectToDecodeServer()
{
	int socketFd;
	struct sockaddr_in serv_addr;
	const char* pAddr  = "127.0.0.1";
	socketFd= socket(PF_INET, SOCK_STREAM, 0);
	if(socketFd < 1)
		return -1;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(ENCODESERVER_PORT);
	inet_aton(pAddr,(struct in_addr *)&serv_addr.sin_addr);
	bzero(&(serv_addr.sin_zero),8);
	if (connect(socketFd, (struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1)	{
//		printf("Connet to server failed \n");
		close(socketFd);
		return -1;
	}
	return socketFd;
}

/*0成功 -1文件格式错误,-2升级失败*/
int WebUpdateFile(char* filename)
{
	int inlen = strlen(filename);
	int outlen=0;
	int ret;
	char outval[256] = {0};
	ret = appCmdStringParse(MSG_UPDATESYS, filename,inlen,outval ,&outlen);
	return ret;
}
int WebUploadLogo(char* filename)
{
	int inlen = strlen(filename);
	int outlen=0;
	int ret;
	char outval[256] = {0};
	ret = appCmdStringParse(MSG_UPLOADIMG, filename,inlen,outval ,&outlen);
	return ret;
}
int webGetSoftConfigTime(void)
{
	int inval=0,inlen=sizeof(int);
	int outval=0,outlen=sizeof(int);
	int ret;
	ret = appCmdIntParse(MSG_GETSOFTCONFIGTIME,inval,inlen,&outval,&outlen);
	return outval;
}

int webGetCPULoad(int *outval)
{
	int inval=0,inlen=sizeof(int);
	int outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_GETCPULOAD,inval,inlen,outval,&outlen);
	return ret;
}


int webGetHDCPValue(int *outval)
{
	int inval=0,inlen=sizeof(int);
	int outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_GETHDCPVALUE,inval,inlen,outval,&outlen);
	return ret;
}

int webGetColorSpace(ColorSpace *outval)
{	
	int inval=0,inlen=sizeof(int);
	int outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_GETCOLORSPACE,inval,inlen,outval,&outlen);
	return outval;
}
int webSetColorSpace(ColorSpace inval,ColorSpace *outval)
{
	int inlen=sizeof(int);
	int outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_SETCOLORSPACE,inval,inlen,outval,&outlen);
	return outval;
}

int webgetInputSignalInfo(char *outval)
{
	int inlen=strlen(outval);
	int outlen = 0,ret = 0;
	ret =appCmdStringParse(MSG_GETINPUTSIGNALINFO,NULL,inlen,outval,&outlen);
	return ret;
}
#if 0
int webGreenScreenAjust(ColorSpace *inval)
{
	int inlen=sizeof(int);
	int outval=0,outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_BLUE_REVISE,inval,inlen,&outval,&outlen);
	return ret;
}
#endif
int webRevisePicture(short hporch,short vporch)
{
	unsigned char inval[4] = {0};
	char outval[100] = {0};
	int outlen=0;
	int ret;
	memcpy(inval,&hporch,sizeof(hporch));
	memcpy(inval+sizeof(short),&vporch,sizeof(vporch));
	ret = appCmdStringParse(MSG_REVISE_PICTURE,inval,sizeof(inval),outval,&outlen);
	return ret;
}

int webSignalDetailInfo(char *buf,int inlen)
{
	int outlen,ret=0;
	ret = appCmdStringParse(MSG_SIGNALDETAILINFO,NULL,inlen,buf,&outlen);
	return ret;
}

int webGetSdpInfo(char *buf,int inlen)
{
	int outlen,ret=0;
	ret = appCmdStringParse(MSG_GET_SDP_INFO,NULL,inlen,buf,&outlen);
	return ret;
}


int webGetOutputVideoParam(OutputVideoInfo *outval)
{
	int inlen=sizeof(OutputVideoInfo);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETOUTPUTVIDEOINFO,NULL,inlen,outval,&outlen);
	return ret;
}

int webSetOutputVideoParam(OutputVideoInfo *inval,OutputVideoInfo *outval)
{
	int inlen = sizeof(OutputVideoInfo);
 	int outlen = 0;
	int ret;
	
	ret = appCmdStructParse(MSG_SETOUTPUTVIDEOINFO,inval,inlen,outval,&outlen);
	return ret;
}


int webGetH264encodeParam(H264EncodeParam *outval)
{
	int inlen=sizeof(H264EncodeParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETH264ENCODEPARAM,NULL,inlen,outval,&outlen);
	return ret;
}

int webSetH264encodeParam(H264EncodeParam *inval,H264EncodeParam *outval)
{
	int inlen = sizeof(H264EncodeParam);
 	int outlen = 0;
	int ret;
	
	ret = appCmdStructParse(MSG_SETH264ENCODEPARAM,inval,inlen,outval,&outlen);
	return ret;
}


/*获得视频参数表*/
int WebGetVideoParam(VideoParam *video)
{
	int inlen=sizeof(VideoParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETVIDEOPARAM,NULL,inlen, (void*)video,&outlen);
	return ret;
}

/*设置视频参数*/
int WebSetVideoParam(VideoParam *invideo,VideoParam *outvideo)
{
	int inlen=sizeof(VideoParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_SETVIDEOPARAM,invideo,inlen, (void*)outvideo,&outlen);
	return ret;
}

/*获得音频参数表*/
int WebGetAudioParam(AudioParam *audio)
{
	int inlen=sizeof(AudioParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETAUDIOPARAM,(AudioParam*)NULL,inlen, (AudioParam*)audio,&outlen);
	return ret;
}

/*设置音频参数*/
int WebSetAudioParam(AudioParam *inaudio,AudioParam *outaudio)
{
	int inlen=sizeof(AudioParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_SETAUDIOPARAM,inaudio,inlen, (void*)outaudio,&outlen);
	return ret;
}

/*获得系统参数表*/
int WebGetSysParam(SYSPARAMS *sys)
{
	int inlen=sizeof(SYSPARAMS);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETSYSPARAM,NULL,inlen, (void*)sys,&outlen);
	return ret;
}
/*设置系统参数*/
int WebSetSysParam(SYSPARAMS *insys,SYSPARAMS *outsys)
{
	int inlen=sizeof(SYSPARAMS);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_SETSYSPARAM,insys,inlen, (void*)outsys,&outlen);
	return ret;
}

int webGetStreamParam(StreamParam *outval)
{
	int inlen=sizeof(StreamParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETSTREAMPARAM,NULL,inlen,outval,&outlen);
	return ret;
}

int webSetStreamParam(StreamParam *inval,StreamParam *outval)
{
	int inlen=sizeof(StreamParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_SETSTREAMPARAM,inval,inlen,outval,&outlen);
	return ret;
}

int webGetRtspParam(RtspParam *outval)
{
	int inlen=sizeof(RtspParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETRTSPPARAM,NULL,inlen, (void*)outval,&outlen);
	return ret;
}


int webSetRtspParam(RtspParam  *inval,RtspParam  *outval)
{
	int inlen=sizeof(RtspParam);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_SETRTSPPARAM,inval,inlen,outval,&outlen);
	return ret;
}

int webFarCtrlCamera(int addr,int type,int speed)
{
	unsigned char inval[12]={0};
	int inlen=sizeof(inval);
	int outlen = 0,ret = 0;
	char outval[64]={0};
	memcpy(inval,&type,sizeof(type));
	memcpy(inval+sizeof(type),&speed,sizeof(speed));
	memcpy(inval+sizeof(type)+sizeof(speed),&addr,sizeof(addr));
	ret =appCmdStringParse(MSG_FAR_CTRL,inval,inlen,outval,&outlen);
	return ret;
}


/*reboot */
int webRebootSystem(void)
{
	int inval=0,inlen=sizeof(int);
	int outval=0,outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_REBOOTSYS,inval,inlen,&outval,&outlen);
	return ret;
}



/*Web Sync time*/
int Websynctime(DATE_TIME_INFO* inval,DATE_TIME_INFO *outval)
{
	int inlen=sizeof(DATE_TIME_INFO);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_SYNCTIME,inval,inlen,outval,&outlen);
	return ret;
}

int WebgetEncodetime(DATE_TIME_INFO *outval)
{
	int inlen=sizeof(DATE_TIME_INFO);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GETENCODETIME,NULL,inlen,outval,&outlen);
	return ret;
}
/*Save Param flash*/
int WebSaveParam(void)
{
	int inval = 0;
	int inlen=sizeof(int);
	int outlen = 0,ret = 0;
	int outval =0;
	ret =appCmdIntParse(MSG_SAVEPARAM,inval,inlen, &outval,&outlen);
	return ret;
}

#if 0
int webGetDHCPFlag(int *outval)
{
	int inval = 0;
	int inlen=sizeof(int);
	int outlen = 0,ret = 0;
	ret =appCmdIntParse(MSG_GETDHCPFLAG,inval,inlen, outval,&outlen);
	return ret;
}

int webSetDHCPFlag(int inval,int *outval)
{
	int inlen=sizeof(int);
	int outlen = 0,ret = 0;
	ret =appCmdIntParse(MSG_SETDHCPFLAG,inval,inlen,outval,&outlen);
	return ret;
}

#endif

int restoreSet(void)
{
	int inlen=sizeof(int);
	int outlen = 0,ret = 0;
	int inval = 0;
	int outval=0;
	ret =appCmdIntParse(MSG_RESTORESET,inval,inlen,&outval,&outlen);
	return ret;
}
#if 0
/*get Multi */
int WebGetMultiAddr(MultAddr *ip,int protocol)
{
	webMsgInfo  msg;
	int Sockfd  =  0;
	char temp[100];

	if(Sockfd == 0)	{
		Sockfd= connectToDecodeServer();
	}
	if(Sockfd <= 0)	{
		return -1;
	}
	msg.cmd = MSG_GET_MULT_ADDR;
	msg.len = sizeof(webMsgInfo)+4;	
	memcpy(temp,&msg,sizeof(webMsgInfo));
	memcpy(temp+sizeof(webMsgInfo),&protocol,4);
	if(WriteData(Sockfd, (void*)temp,msg.len) < 1)	{
		close(Sockfd);
		return -1;
	}
	ReadData(Sockfd,(char *)ip,sizeof(MultAddr));
	close(Sockfd);		
    return 0;
}


/*start Multi */
int WebSetMultiStart(MultAddr *ip,int 	protocol)
{
	webMsgInfo  msg;
	int 	Sockfd  =  0,ret = 0;
	char 	temp[100];
	
	if(Sockfd == 0)	{
		Sockfd= connectToDecodeServer();
	}
	if(Sockfd <= 0)	{
		return -1;
	}
	msg.cmd = MSG_START_MULT;
	msg.len = sizeof(webMsgInfo)+sizeof(MultAddr)+sizeof(protocol);
	memcpy(temp,&msg,sizeof(webMsgInfo));
	memcpy(temp+sizeof(webMsgInfo),&protocol,sizeof(protocol));
	memcpy(temp+sizeof(webMsgInfo)+sizeof(protocol),ip,sizeof(MultAddr));
	if(WriteData(Sockfd, (void*)temp,msg.len) < 1)	{
		close(Sockfd);
		return -1;
	}
	ReadData(Sockfd,&ret,sizeof(int));
	close(Sockfd);	
	return ret;	
}


#endif
/*start Multi */
int WebSetMultiStop(int outval)
{
	int inlen=sizeof(int);
	int outlen = 0,ret = 0;
	int inval = 0;
	ret =appCmdIntParse(MSG_STOP_MULT,inval,inlen,&outval,&outlen);
	return ret;
}



/*get protocol status*/
int WebGetProtocol(Protocol *outval)
{
	int inlen=sizeof(Protocol);
	int outlen = 0,ret = 0;
	ret =appCmdStructParse(MSG_GET_PROTOCOL,NULL,inlen,outval,&outlen);
	return ret;
}





/*Web get input source*/
int  WebGetinputSource(inputtype *outval)
{
	int inval = 0;
	int inlen=sizeof(inputtype);
	int outlen = 0,ret = 0;
	ret =appCmdIntParse(MSG_GETINPUTTYPE,inval,inlen,outval,&outlen);
	return ret;
}



/*Set Input Source Mode*/
int WebSetinputSource(inputtype inval,inputtype *outval)
{
	int inlen=sizeof(int);
	int	outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_SETINPUTTYPE,inval,inlen,&outval,&outlen);
	return outval;
}


/*remote control protocol*/
int WebGetCtrlProto(int *outdex)
{
	int index = 0;
	int inlen=sizeof(int);
	int	outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_GET_CTRL_PROTO,index,inlen,outdex,&outlen);
	return ret;
}


/*set control protocol*/
int WebSetCtrlProto(int index,int *outdex)
{
	int inlen=sizeof(int);
	int	outlen=sizeof(int);
	int ret;
	ret =appCmdIntParse(MSG_SET_CTRL_PROTO,index,inlen,&outdex,&outlen);
	return ret;
}


int WebGetDevideType(char *outval,int inlen)
{
	int outlen = 0,ret = 0;
	int inval = 0;
	ret =appCmdStringParse(MSG_GETDEVICETYPE,NULL,inlen,outval,&outlen);
	return ret;
}


int WebSetDeviceType(const char *inval,char *outval)    // devicetype len must be 15 bit
{
	int inlen=strlen(inval);
	int outlen = 0,ret = 0;
	ret =appCmdStringParse(MSG_SETDEVICETYPE,inval,inlen, outval,&outlen);
	return ret;
}

int WebGetTextPos(int type,int *x,int *y)
{
	if(type == TOP_LEFT)
	{
		*x = 0;
		*y = 0;
	}
	else if(type == TOP_RIGHT)
	{
		*x = 1920;
		*y = 0;		
	}
	else if(type == BOTTOM_LEFT)
	{
		*x = 0;
		*y = 1080-64 ;			
	}
	else if(type == BOTTOM_RIGHT)
	{
		*x = 1920 ;
		*y = 1080-64 ;		
	}
	else if(type == CENTERED)
	{
		*x = (1920 - 16*10)/2;
		*y = 1080/2 -8;
	}
	else
	{
		*x = 0;
		*y = 0;	
	}
	return 0;
}

int WebGetLogoPos(int type,int *x,int *y)
{
	if(type == TOP_LEFT)
	{
		*x = 0;
		*y = 0;
	}
	else if(type == TOP_RIGHT)
	{
		*x = 1920 ;
		*y = 0;		
	}
	else if(type == BOTTOM_LEFT)
	{
		*x = 0;
		*y = 1080;			
	}
	else if(type == BOTTOM_RIGHT)
	{
		*x = 1920;
		*y = 1080 ;		
	}
	else if(type == CENTERED)
	{
		*x = (1920 )/2;
		*y = 1080/2 ;
	}
	else
	{
		*x = 0;
		*y = 0;	
	}
	return 0;
}

#if 0
/*Test main function*/
int main()
{
	AudioParam audio;
	VideoParam video;
	SYSPARAMS sys;
	int ret = 0;
	printf("Web start main!!\n");
	inputtype input = 0;
	char version[20];
	DATE_TIME_INFO time;
	textinfo text;

	while(1) {
/*		memset(&audio,0,sizeof(audio));
		WebGetAudioParam(&audio);
		printf("#################audio####################\n");
		printf("audio.SampleRate = %d\n",audio.SampleRate);
		printf("audio.BitRate = %d\n",audio.BitRate);
		WebGetVideoParam(&video);
		printf("video.sBitrate=%d\n",video.sBitrate);
		printf("video.nQuality = %d \n",video.nQuality);
		printf("video.nDataCodec=%x\n",video.nDataCodec);
		WebGetSysParam(&sys);
		printf("sys.strName = %s\n",sys.strName);
		printf("sys.strVer = %s\n",sys.strVer);
		printf("sys.bType=%d\n",sys.bType);


		printf("###########################################\n");
		audio.SampleRate  = 2;
		audio.BitRate  = 64000;
		WebSetAudioParam(&audio);
		video.nQuality = 50;
		video.sBitrate = 128;
		WebSetVideoParam(&video);
		strcpy(sys.strName,"enc1200");
		sys.bType = 3;
		WebSetSysParam(&sys);	
		input = SDI;
		printf("SDI   input = %d \n",input);	
		WebSetinputSource(input);
		printf("get input source \n");
		WebGetinputSource(&input);
		printf("input = %d \n",input);	
//		input = SDI;
		WebSetinputSource(input);
		WebGetkernelversion(version);
		printf("kernel version = %s\n",version);
		WebGetfpgaversion(version);
		printf("FPGA version = %s\n",version);
*/
/*		memset(&time,0,sizeof(time));
		Websynctime(&time);
		text.xpos = 300;
		text.ypos = 400;
		strcpy(text.msgtext,"好的");
		ret =Webaddtexttodisplay(&text);
		printf("add Text To display!! = %d\n",ret);*/
		//WebUpdatesystem("/opt/reach/update.tgz");
		ret = webGetDHCPFlag();
		printf("return = %d\n",ret);
		sleep(14);
	}

	return 0;
}
#endif
