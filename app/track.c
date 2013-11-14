#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include "sysparam.h"
#include "common.h"
#include "tcpcom.h"
#include "track.h"
#include "demo.h"
#include "log_common.h"
extern int gRemoteFD;
teachdsp teacher;
extern PLock            gSetP_m;
extern DSPCliParam gDSPCliPara[PORT_NUM];
#if 0
//SendCommand用于发送4个字节的常用命令，
//最后一个字节为checksum
//参数 type为命令类型,val为值
static int SendCommand(int fd, unsigned char type, unsigned char val)
{
	unsigned char pCode[5];
	int ret;
	memset(pCode, 0, 5);

	if(fd == -1) {
		DEBUG(DL_ERROR, "gRemoteFD=-1\n");
		return -1;
	}

	pCode[0] = 0xA0;
	pCode[1] = type;
	pCode[2] = val;
	pCode[3] = 0xFF;
	pCode[4] = pCode[0] ^ pCode[1] ^ pCode[2] ^ pCode[3];
	ret = SendDataToCom(fd, pCode, 5);

	if(ret == -1) {
		DEBUG(DL_ERROR, "SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "SendCommand() End\n");
	return 0;

}
#endif
int SetTrackStatus(int fd, int val)
{
	unsigned char pCode[5];
	int ret;
	memset(pCode, 0, 5);

	if(fd == -1) {
		DEBUG(DL_ERROR, "gRemoteFD=-1\n");
		return -1;
	}

	if(0 != val) {
		val = 0x3;
	}

	pCode[0] = 0xC0;
	pCode[1] = 0x00;
	pCode[2] = val;
	pCode[3] = 0xFF;
	pCode[4] = pCode[0] ^ pCode[1] ^ pCode[2] ^ pCode[3];
	ret = SendDataToCom(fd, pCode, 5);

	if(ret == -1) {
		DEBUG(DL_ERROR, "SendDataToCom() Error\n");
		return -1;
	}

	DEBUG(DL_DEBUG, "SendCommand() End\n");
	return 0;
}

int ListenSeries(void *pParam)
{
	unsigned char data[256];
	char szData[256];
	int len, i, cnt, sendsocket, nRet;
	fd_set rfds;
	PRINTF("get pid= %d\n", getpid());

	while(!gblGetQuit()) {

#ifdef DSS_ENC_1100_1200_DEBUG
		sleep(1);
		continue;
#endif
		FD_ZERO(&rfds);
		FD_SET(gRemoteFD, &rfds);
		select(gRemoteFD + 1, &rfds, NULL, NULL, NULL);
		len = read(gRemoteFD, data, 256) ; //成功返回数据量大小，失败返回－1
		PackHeaderMSG((BYTE *)szData, MSG_TRACKAUTO, HEAD_LEN + 1);
		szData[HEAD_LEN] = data[3];

		if(data[0] == 0x08 && data[1] == 0x09  && data[2] == 0x08) {
			for(i = 0; i < len; i++) {
				//printf("ReadDataFroCom:%x\n",data[i]);
			}

			for(cnt = 0; cnt < MAX_CLIENT; cnt++) {
				if(ISUSED(0, cnt) && ISLOGIN(0, cnt)) {
					pthread_mutex_lock(&gSetP_m.audio_video_m);
					sendsocket = GETSOCK(0, cnt);

					if(sendsocket > 0)  {
						nRet = WriteData(sendsocket, szData, HEAD_LEN + 1);

						if(nRet < 0) {
							SETCLIUSED(0, cnt, FALSE);
							SETCLILOGIN(0, cnt, FALSE);
							//SETFARCTRLLOCK(index, cnt, 0);
							writeWatchDog();
							DEBUG(DL_ERROR, "track Send to socket Error: SOCK = %d count = %d  errno = %d  ret = %d\n", sendsocket, cnt, errno, nRet);
						}
					}

					pthread_mutex_unlock(&gSetP_m.audio_video_m);
				}
			}

		}
	}

	return len;
}

