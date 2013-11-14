#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <pthread.h>

#include "rtmp.h"
#include "h264.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/bio.h"
#include "openssl/rand.h"

unsigned int                            g_uiDataStreamLen = 0;
RTMPCliParam                            gRTMPCliPara[CLIENT_MAX];
char                                    g_presv[1500];
char                                    *g_rtmp_sendbuf;
char                                    *g_pSendBuf_video;
char                                    *g_pSendBuf_audio;

unsigned long                           rtmp_time_org_video;
unsigned long                           rtmp_time_org_audio;
STR_RTMP_PARAM                          strRtmpParams0;
static STR_RTMP_PARAM                          *strRtmpParams = &strRtmpParams0;
char                                    g_psend[1500];
/*pthread mutex lock*/
pthread_mutex_t                         rtmp_send_m;
pthread_t                               rtmp_threadid[CLIENT_MAX] = {0};
typedef enum bool {
    FALSE_ = 0,
    TRUE_
} BOOL;

/*Resize Param table*/
typedef struct __THREAD_PARAM__ {
	int                 nPos;
	int                 client_socket;
	struct sockaddr_in  client_addr;
} Thread_Param;


#ifdef PRINT_DEUBG
#include <stdio.h>
typedef enum {
    DL_NONE, //=0
    DL_ERROR,  //=1
    DL_WARNING,  //=2
    DL_FLOW,	//=3
    DL_DEBUG,	//=4
    DL_ALL, 	//all
} EDebugLevle;

#define DEBUG_LEVEL   	(DL_ALL)	// // TODO: modify DEBUG Level

#if 1		//日志文件
#define DEBUG(x,fmt,arg...) \
	do {\
		if ((x)<DEBUG_LEVEL){\
			fprintf(stderr, fmt, ##arg);\
		}\
	}while (0)
#else
#include <syslog.h>
#define DEBUG(x,fmt,arg...) \
	do {\
		if ((x)<DEBUG_LEVEL){ \
			syslog(LOG_INFO, "%s",__func__);\
			syslog(LOG_INFO,fmt, ##arg);\
		}\
	}while (0)
#endif
#else
#define DEBUG(x,fmt,arg...) do {}while(0)
#endif




/**************************************************************************************************
                    获取系统当前时间
**************************************************************************************************/
static unsigned long getCurrentTime(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned long ultime;

	gettimeofday(&tv , &tz);
	ultime = tv.tv_sec * 1000 + ((tv.tv_usec) / 1000);
	return (ultime);
}

/**************************************************************************************************
                    获取rtmp 当前版本号
**************************************************************************************************/
char *RtmpGetVersion()
{
	char *version;
	version = RTMP_VERSION;
	return (version);
}

void RtmpInit()
{
	int cli;

	for(cli = 0; cli < CLIENT_MAX; cli++) {
		SETRTMPREADY(0, cli, FALSE);
		SETRTMPUSED(0, cli, FALSE);
	}

	g_rtmp_sendbuf = (char *)malloc(sizeof(char) * 0x400000);
	g_pSendBuf_video = malloc(0x500000);
	g_pSendBuf_audio = malloc(0x2000);
	pthread_mutex_init(&rtmp_send_m, NULL);
}
//**************************************************************************
/*RTMP clent exit*/
//**************************************************************************
int RtmpDelClient(int nPos)
{
	SETRTMPREADY(0, nPos, 0);
	SETRTMPUSED(0, nPos, FALSE);
	return 0;
}
//**************************************************************************
/*RTMP protocol exit*/
//**************************************************************************
int RtmpExit()
{
	free(g_rtmp_sendbuf);
	free(g_pSendBuf_video);
	free(g_pSendBuf_audio);
	pthread_mutex_destroy(&rtmp_send_m);
	return 0;
}

//**************************************************************************
/*get null RTMP client index*/
//**************************************************************************
int RtmpGetNullClientIndex()
{
	int cli ;

	for(cli = 0; cli < CLIENT_MAX; cli++) {
		if((!ISRTMPUSED(0, cli))) {
			SETRTMPUSED(0, cli, TRUE);
			return cli;
		}
	}

	return -1;
}

//**************************************************************************
/*get null RTMP client index*/
//**************************************************************************
void RtmpAddClient(struct sockaddr_in client_addr, int sock, int nPos)
{
	SETRTMPCLIENTADDR(0, nPos, client_addr);
	SETRTMPCLIENTSOCK(0, nPos, sock);
}

//**************************************************************************
/*get RTSP clients' number*/
//**************************************************************************
int RtmpGetClientNum()
{
	int cli ;
	int count = 0;

	for(cli = 0; cli < CLIENT_MAX; cli++) {
		if((ISRTMPUSED(0, cli)) && (ISRTMPREADY(0, cli))) {
			count = count + 1;
		}
	}

	return count;
}


unsigned int GetDigestOffset0(unsigned char *pBuffer)
{
	unsigned int offset = pBuffer[8] + pBuffer[9] + pBuffer[10] + pBuffer[11];
	offset = offset % 728;
	offset = offset + 12;

	if(offset + 32 >= 1536) {
		//	ASSERT("Invalid digest offset");
	}

	return offset;
}

unsigned int GetDigestOffset1(unsigned char *pBuffer)
{
	unsigned int offset = pBuffer[772] + pBuffer[773] + pBuffer[774] + pBuffer[775];
	offset = offset % 728;
	offset = offset + 776;

	//    DEBUG(DL_DEBUG,"+++++++++++++offset = %d++++++++++++++\n",offset);
	if(offset + 32 >= 1536) {
		//	ASSERT("Invalid digest offset");
	}

	return offset;
}

unsigned int GetDigestOffset(unsigned char *pBuffer, unsigned char schemeNumber)
{
	switch(schemeNumber) {
		case 0: {
			return GetDigestOffset0(pBuffer);
		}

		case 1: {
			return GetDigestOffset1(pBuffer);
		}

		default: {
			return GetDigestOffset0(pBuffer);
		}
	}
}

void HMACsha256(const void *pData, unsigned int dataLength,
                const void *pKey, unsigned int keyLength, void *pResult)
{
	unsigned int digestLen;

	HMAC_CTX ctx;
	HMAC_CTX_init(&ctx);
	HMAC_Init_ex(&ctx, (unsigned char *) pKey, keyLength, EVP_sha256(), NULL);
	HMAC_Update(&ctx, (unsigned char *) pData, dataLength);
	HMAC_Final(&ctx, (unsigned char *) pResult, &digestLen);
	HMAC_CTX_cleanup(&ctx);
}



void MsgChunkHead(unsigned char *pBuf, unsigned char ucChunkType, unsigned int uiChunkId,
                  unsigned char ucMsgType, unsigned int uiMsgStreamid, unsigned int uiMsgLen,
                  unsigned char *pucData, BOOL bFillInData, unsigned int uiTimeStamp) //ONLY TYPE 1 CHUNK HEAD
{
	unsigned char i = 0;

	if(ucChunkType > 3 || uiChunkId > 63) {
		return;
	}

	if(ucChunkType == CHUNK_HEAD_12_BYTES) {
		pBuf[0] = ((ucChunkType << 6) & 0xc) | (uiChunkId & 0x3f);
		pBuf[1] = (uiTimeStamp >> 16) & 0xff;
		pBuf[2] = (uiTimeStamp >> 8) & 0xff;
		pBuf[3] = uiTimeStamp & 0xff;
		pBuf[4] = (uiMsgLen >> 16) & 0xff;
		pBuf[5] = (uiMsgLen >> 8) & 0xff;
		pBuf[6] = uiMsgLen & 0xff;
		pBuf[7] = ucMsgType;
		pBuf[8] = (uiMsgStreamid >> 24) & 0xff;
		pBuf[9] = (uiMsgStreamid >> 16) & 0xff;
		pBuf[10] = (uiMsgStreamid >> 8) & 0xff;
		pBuf[11] = uiMsgStreamid & 0xff;

		if(bFillInData == TRUE) {
			for(i = 0; i < uiMsgLen; i++) {
				pBuf[CHUNK_HEAD_LEN_TYPE_ZERO + i] = pucData[i];
			}
		}
	} else if(ucChunkType == CHUNK_HEAD_8_BYTES) {
		pBuf[0] = (0x40) | (uiChunkId & 0x3f);
		pBuf[1] = (uiTimeStamp >> 16) & 0xff;
		pBuf[2] = (uiTimeStamp >> 8) & 0xff;
		pBuf[3] = uiTimeStamp & 0xff;
		pBuf[4] = (uiMsgLen >> 16) & 0xff;
		pBuf[5] = (uiMsgLen >> 8) & 0xff;
		pBuf[6] = uiMsgLen & 0xff;
		pBuf[7] = ucMsgType;

		if(bFillInData == TRUE) {
			for(i = 0; i < uiMsgLen; i++) {
				pBuf[CHUNK_HEAD_LEN_TYPE_ONE + i] = pucData[i];
			}
		}
	}

	return;
}



unsigned char MsgServerBw(unsigned char *pucData, unsigned int uiBw)
{
	unsigned char pData[8], ucDataLen = 0;
	pData[0] = (uiBw >> 24) & 0xff;
	pData[1] = (uiBw >> 16) & 0xff;
	pData[2] = (uiBw >> 8) & 0xff;
	pData[3] = uiBw & 0xff;
	ucDataLen = 4;
	MsgChunkHead(pucData, CHUNK_HEAD_12_BYTES, 2, ServerBw, 0, ucDataLen, pData, 1, 0);
	return (CHUNK_HEAD_LEN_TYPE_ZERO + ucDataLen);
}

unsigned char MsgClientBw(unsigned char *pucData, unsigned int uiBw, unsigned char ucType)
{
	unsigned char pData[8], ucDataLen = 0;
	pData[0] = (uiBw >> 24) & 0xff;
	pData[1] = (uiBw >> 16) & 0xff;
	pData[2] = (uiBw >> 8) & 0xff;
	pData[3] = uiBw & 0xff;
	pData[4] = ucType;
	ucDataLen = 5;
	MsgChunkHead(pucData, CHUNK_HEAD_12_BYTES, 2, ClientBw, 0, ucDataLen, pData, 1, 0);
	return (CHUNK_HEAD_LEN_TYPE_ZERO + ucDataLen);
}

unsigned char MsgPing(unsigned char *pucData, unsigned char *pucValue)
{
	unsigned char pData[8], ucDataLen = 0;
	pData[0] = *pucValue;
	pData[1] = *(pucValue + 1);
	pData[2] = *(pucValue + 2);
	pData[3] = *(pucValue + 3);
	pData[4] = *(pucValue + 4);
	pData[5] = *(pucValue + 5);
	ucDataLen = 6;
	MsgChunkHead(pucData, CHUNK_HEAD_12_BYTES, 2, Ping, 0, ucDataLen, pData, 1, 0);
	return (CHUNK_HEAD_LEN_TYPE_ZERO + ucDataLen);
}


unsigned char MsgChunkSize(unsigned char *pucData, unsigned int uiChunkSize)
{
	unsigned char pData[8], ucDataLen = 0;
	//	unsigned int uiChunkLen=CHUNK_LEN;
	pData[0] = (uiChunkSize >> 24) & 0xff;
	pData[1] = (uiChunkSize >> 16) & 0xff;
	pData[2] = (uiChunkSize >> 8) & 0xff;
	pData[3] = uiChunkSize & 0xff;
	ucDataLen = 4;
	MsgChunkHead(pucData, CHUNK_HEAD_12_BYTES, 2, ChunkSize, 0, ucDataLen, pData, 1, 0);
	return (CHUNK_HEAD_LEN_TYPE_ZERO + ucDataLen);
}


unsigned short  MsgInvokeResult(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucResultName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = (usCharLen >> 8) & 0xff;
	pSend[usDataLen++] = usCharLen & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->aucResult, 8);
	usDataLen += 8;

	return usDataLen;
}


unsigned short MsgInvokeFmsver(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucFmsVerName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = (usCharLen >> 8) & 0xff;
	pSend[usDataLen++] = usCharLen & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = (p->ucFmsVerLen >> 8) & 0xff;
	pSend[usDataLen++] = p->ucFmsVerLen & 0xff;
	memcpy(&pSend[usDataLen], p->aucFmsVer, p->ucFmsVerLen);
	usDataLen += p->ucFmsVerLen;

	return usDataLen;
}


void RtmpHandshake(int nPos)
{
	char pRecv1[TCP_BUF_LEN], pRecv2[TCP_BUF_LEN], pSend[1536 * 2 + 1];
	char _pOutputBuffer[3100];
	char pInputBuffer[3100];
	char buffer1[1504];
	char buffer2[512];
	memset(_pOutputBuffer, 0, 3100);
	memset(pInputBuffer, 0, 1504);
	memset(buffer1, 0, 1504);
	memset(buffer2, 0, 512);
	unsigned char genuineFMSKey[] = {
		0x47, 0x65, 0x6e, 0x75, 0x69, 0x6e, 0x65, 0x20,
		0x41, 0x64, 0x6f, 0x62, 0x65, 0x20, 0x46, 0x6c,
		0x61, 0x73, 0x68, 0x20, 0x4d, 0x65, 0x64, 0x69,
		0x61, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
		0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Media Server 001
		0xf0, 0xee, 0xc2, 0x4a, 0x80, 0x68, 0xbe, 0xe8,
		0x2e, 0x00, 0xd0, 0xd1, 0x02, 0x9e, 0x7e, 0x57,
		0x6e, 0xec, 0x5d, 0x2d, 0x29, 0x80, 0x6f, 0xab,
		0x93, 0xb8, 0xe6, 0x36, 0xcf, 0xeb, 0x31, 0xae
	}; // 68


	memset(pRecv1, 0, TCP_BUF_LEN);
	memset(pSend, 0x88, TCP_BUF_LEN);
	int resvlen1 = 0, resvlen2 = 0, TotalShakeLen = 0, HandShakeStep = 0, i, j;

	while(TRUE) {
		resvlen1 = recv(GETRTMPCLIENTSOCK(0, nPos), pRecv1, TCP_BUF_LEN, 0);
		resvlen2 = recv(GETRTMPCLIENTSOCK(0, nPos), pRecv2, TCP_BUF_LEN, 0);

		if(pRecv1[0] == 0x03) {
			TotalShakeLen = (resvlen1 + resvlen2);
			//	continueflg=1;
		}

		if(TotalShakeLen == HAND_SHAKE_FIX_LEN + 1) {
			HandShakeStep = 1;
			TotalShakeLen = 0;
		}

		//step1
		if(HandShakeStep == 1) {
			pSend[0] = 0x03;
			pSend[1] = 0; //0x05;
			pSend[2] = 0; //0xc5;
			pSend[3] = 0; //0xd0;
			pSend[4] = 0; //0x84;
			pSend[5] = 1; //0x03;
			pSend[6] = 2; //0x00;
			pSend[7] = 3; //0x01;
			pSend[8] = 4; //0x01;

			//=======================
			//generate the digest

			unsigned char _validationScheme = 1;
			/*此处从图像BUFF内任意地址拷贝一段长3072字节的数据到_pOutputBuffer memcpy(_pOutputBuffer,p,3072);*/
			unsigned int serverDigestOffset = GetDigestOffset(_pOutputBuffer, _validationScheme);

			if(resvlen1 + resvlen2 != 1537) {
				return;
			}

			memcpy(pInputBuffer, &pRecv1[1], (resvlen1 - 1));
			memcpy(pInputBuffer + resvlen1 - 1, pRecv2, resvlen2);

			//UINT8 *pTempBuffer = new UINT8[1536 - 32];
			unsigned char *pTempBuffer = buffer1;
			/*此处从图像BUFF内任意地址拷贝一段长[1536 - 32]字节的数据到pTempBuffer memcpy(_pOutputBuffer,p,(1536 - 32));*/
			memcpy(pTempBuffer, _pOutputBuffer, serverDigestOffset);
			memcpy(pTempBuffer + serverDigestOffset, _pOutputBuffer + serverDigestOffset + 32,
			       1536 - serverDigestOffset - 32);

			//			UINT8 *pTempHash = new UINT8[512];
			unsigned char *pTempHash = buffer2;
			/*此处从图像BUFF内任意地址拷贝一段长[512]字节的数据到pTempHash
			memcpy(_pOutputBuffer,p,512);*/
			HMACsha256(pTempBuffer, 1536 - 32, genuineFMSKey, 36, pTempHash);

			//put the digest in place
			memcpy(_pOutputBuffer + serverDigestOffset, pTempHash, 32);

			//**** SECOND 1536 bytes from server response ****//
			//Compute the chalange index from the initial client request
			unsigned int keyChallengeIndex = GetDigestOffset(pInputBuffer, _validationScheme);
			//FINEST("keyChallengeIndex: %u", keyChallengeIndex);//1462

			//compute the key
			//			pTempHash = new UINT8[512];
			pTempHash = buffer2;
			/*此处从图像BUFF内任意地址拷贝一段长[512]字节的数据到pTempHash
			memcpy(_pOutputBuffer,p,512);*/

			HMACsha256(pInputBuffer + keyChallengeIndex, //pData
			           32, //dataLength
			           genuineFMSKey, //key
			           68, //keyLength
			           pTempHash //pResult
			          );

			//generate the hash
			//			UINT8 *pLastHash = new UINT8[512];
			unsigned char *pLastHash = buffer2;
			/*此处从图像BUFF内任意地址拷贝一段长[512]字节的数据到pLastHash
			memcpy(pLastHash,p,512);*/

			HMACsha256(_pOutputBuffer + 1536, //pData
			           1536 - 32, //dataLength
			           pTempHash, //key
			           32, //keyLength
			           pLastHash //pResult
			          );

			//put the hash where it belongs
			memcpy(_pOutputBuffer + 1536 * 2 - 32, pLastHash, 32);
			memcpy(&pSend[1], _pOutputBuffer, 3072);

			//delete[] _pOutputBuffer;
			pSend[0] = 0x03;
			send(GETRTMPCLIENTSOCK(0, nPos), pSend, HAND_SHAKE_FIX_LEN * 2 + 1, 0);

			//=======================
			resvlen1 = recv(GETRTMPCLIENTSOCK(0, nPos), pRecv1, TCP_BUF_LEN, 0);
			resvlen2 = recv(GETRTMPCLIENTSOCK(0, nPos), pRecv2, TCP_BUF_LEN, 0);

			if(pRecv2[(HAND_SHAKE_FIX_LEN - resvlen1)] == 0x03) {
				for(i = HAND_SHAKE_FIX_LEN - resvlen1; i < resvlen2; i++) {
					if(pRecv2[i] == 0x63 && pRecv2[i + 1] == 0x6f && pRecv2[i + 2] == 0x6e && pRecv2[i + 3] == 0x6e
					   && pRecv2[i + 4] == 0x65 && pRecv2[i + 5] == 0x63 && pRecv2[i + 6] == 0x74 && pRecv2[i + 7] == 0) { //connect
						for(j = 0; j < 8; j++) {
							strRtmpParams->aucResult[j] = pRecv2[i + 8 + j];
						}
					}

					if(pRecv2[i] == 0x63 && pRecv2[i + 1] == 0x61 && pRecv2[i + 2] == 0x70 && pRecv2[i + 3] == 0x61
					   && pRecv2[i + 4] == 0x62 && pRecv2[i + 5] == 0x69 && pRecv2[i + 6] == 0x6c && pRecv2[i + 7] == 0x69
					   && pRecv2[i + 8] == 0x74 && pRecv2[i + 9] == 0x69 && pRecv2[i + 0xa] == 0x65 && pRecv2[i + 0xb] == 0x73
					   && pRecv2[i + 0xc] == 0) { //capablities
						strRtmpParams->aucCapabilities[0] = 0x40;
						strRtmpParams->aucCapabilities[1] = 0x3f;
						strRtmpParams->aucCapabilities[2] = 0;
						strRtmpParams->aucCapabilities[3] = 0;
						strRtmpParams->aucCapabilities[4] = 0;
						strRtmpParams->aucCapabilities[5] = 0;
						strRtmpParams->aucCapabilities[6] = 0;
						strRtmpParams->aucCapabilities[7] = 0;
					}

					if(pRecv2[i] == 0x6f && pRecv2[i + 1] == 0x62 && pRecv2[i + 2] == 0x6a && pRecv2[i + 3] == 0x65
					   && pRecv2[i + 4] == 0x63 && pRecv2[i + 5] == 0x74 && pRecv2[i + 6] == 0x45 && pRecv2[i + 7] == 0x6e
					   && pRecv2[i + 8] == 0x63 && pRecv2[i + 9] == 0x6f && pRecv2[i + 0xa] == 0x64 && pRecv2[i + 0xb] == 0x69
					   && pRecv2[i + 0xc] == 0x6e && pRecv2[i + 0xd] == 0x67 && pRecv2[i + 0xe] == 0) { //objectEncoding
						for(j = 0; j < 8; j++) {
							strRtmpParams->aucObjectEnc[j] = pRecv2[i + 0xf + j];
						}
					}

				}

				return ;

			}
		}
	}

	return;
}


unsigned short MsgInvokeCapabilities(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucCapabilitiesName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->aucCapabilities, 8);
	usDataLen += 8;

	return usDataLen;
}

unsigned short MsgInvokeMode(unsigned char *pSend)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucModeName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->aucMode, 8);
	usDataLen += 8;

	return usDataLen;
}

unsigned short MsgInvokeLevel(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucLevelName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = ((p->ucLevelLen) >> 8) & 0xff;
	pSend[usDataLen++] = (p->ucLevelLen) & 0xff;
	memcpy(&pSend[usDataLen], p->aucLevel, p->ucLevelLen);
	usDataLen += p->ucLevelLen;

	return usDataLen;
}


unsigned short MsgInvokeCode(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucCodeName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = ((p->ucCodeLen) >> 8) & 0xff;
	pSend[usDataLen++] = (p->ucCodeLen) & 0xff;
	memcpy(&pSend[usDataLen], p->aucCode, p->ucCodeLen);
	usDataLen += p->ucCodeLen;

	return usDataLen;
}


unsigned short MsgInvokeData(unsigned char *pSend)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucDataName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_MIXED_ARRAY;
	memcpy(&pSend[usDataLen], p->aucData, 4);
	usDataLen += 4;

	return usDataLen;
}


unsigned short MsgInvokeVersion(unsigned char *pSend)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucVersionName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = (p->ucVersionLen >> 8) & 0xff;
	pSend[usDataLen++] = p->ucVersionLen & 0xff;
	memcpy(&pSend[usDataLen], p->aucVersion, p->ucVersionLen);
	usDataLen += p->ucVersionLen;

	return usDataLen;
}


unsigned short MsgInvokeClientid(unsigned char *pSend)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;
#ifdef WOWZA
	pucChar = (char *)p->aucClientIdName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->aucClientId, 8);
	usDataLen += 8;
#else
	pucChar = (char *)p->aucClientIdName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = ((p->ucClientidLen) >> 8) & 0xff;
	pSend[usDataLen++] = (p->ucClientidLen) & 0xff;
	memcpy(&pSend[usDataLen], p->aucClientId, p->ucClientidLen);
	usDataLen += p->ucClientidLen;
#endif
	return usDataLen;
}


unsigned short MsgInvokeObjectencoding(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucObjectEncName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->aucObjectEnc, 8);
	usDataLen += 8;

	return usDataLen;
}

unsigned char MsgInvoke(unsigned char *pucData, unsigned char ucChunkType,
                        unsigned char ucChunkId, unsigned int uiMsgLen, unsigned int uiMsgStreamid)
{
	if(ucChunkType == CHUNK_HEAD_12_BYTES) {
		pucData[0] = ((ucChunkType << 6) & 0xc0) | (ucChunkId & 0x3f);
		pucData[1] = 0;
		pucData[2] = 0;
		pucData[3] = 0;
		pucData[4] = (uiMsgLen >> 16) & 0xff;
		pucData[5] = (uiMsgLen >> 8) & 0xff;
		pucData[6] = uiMsgLen & 0xff;
		pucData[7] = Invoke;
		pucData[8] = (uiMsgStreamid >> 24) & 0xff;
		pucData[9] = (uiMsgStreamid >> 16) & 0xff;
		pucData[10] = (uiMsgStreamid >> 8) & 0xff;
		pucData[11] = uiMsgStreamid & 0xff;

		return (CHUNK_HEAD_LEN_TYPE_ZERO);
	} else if(ucChunkType == CHUNK_HEAD_8_BYTES) {
		pucData[0] = ((ucChunkType << 6) & 0xc0) | (ucChunkId & 0x3f);
		pucData[1] = 0;
		pucData[2] = 0;
		pucData[3] = 0;
		pucData[4] = (uiMsgLen >> 16) & 0xff;
		pucData[5] = (uiMsgLen >> 8) & 0xff;
		pucData[6] = uiMsgLen & 0xff;
		pucData[7] = Invoke;

		return (CHUNK_HEAD_LEN_TYPE_ONE);
	}

	return 0;
}


void RtmpParamConfigFirstTime()
{
	STR_RTMP_PARAM *pParam = NULL;
	pParam = strRtmpParams;
	strcpy(pParam->aucResultName, "_result");

	strcpy(pParam->aucCapabilitiesName, "capabilities");

	strcpy(pParam->aucCode, "NetConnection.Connect.Success");
	pParam->ucCodeLen = strlen(pParam->aucCode);
	strcpy(pParam->aucCodeName, "code");

	strcpy(pParam->aucDescription, "Connection succeeded");
	pParam->ucDescriptionLen = strlen(pParam->aucDescription);
	strcpy(pParam->aucDescriptionName, "description");

	strcpy(pParam->aucFmsVer, "FMS/3,0,1,123");
	pParam->ucFmsVerLen = strlen(pParam->aucFmsVer);
	strcpy(pParam->aucFmsVerName, "fmsVer");

	strcpy(pParam->aucClientIdName, "clientid");

	strcpy(pParam->aucObjectEncName, "objectEncoding");

	pParam->aucMode[0] = 0x3f;
	pParam->aucMode[1] = 0xf0;
	pParam->aucMode[2] =	0;
	pParam->aucMode[3] =	0;
	pParam->aucMode[4] =	0;
	pParam->aucMode[5] =	0;
	pParam->aucMode[6] =	0;
	pParam->aucMode[7] =	0;
	strcpy(pParam->aucModeName, "mode");

	strcpy(pParam->aucLevel, "status");
	pParam->ucLevelLen = strlen(pParam->aucLevel);
	strcpy(pParam->aucLevelName, "level");

	strcpy(pParam->aucVersion, "3,5,2,654");
	pParam->ucVersionLen = strlen(pParam->aucVersion);
	strcpy(pParam->aucVersionName, "version");

	//pParam->aucData[8]={};
	pParam->aucData[0] = 0;
	pParam->aucData[1] = 0;
	pParam->aucData[2] = 0;
	pParam->aucData[3] = 0;
	strcpy(pParam->aucDataName, "data");

	strcpy(pParam->aucIsFastPlayName, "isFastPlay");
	strcpy(pParam->aucOnStatusName, "onStatus");
	strcpy(pParam->aucRtmpSampleAccessName, "|RtmpSampleAccess");
	return;
}

void RtmpSendFirstPacket(int nPos)
{
	unsigned char pSend[TCP_BUF_LEN];
	unsigned char aucPingValue[6];
	unsigned short usDataLen = 0, usLenForInvoke = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;
	usDataLen += MsgServerBw(pSend, 0x2625a0);
	usDataLen += MsgClientBw((pSend + usDataLen), 0x2625a0, 2);
	aucPingValue[0] = 0;
	aucPingValue[1] = 0;
	aucPingValue[2] = 0;
	aucPingValue[3] = 0;
	aucPingValue[4] = 0;
	aucPingValue[5] = 0;
	usDataLen += MsgPing((pSend + usDataLen), &aucPingValue[0]);
#ifdef WOWZA
	usDataLen += MsgChunkSize((pSend + usDataLen), CHUNK_LEN);
#endif

	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
	usLenForInvoke = usDataLen;

	pSend[usDataLen++] = TYPE_STRING;
	//result
	usDataLen += MsgInvokeResult((pSend + usDataLen), strRtmpParams);

	//object start
	pSend[usDataLen++] = TYPE_OBJECT;

	//fmsver
	usDataLen += MsgInvokeFmsver((pSend + usDataLen), strRtmpParams);

	//capabilities
	usDataLen += MsgInvokeCapabilities((pSend + usDataLen), strRtmpParams);
#ifdef WOWZA
	//mode
	usDataLen += MsgInvokeMode((pSend + usDataLen), strRtmpParams);
#endif
	//object ending & start
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 9;
	pSend[usDataLen++] = TYPE_OBJECT;

	//level
	usDataLen += MsgInvokeLevel((pSend + usDataLen), strRtmpParams);

	//code
	usDataLen += MsgInvokeCode((pSend + usDataLen), strRtmpParams);

	//description
	//usDataLen += MsgInvokeDescription((pSend+usDataLen),strRtmpParams);
	pucChar = (char *)p->aucDescriptionName;
	unsigned short usCharLen1 = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen1) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen1) & 0xff;

	pSend[usDataLen++] = 0x64;
	pSend[usDataLen++] = 0xc3;
	strcpy((char *)&pSend[usDataLen], "escription");
	usDataLen += strlen("escription");
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0x14;
	strcpy((char *)&pSend[usDataLen], "Connection succeeded");
	usDataLen += strlen("Connection succeeded");

#ifdef WOWZA
	//data
	usDataLen += MsgInvokeData((pSend + usDataLen), strRtmpParams);

	//version
	usDataLen += MsgInvokeVersion((pSend + usDataLen), strRtmpParams);

	//object ending & start
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 9;


	//clientid
	usDataLen += MsgInvokeClientid((pSend + usDataLen), strRtmpParams);
#endif

	//objectencding
	usDataLen += MsgInvokeObjectencoding((pSend + usDataLen), strRtmpParams);

	//object end
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 9;

	MsgInvoke((pSend + usLenForInvoke - CHUNK_HEAD_LEN_TYPE_ZERO), CHUNK_HEAD_12_BYTES, 3, usDataLen - usLenForInvoke - 1, 0);
#ifdef _WRITEFILE
	FILE *pfile = fopen("D:\\rtmp.bin", "w+b");

	if(pfile != NULL) {
		fwrite(pSend, (usDataLen), sizeof(char), pfile);
		fclose(pfile);
	}

#endif
	send(GETRTMPCLIENTSOCK(0, nPos), (char *)(&pSend[0]), usDataLen, 0);
	FILE *pfile0 = fopen("RtmpSendFirstPacket.bin", "w+b");

	if(pfile0 != NULL) {
		fwrite(pSend, (usDataLen), sizeof(char), pfile0);
		fclose(pfile0);
	}

	return;

}

unsigned short RtmpResvMsg(int nPos)
{
	char aucResv1[TCP_BUF_LEN], aucResv2[TCP_BUF_LEN];
	char *pResv1 = aucResv1;
	unsigned int resvlen1 = 0, resvlen2 = 0;
	//	Sleep(30);

	resvlen1 = recv(GETRTMPCLIENTSOCK(0, nPos), pResv1, TCP_BUF_LEN, 0);
	memcpy(&aucResv2[resvlen2], pResv1, resvlen1);
	resvlen2 += resvlen1;
#if _WRITEFILE
	pfile1 = fopen("D:\\rtmp3.bin", "w+b");

	if(pfile1 != NULL) {
		fwrite(aucResv2, resvlen2, sizeof(char), pfile1);
		fclose(pfile1);
	}

#endif
	return resvlen1;
}


unsigned short MsgInvokeOnstatus(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucOnStatusName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->aucOnStatus, 8);
	usDataLen += 8;

	return usDataLen;
}


unsigned short MsgInvokeDescription(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucDescriptionName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = (p->ucDescriptionLen >> 8) & 0xff;
	pSend[usDataLen++] = p->ucDescriptionLen & 0xff;
	memcpy(&pSend[usDataLen], p->aucDescription, p->ucDescriptionLen);
	usDataLen += p->ucDescriptionLen;

	return usDataLen;
}

unsigned short MsgInvokeDetails(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucDetailsName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = ((p->ucDetailsLen) >> 8) & 0xff;
	pSend[usDataLen++] = (p->ucDetailsLen) & 0xff;
	memcpy(&pSend[usDataLen], p->aucDetails, p->ucDetailsLen);
	usDataLen += p->ucDetailsLen;

	return usDataLen;
}

unsigned short MsgInvokeRtmpsampleaccess(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->aucRtmpSampleAccessName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_BOOLEAN;
	pSend[usDataLen++] = p->boolRtmpSampleAccess;

	return usDataLen;
}


unsigned char MsgNotify(unsigned char *pucData, unsigned char ucChunkType,
                        unsigned char ucChunkId, unsigned int uiMsgLen, unsigned int uiMsgStreamid)
{
	if(ucChunkType == CHUNK_HEAD_12_BYTES) {
		pucData[0] = ((ucChunkType << 6) & 0xc0) | (ucChunkId & 0x3f);
		pucData[1] = 0;
		pucData[2] = 0;
		pucData[3] = 0;
		pucData[4] = (uiMsgLen >> 16) & 0xff;
		pucData[5] = (uiMsgLen >> 8) & 0xff;
		pucData[6] = uiMsgLen & 0xff;
		pucData[7] = Notify;
		pucData[8] = (uiMsgStreamid >> 24) & 0xff;
		pucData[9] = (uiMsgStreamid >> 16) & 0xff;
		pucData[10] = (uiMsgStreamid >> 8) & 0xff;
		pucData[11] = uiMsgStreamid & 0xff;

		return (CHUNK_HEAD_LEN_TYPE_ZERO);
	} else if(ucChunkType == CHUNK_HEAD_8_BYTES) {
		pucData[0] = ((ucChunkType << 6) & 0xc0) | (ucChunkId & 0x3f);
		pucData[1] = 0;
		pucData[2] = 0;
		pucData[3] = 0;
		pucData[4] = (uiMsgLen >> 16) & 0xff;
		pucData[5] = (uiMsgLen >> 8) & 0xff;
		pucData[6] = uiMsgLen & 0xff;
		pucData[7] = Notify;

		return (CHUNK_HEAD_LEN_TYPE_ONE);
	}

	return 0;
}


unsigned short MsgNotifyDuration(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->strData.aucDurationName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->strData.aucDuration, 8);
	usDataLen += 8;

	return usDataLen;
}


unsigned short MsgNotifyWidth(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->strData.aucWidthName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->strData.aucWidth, 8);
	usDataLen += 8;

	return usDataLen;
}


unsigned short MsgNotifyHeight(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;

	pucChar = (char *)p->strData.aucHeightName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->strData.aucHeight, 8);
	usDataLen += 8;

	return usDataLen;
}

unsigned short MsgInvokeServer(unsigned char *pSend, STR_RTMP_PARAM *strRtmpParams)
{
	unsigned short usDataLen = 0, usCharLen = 0;
	STR_RTMP_PARAM *p = strRtmpParams;
	char *pucChar = NULL;
	pucChar = (char *)p->aucServerName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = ((usCharLen) >> 8) & 0xff;
	pSend[usDataLen++] = (usCharLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = ((p->ucServerLen) >> 8) & 0xff;
	pSend[usDataLen++] = (p->ucServerLen) & 0xff;
	memcpy(&pSend[usDataLen], p->aucServer, p->ucServerLen);
	usDataLen += p->ucServerLen;
	return usDataLen;
}

unsigned int SearchingAvcc(unsigned char ucVideoSolution, unsigned char *pSend)
{
	unsigned int        uiSectionLen = 0;
	unsigned char      *paucAvccParam = NULL;
	unsigned char g_aucAvccParam[10][0x40] = {
		//480p
		{
			0x21, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 0x80, 0x28, 0xff, 0xe1, 0x00, 0x0a, 0x67, 0x42, 0x80,
			0x28, 0x8c, 0x8c, 0x40, 0x50, 0x1e, 0x88, 0x01, 0x00, 0x04, 0x68, 0xde, 0x3c, 0x80, 0x00, 0x00,
			0x02
		},
		//720p
		{
			0x22, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 0x80, 0x28, 0xff, 0xe1, 0x00, 0x0b, 0x67, 0x42, 0x80,
			0x28, 0x8c, 0x8c, 0x40, 0x28, 0x02, 0xd8, 0x80, 0x01, 0x00, 0x04, 0x68, 0xde, 0x3c, 0x80, 0x00,
			0x00, 0x01
		},
		//1080p
		{
			0x22, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 0x80, 0x28, 0xff, 0xe1, 0x00, 0x0b, 0x67, 0x42, 0x80,
			0x28, 0x8c, 0x8c, 0x40, 0x3c, 0x01, 0x12, 0x20, 0x01, 0x00, 0x04, 0x68, 0xde, 0x3c, 0x80, 0x00,
			0x00, 0x01
		},
		//vga
		{
			0x22, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 0x80, 0x28, 0xff, 0xe1, 0x00, 0x0b, 0x67, 0x42, 0x80,
			0x28, 0x8c, 0x8c, 0x40, 0x20, 0x03, 0x08, 0x80, 0x01, 0x00, 0x04, 0x68, 0xde, 0x3c, 0x80, 0x00,
			0x00, 0x01
		},
		//DEFAULT
		{
			0x21, 0x61, 0x76, 0x63, 0x43, 0x01, 0x42, 0x80, 0x28, 0xff, 0xe1, 0x00, 0x0a, 0x67, 0x42, 0x80,
			0x28, 0x8c, 0x8c, 0x40, 0x50, 0x1e, 0x88, 0x01, 0x00, 0x04, 0x68, 0xde, 0x3c, 0x80, 0x00, 0x00,
			0x02
		},
	};

	switch(ucVideoSolution) {
		case VIDEO_RESOLUTION_480P:
			paucAvccParam = &g_aucAvccParam[0][5];
			uiSectionLen = g_aucAvccParam[0][0];
			break;

		case VIDEO_RESOLUTION_720P:
			paucAvccParam = &g_aucAvccParam[1][5];
			uiSectionLen = g_aucAvccParam[1][0];
			break;

		case VIDEO_RESOLUTION_1080P:
			paucAvccParam = &g_aucAvccParam[2][5];
			uiSectionLen = g_aucAvccParam[2][0];
			break;

		case VIDEO_RESOLUTION_VGA:
			paucAvccParam = &g_aucAvccParam[3][5];
			uiSectionLen = g_aucAvccParam[3][0];
			break;

		default:
			paucAvccParam = &g_aucAvccParam[4][5];
			uiSectionLen = g_aucAvccParam[4][0];
			break;
	}

	pSend[0] = 0x17;
	pSend[1] = 0;
	pSend[2] = 0;
	pSend[3] = 0;
	pSend[4] = 0;
	memcpy(&pSend[5], paucAvccParam, (uiSectionLen - 5));
	return uiSectionLen;

}


void RtmpParamConfigSecondTime()
{
	STR_RTMP_PARAM *pParam = NULL;
	pParam = strRtmpParams;

	strcpy(pParam->aucOnStatusName, "onStatus");
	pParam->aucOnStatus[0] = 0;
	pParam->aucOnStatus[1] = 0;
	pParam->aucOnStatus[2] = 0;
	pParam->aucOnStatus[3] = 0;
	pParam->aucOnStatus[4] = 0;
	pParam->aucOnStatus[5] = 0;
	pParam->aucOnStatus[6] = 0;
	pParam->aucOnStatus[7] = 0;

	strcpy(pParam->aucCode, "NetStream.Play.Reset");
	pParam->ucCodeLen = strlen(pParam->aucCode);
	strcpy(pParam->aucCodeName, "code");
#ifdef WOWZA
	strcpy(pParam->aucDescription, "Playing and resetting Extremists.m4v.");
#else
	strcpy(pParam->aucDescription, "reset...");
#endif
	pParam->ucDescriptionLen = strlen(pParam->aucDescription);
	strcpy(pParam->aucDescriptionName, "description");

	return;
}


void RtmpSendSecondPacket(int nPos)
{

	unsigned short usDataLen = 0, usCharLen = 0;
	unsigned char pSend[TCP_BUF_LEN];
	char *pucChar = NULL;
	STR_RTMP_PARAM *p = strRtmpParams;
#ifdef WOWZA
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
#else
	usDataLen += CHUNK_HEAD_LEN_TYPE_ONE;
#endif

	//pSend[usDataLen++]=0;
	pSend[usDataLen++] = TYPE_STRING;
	pucChar = (char *)p->aucResultName;
	usCharLen = strlen(pucChar);
	pSend[usDataLen++] = (usCharLen >> 8) & 0xff;
	pSend[usDataLen++] = usCharLen & 0xff;
	strcpy((char *)&pSend[usDataLen], pucChar);
	usDataLen += usCharLen;
	pSend[usDataLen++] = TYPE_NUMBER;
	pSend[usDataLen++] = 0x40;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;

	pSend[usDataLen++] = TYPE_NULL;
	pSend[usDataLen++] = TYPE_NUMBER;
	memcpy(&pSend[usDataLen], p->aucResult, 8);
	usDataLen += 8;
#ifdef WOWZA
	MsgChunkHead(pSend, CHUNK_HEAD_12_BYTES, 3, Invoke, 0, (usDataLen - CHUNK_HEAD_LEN_TYPE_ZERO), NULL, 0, 0);
#else
	MsgChunkHead(pSend, CHUNK_HEAD_8_BYTES, 3, Invoke, 0, (usDataLen - CHUNK_HEAD_LEN_TYPE_ONE), NULL, 0, 0);
#endif


	send(GETRTMPCLIENTSOCK(0, nPos), (char *)(&pSend[0]), usDataLen, 0);
	FILE *pfile1 = fopen("RtmpSendSecondPacket.bin", "w+b");

	if(pfile1 != NULL) {
		fwrite(pSend, usDataLen, sizeof(char), pfile1);
		fclose(pfile1);
	}

	return;
}

//audio and video use this function //
unsigned char *RtmpdSendThirdPacket(int nPos)
{
	unsigned char          *pSend, aucPingValue[6];
	unsigned short          usDataLen = 0, usLenForInvoke = 0;
	pSend = g_psend;
	RtmpParamConfigSecondTime(strRtmpParams);
	aucPingValue[0] = 0;
	aucPingValue[1] = 4;
	aucPingValue[2] = 0;
	aucPingValue[3] = 0;
	aucPingValue[4] = 0;
	aucPingValue[5] = 1;
	usDataLen += MsgPing((pSend + usDataLen), &aucPingValue[0]);
	aucPingValue[1] = 0;
	usDataLen += MsgPing((pSend + usDataLen), &aucPingValue[0]);
	//    DEBUG(DL_FLOW,"------------- MsgPing -----------------\n\n");
	//invoke:reset
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
	usLenForInvoke = usDataLen;
	pSend[usDataLen++] = TYPE_STRING;
	usDataLen += MsgInvokeOnstatus((pSend + usDataLen), strRtmpParams);
	pSend[usDataLen++] = TYPE_NULL;
	pSend[usDataLen++] = TYPE_OBJECT;
	usDataLen += MsgInvokeLevel((pSend + usDataLen), strRtmpParams);
	usDataLen += MsgInvokeCode((pSend + usDataLen), strRtmpParams);
	usDataLen += MsgInvokeDescription((pSend + usDataLen), strRtmpParams);

	strcpy(strRtmpParams->aucDetailsName, "details");
	strcpy(strRtmpParams->aucDetails, "mp4:onlyvideo.mp4");
	strRtmpParams->ucDetailsLen = strlen(strRtmpParams->aucDetails);
	usDataLen += MsgInvokeDetails((pSend + usDataLen), strRtmpParams);
	strcpy(strRtmpParams->aucClientId, "9_1_159986376");
	strRtmpParams->ucClientidLen = strlen(strRtmpParams->aucClientId);
	//usDataLen += MsgInvokeClientid((pSend+usDataLen),strRtmpParams);
	unsigned short uslen = strlen("clientid");
	pSend[usDataLen++] = (uslen >> 8) & 0xff;
	pSend[usDataLen++] = uslen & 0xff;
	strcpy((char *)&pSend[usDataLen], "client");
	usDataLen += strlen("client");
	pSend[usDataLen++] = 0xd4;
	pSend[usDataLen++] = 0x69;
	pSend[usDataLen++] = 0x64;
	pSend[usDataLen++] = TYPE_STRING;
	uslen = strlen("9_1_159986376");
	pSend[usDataLen++] = (uslen >> 8) & 0xff;
	pSend[usDataLen++] = uslen & 0xff;
	strcpy((char *)&pSend[usDataLen], "9_1_159986376");
	usDataLen += strlen("9_1_159986376");
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 9;
	MsgInvoke((pSend + usLenForInvoke - CHUNK_HEAD_LEN_TYPE_ZERO), CHUNK_HEAD_12_BYTES,
	          0x14, usDataLen - usLenForInvoke - 1, 0x01000000);


	//invoke:start
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
	usLenForInvoke = usDataLen;
	pSend[usDataLen++] = TYPE_STRING;
	usDataLen += MsgInvokeOnstatus((pSend + usDataLen), strRtmpParams);
	pSend[usDataLen++] = TYPE_NULL;
	pSend[usDataLen++] = TYPE_OBJECT;
	usDataLen += MsgInvokeLevel((pSend + usDataLen), strRtmpParams);
	strcpy(strRtmpParams->aucCode, "NetStream.Play.Start");
	usDataLen += MsgInvokeCode((pSend + usDataLen), strRtmpParams);
	strcpy(strRtmpParams->aucDescription, "start...");
	strRtmpParams->ucDescriptionLen = strlen(strRtmpParams->aucDescription);
	usDataLen += MsgInvokeDescription((pSend + usDataLen), strRtmpParams);
	usDataLen += MsgInvokeDetails((pSend + usDataLen), strRtmpParams);
	//strcpy(strRtmpParams->aucClientId,"9_1_159986376");
	//usDataLen += MsgInvokeClientid((pSend+usDataLen),strRtmpParams);
	uslen = strlen("clientid");
	pSend[usDataLen++] = (uslen >> 8) & 0xff;
	pSend[usDataLen++] = uslen & 0xff;
	strcpy((char *)&pSend[usDataLen], "client");
	usDataLen += strlen("client");
	pSend[usDataLen++] = 0xd4;
	pSend[usDataLen++] = 0x69;
	pSend[usDataLen++] = 0x64;
	pSend[usDataLen++] = TYPE_STRING;
	uslen = strlen("9_1_159986376");
	pSend[usDataLen++] = (uslen >> 8) & 0xff;
	pSend[usDataLen++] = uslen & 0xff;
	strcpy((char *)&pSend[usDataLen], "9_1_159986376");
	usDataLen += strlen("9_1_159986376");

	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 9;
	MsgInvoke((pSend + usLenForInvoke - CHUNK_HEAD_LEN_TYPE_ZERO), CHUNK_HEAD_12_BYTES,
	          0x14, usDataLen - usLenForInvoke - 1, 0x01000000);

	//notify
	usLenForInvoke = usDataLen;
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
	pSend[usDataLen++] = TYPE_STRING;
	strRtmpParams->boolRtmpSampleAccess = 0;
	usDataLen += MsgInvokeRtmpsampleaccess((pSend + usDataLen), strRtmpParams);
	pSend[usDataLen++] = TYPE_BOOLEAN;
	pSend[usDataLen++] = 0;
	MsgNotify((pSend + usLenForInvoke), CHUNK_HEAD_12_BYTES, 0x14, usDataLen - usLenForInvoke - CHUNK_HEAD_LEN_TYPE_ZERO, 0x01000000);


	//notify
	usLenForInvoke = usDataLen;
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
	pSend[usDataLen++] = TYPE_STRING;
	unsigned char ucStreamLen = 0;
	ucStreamLen = strlen("onStatus");
	pSend[usDataLen++] = (ucStreamLen >> 8) & 0xff;
	pSend[usDataLen++] = (ucStreamLen) & 0xff;
	strcpy((char *)&pSend[usDataLen], "onStatus");
	usDataLen += strlen("onStatus");
	pSend[usDataLen++] = TYPE_OBJECT;
	strcpy(strRtmpParams->aucCode, "NetStream.Data.Start");
	strRtmpParams->ucCodeLen = strlen(strRtmpParams->aucCode);
	usDataLen += MsgInvokeCode((pSend + usDataLen), strRtmpParams);
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 9;

	MsgNotify((pSend + usLenForInvoke), CHUNK_HEAD_12_BYTES, 0x14,
	          usDataLen - usLenForInvoke - CHUNK_HEAD_LEN_TYPE_ZERO, 0x01000000);

	//notify
	usLenForInvoke = usDataLen;
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
	pSend[usDataLen++] = TYPE_STRING;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0xa;
	strcpy((char *)&pSend[usDataLen], "onMetaData");
	usDataLen += 0xa;
	pSend[usDataLen++] = TYPE_OBJECT;

	strcpy(strRtmpParams->strData.aucDurationName, "duration");
	strRtmpParams->strData.aucDuration[0] = 0x41; //0x40;
	strRtmpParams->strData.aucDuration[1] = 0x72; //8;//0x27;
	strRtmpParams->strData.aucDuration[2] = 0xe4; //0;//0xc2;
	strRtmpParams->strData.aucDuration[3] = 0x86; //0;//0x8f;
	strRtmpParams->strData.aucDuration[4] = 0xa0; //0;//0x5c;
	strRtmpParams->strData.aucDuration[5] = 0; //0x28;
	strRtmpParams->strData.aucDuration[6] = 0; //0xf5;
	strRtmpParams->strData.aucDuration[7] = 0; //0xc3;
	usDataLen += MsgNotifyDuration((pSend + usDataLen), strRtmpParams); //延续时间，似乎无影响
	strcpy(strRtmpParams->strData.aucWidthName, "width");
	strRtmpParams->strData.aucWidth[0] = 0x40;
	strRtmpParams->strData.aucWidth[1] = 0x84; //0x66;
	strRtmpParams->strData.aucWidth[2] = 0;
	strRtmpParams->strData.aucWidth[3] = 0;
	strRtmpParams->strData.aucWidth[4] = 0;
	strRtmpParams->strData.aucWidth[5] = 0;
	strRtmpParams->strData.aucWidth[6] = 0;
	strRtmpParams->strData.aucWidth[7] = 0;
	usDataLen += MsgNotifyWidth((pSend + usDataLen), strRtmpParams);
	strcpy(strRtmpParams->strData.aucHeightName, "height");
	strRtmpParams->strData.aucHeight[0] = 0x40;
	strRtmpParams->strData.aucHeight[1] = 0x7e; //0x62;
	strRtmpParams->strData.aucHeight[2] = 0;
	strRtmpParams->strData.aucHeight[3] = 0;
	strRtmpParams->strData.aucHeight[4] = 0;
	strRtmpParams->strData.aucHeight[5] = 0;
	strRtmpParams->strData.aucHeight[6] = 0;
	strRtmpParams->strData.aucHeight[7] = 0;
	usDataLen += MsgNotifyHeight((pSend + usDataLen), strRtmpParams);
	strcpy(strRtmpParams->aucServerName, "Server");
	strcpy(strRtmpParams->aucServer, "C++ RTMP Server (http://www.szreach.com)");
	strRtmpParams->ucServerLen = strlen(strRtmpParams->aucServer);
	usDataLen += MsgInvokeServer((pSend + usDataLen), strRtmpParams);
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 0;
	pSend[usDataLen++] = 9;
	MsgNotify((pSend + usLenForInvoke), CHUNK_HEAD_12_BYTES, 0x14, usDataLen - usLenForInvoke - CHUNK_HEAD_LEN_TYPE_ZERO, 0x01000000);

	//chunksize
	unsigned char aucChunkData[4] = {0, 0x40, 0, 0};
	MsgChunkHead((pSend + usDataLen), CHUNK_HEAD_12_BYTES, 2, ChunkSize, 0x00000000, 4, aucChunkData, TRUE, 0);
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO + 4;

	//修改从此开始，替换掉以前的代码
	/***************************************************************/
	unsigned char pucDataVideo[0x30] = {0x17, 0x00, 0x00, 0x00, 0x00, 0x01, 0x4d, 0x40, 0x1e, 0xff, 0xe1, 0x00, 0x19, 0x67, 0x4d, 0x40,
	                                    0x1e, 0x9a, 0x72, 0x01, 0x20, 0x6f, 0x60, 0x22, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x03,
	                                    0x00, 0x65, 0x1e, 0x2c, 0x5a, 0x24, 0x01, 0x00, 0x04, 0x68, 0xee, 0x3c, 0x80
	                                   };


	unsigned char pucDataAudio[0x10] = {0xaf, 0x00, 0x13, 0x90};
	//     unsigned char pucDataAudio[0x10]={0xaf,0x00,0x12,0x10};

	unsigned int uiStreamStartSectionLen = 0, uiStreamStartSectionLen1 = 0;
	usLenForInvoke = usDataLen;
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;

	//uiStreamStartSectionLen=SearchingAvcc(VIDEO_RESOLUTION_480P,(pSend+usDataLen));
	uiStreamStartSectionLen = 0x2d;
	MsgChunkHead((pSend + usLenForInvoke), CHUNK_HEAD_12_BYTES, 0x15, VideoData, 0x01000000, \
	             uiStreamStartSectionLen, pucDataVideo, TRUE, 0); //avcc
	usDataLen += uiStreamStartSectionLen;

	usLenForInvoke = usDataLen;
	usDataLen += CHUNK_HEAD_LEN_TYPE_ZERO;
	uiStreamStartSectionLen1 = 0x4;
	MsgChunkHead((pSend + usLenForInvoke), CHUNK_HEAD_12_BYTES, 0x14, AudioData, 0x01000000, \
	             uiStreamStartSectionLen1, pucDataAudio, TRUE, 0); //avcc
	usDataLen += uiStreamStartSectionLen1;

	pSend[usDataLen++] = 0xd5;
	memcpy(&pSend[usDataLen], pucDataVideo, uiStreamStartSectionLen);
	usDataLen += uiStreamStartSectionLen;

	pSend[usDataLen++] = 0xd4;
	memcpy(&pSend[usDataLen], pucDataAudio, uiStreamStartSectionLen1);
	usDataLen += uiStreamStartSectionLen1;
	send(GETRTMPCLIENTSOCK(0, nPos), (char *)(&pSend[0]), usDataLen, 0);
	/********************************************************/
	//修改到此结束,之后开始发送音视频包
	SETRTMPREADY(0, nPos, TRUE);
	return 0;
}

void RtmpSetupConnection(int nPos)
{
	RtmpHandshake(nPos);
	RtmpParamConfigFirstTime();
	RtmpSendFirstPacket(nPos);
	RtmpResvMsg(nPos);
	RtmpSendSecondPacket(nPos);
	RtmpResvMsg(nPos);
	RtmpdSendThirdPacket(nPos);
	return;
}

int SendRtmpData(int sock, char *pData, int length)
{
	int                     nRet;
	int                     sendlen = 0;
	int                     nSize = length;
	int                     nCount = 0;

	while(sendlen < nSize) {
		nRet = send(sock, pData + sendlen, nSize - sendlen, 0);

		if(nRet < 0) {
			if((errno == ENOBUFS) && (nCount < 10)) {
				perror("network buffer have been full!");
				usleep(10000);
				nCount++;
				continue;
			}

			return nRet;
		} else if(0 == nRet) {
			perror("Send Net Data Error");
			continue;
		}

		sendlen += nRet;
		nCount = 0;
	}

	return sendlen;
}


void RtmpAudioPack(int nLen, unsigned char *pData)
{

	unsigned char                  *pBuf;
	unsigned char                   ucChunkheadLen = 0;
	unsigned int                    uiNALLen = 0;
	unsigned long                   uiTime;
	static   int                    flag = 0;
	int                             cnt = 0;
	int                             nRet = 0;
	pBuf = pData;
	uiNALLen = nLen - 7;

	if(0 == flag) {
		uiTime = 0x18;
		rtmp_time_org_audio = getCurrentTime();
		flag = 1;
	} else {
		uiTime = getCurrentTime() - rtmp_time_org_audio;
		rtmp_time_org_audio = getCurrentTime();
	}

	MsgChunkHead((unsigned char *)g_pSendBuf_audio, CHUNK_HEAD_8_BYTES, 0x14, AudioData, 0, uiNALLen + 2, NULL, FALSE, uiTime);
	ucChunkheadLen = CHUNK_HEAD_LEN_TYPE_ONE;
	g_pSendBuf_audio[ucChunkheadLen] = 0xaf; //(uiNALLen&0xff00)>>8;
	g_pSendBuf_audio[ucChunkheadLen + 1] = 0x01; //uiNALLen&0xff;

#if 1
	memcpy(&g_pSendBuf_audio[ucChunkheadLen + 2], &pBuf[7], uiNALLen);

	for(cnt = 0; cnt < CLIENT_MAX; cnt++) {
		if(ISRTMPUSED(0, cnt) && (ISRTMPREADY(0, cnt))) {
			pthread_mutex_lock(&rtmp_send_m);
			nRet = SendRtmpData(GETRTMPCLIENTSOCK(0, cnt), (char *)g_pSendBuf_audio, uiNALLen + 2 + CHUNK_HEAD_LEN_TYPE_ONE);

			if(nRet < 0) {
				SETRTMPREADY(0, cnt, FALSE);
				SETRTMPUSED(0, cnt, FALSE);
				perror("------------------socket cutdown----------------!\n");
				perror("rtmp client cutdown\n");
			}

			pthread_mutex_unlock(&rtmp_send_m);
		}
	}

#endif

	return;
}


void RtmpVideoPack(int nLen, unsigned char *pData)
{

	unsigned char              *pBuf;
	unsigned char               ucChunkheadLen = 0;
	unsigned int                uiLastSlicePos = 0;
	unsigned int                uiNALLen = 0, uiSliceLen = 0;
	unsigned int                j, uiOutData = 0;
	static  int                 flag = 0;
	unsigned long               uiTime;
	int                         cnt = 0;
	int                         nRet = 0;
	pBuf = pData;
	uiLastSlicePos = 0;

	for(j = 1; j < 26; j++) {
		if(pBuf[j] == 0 && pBuf[1 + j] == 0 && pBuf[2 + j] == 0 && pBuf[3 + j] == 1) {
			uiSliceLen = j - uiLastSlicePos - 4;
			pBuf[uiLastSlicePos] = ((uiSliceLen & 0xff000000) >> 24) & 0xff;
			pBuf[1 + uiLastSlicePos] = ((uiSliceLen & 0xff0000) >> 16) & 0xff;
			pBuf[2 + uiLastSlicePos] = ((uiSliceLen & 0xff00) >> 8) & 0xff;
			pBuf[3 + uiLastSlicePos] = (uiSliceLen & 0xff);
			uiLastSlicePos = j;
		}
	}

	uiSliceLen = nLen - 4 - uiLastSlicePos;
	pBuf[uiLastSlicePos] = ((uiSliceLen & 0xff000000) >> 24) & 0xff;
	pBuf[1 + uiLastSlicePos] = ((uiSliceLen & 0xff0000) >> 16) & 0xff;
	pBuf[2 + uiLastSlicePos] = ((uiSliceLen & 0xff00) >> 8) & 0xff;
	pBuf[3 + uiLastSlicePos] = (uiSliceLen & 0xff);
	uiNALLen = nLen;

	if(0 == flag) {
		uiTime = 0x23;
		rtmp_time_org_video = getCurrentTime();
		flag = 1;
	} else {
		uiTime = getCurrentTime() - rtmp_time_org_video;
		rtmp_time_org_video = getCurrentTime();
	}

	MsgChunkHead((unsigned char *)g_pSendBuf_video, CHUNK_HEAD_8_BYTES, 0X15, VideoData, 0, uiNALLen + 5, NULL, FALSE, uiTime);
	ucChunkheadLen = CHUNK_HEAD_LEN_TYPE_ONE;

	g_pSendBuf_video[ucChunkheadLen] = 0x27;

	g_pSendBuf_video[ucChunkheadLen + 1] = 1;
	g_pSendBuf_video[ucChunkheadLen + 2] = 0;
	g_pSendBuf_video[ucChunkheadLen + 3] = 0;
	g_pSendBuf_video[ucChunkheadLen + 4] = 0x50;

#if 1
	memcpy(&g_pSendBuf_video[ucChunkheadLen + 5], &pBuf[0], uiNALLen);

	for(cnt = 0; cnt < CLIENT_MAX; cnt++) {
		if(ISRTMPUSED(0, cnt) && ISRTMPREADY(0, cnt)) {
			pthread_mutex_lock(&rtmp_send_m);
			nRet = SendRtmpData(GETRTMPCLIENTSOCK(0, cnt), (char *)g_pSendBuf_video, uiNALLen + uiOutData + 13);

			if(nRet < 0) {
				SETRTMPREADY(0, cnt, FALSE);
				SETRTMPUSED(0, cnt, FALSE);
				perror("------------------socket cutdown----------------!\n");
				perror("rtmp client cutdown\n");
			}

			pthread_mutex_unlock(&rtmp_send_m);
		}
	}

#endif
	return;
}


/****************************************************************************************
与Rtmp客户端交互信息，并设置标志位
****************************************************************************************/
void RtmpConnectMsg(void *pParams)
{
	Thread_Param                   *rtmp_thread = (Thread_Param *)pParams;
	char                           *pRecv;
	unsigned char                   g_presv[1500];
	int                             nPos;
	int                             client_socket = rtmp_thread->client_socket;
	struct sockaddr_in              client_addr;
	memcpy(&client_addr, &rtmp_thread->client_addr, sizeof(struct sockaddr_in));
	nPos = rtmp_thread->nPos;
	DEBUG(DL_FLOW, "COME IN RTMP MSG\n");
	memset(g_presv, 0, 1500);
	pRecv = g_presv;
	RtmpAddClient(client_addr, client_socket, nPos);
	RtmpSetupConnection(nPos);
	DEBUG(DL_FLOW, "[RTMPMsg] Exit Pthread %d!!!!\n", nPos);
	rtmp_threadid[nPos] = 0;
	pthread_exit(NULL);
}

/****************************************************************************************
RtmpTask，用于处理Rtmp客户端连接
****************************************************************************************/
void RtmpTask()
{
	int                             nLen;
	int                             RtmpSerSock = 0;
	int                             RtmpCliSock = 0;
	int                             nPos = 0;
	int                             opt = 1;
	struct sockaddr_in              SrvAddr, ClientAddr;
	Thread_Param                    rtmpthread0;
	Thread_Param                    *rtmpthread = &rtmpthread0;
	DEBUG(DL_FLOW, "Rtmp Task........GetPid():%d\n", getpid());
	RtmpInit();
RTMPTARTRUN:
	DEBUG(DL_FLOW, "RtmpTask start...\n");

	bzero(&SrvAddr, sizeof(struct sockaddr_in));
	SrvAddr.sin_family = AF_INET;
	SrvAddr.sin_port = htons(RTMP_LISTEN_PORT);
	SrvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	RtmpSerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(RtmpSerSock < 0) {
		DEBUG(DL_ERROR, "ListenTask create error:%d,error msg: = %s\n", errno, strerror(errno));
		return;
	}

	setsockopt(RtmpSerSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(RtmpSerSock, (struct sockaddr *)&SrvAddr, sizeof(SrvAddr)) < 0) {
		DEBUG(DL_ERROR, "in RtmpTask:bind error:%d,error msg: = %s\n", errno, strerror(errno));
		return;
	}

	if(listen(RtmpSerSock, 10) < 0) {
		DEBUG(DL_ERROR, "listen error:%d,error msg: = %s", errno, strerror(errno));
		return ;
	}

	while(1)  {
		memset(&ClientAddr, 0, sizeof(struct sockaddr_in));
		nLen = sizeof(struct sockaddr_in);
		RtmpCliSock = accept(RtmpSerSock, (void *)&ClientAddr, (DWORD *)&nLen);

		if(RtmpCliSock > 0) {
			nPos = RtmpGetNullClientIndex();
			DEBUG(DL_ERROR, "client number = %d\n", nPos);

			if(-1 == nPos) {
				fprintf(stderr, "ERROR: max client error\n");
				DEBUG(DL_ERROR, "listen error:%d,error msg: = %s", errno, strerror(errno));
			} else {
				int nSize = 0;
				int result;
				nSize = 1;

				if((setsockopt(RtmpCliSock, SOL_SOCKET, SO_REUSEADDR, (void *)&nSize, sizeof(nSize))) == -1) {
					perror("setsockopt failed");
				}

				nSize = 0;
				nLen = sizeof(nLen);
				result = getsockopt(RtmpCliSock, SOL_SOCKET, SO_SNDBUF, &nSize , (DWORD *)&nLen);

				if(result) {
					fprintf(stderr, "getsockopt() errno:%d socket:%d  result:%d\n", errno, RtmpCliSock, result);
				}

				nSize = 1;

				if(setsockopt(RtmpCliSock, IPPROTO_TCP, TCP_NODELAY, &nSize , sizeof(nSize))) {
					fprintf(stderr, "Setsockopt error%d\n", errno);
				}

				rtmpthread->nPos = nPos;
				memcpy(&rtmpthread->client_addr, &ClientAddr, sizeof(struct sockaddr_in));
				rtmpthread->client_socket = RtmpCliSock;
				DEBUG(DL_FLOW, "begin to create thread of rtmp!\n");
				result = pthread_create(&rtmp_threadid[nPos], NULL, (void *)RtmpConnectMsg, (void *)rtmpthread);

				if(result)	{
					close(RtmpCliSock);
					fprintf(stderr, "creat pthread ClientMsg error  = %d!\n" , errno);
					continue;
				}
			}
		} else {
			if(errno == ECONNABORTED || errno == EAGAIN) { //软件原因中断
				DEBUG(DL_ERROR, "rtmp errno =%d program again start!!!\n", errno);
				usleep(100000);
				continue;
			}

			if(RtmpSerSock > 0) {
				close(RtmpSerSock);
			}

			goto RTMPTARTRUN;
		}

	}

	RtmpExit();
	DEBUG(DL_FLOW, "close the gServSock \n");

	if(RtmpSerSock > 0)	{
		DEBUG(DL_FLOW, "close gserv socket \n");
		close(RtmpSerSock);
	}

	RtmpSerSock = 0;
	return;
}


