/*******************************************************************
*  rtsp porting  common for rtsp .
*  主要用于封装RTSP需要用到部分APP的函数的封装
*                               add by zm
* 												2011-5-31
********************************************************************/

/*
* RTSP使用前需要强制I帧，用于VLC/QT快速播放
*/
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
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>

#include "stream_output_struct.h"
#include "rtsp_output_server.h"

#include "common.h"

extern int ForceIframe(int start);

int rtsp_porting_server_need_stop()
{
	if(app_rtsp_get_active() == 0) {
		return 1;
	} else {
		return 0;
	}
}


int rtsp_porting_force_Iframe(int roomid)
{
	ForceIframe(0);
	return 0;
}


int rtsp_porting_close_client(int pos, void *client)
{
	return stream_close_rtsp_client(pos, client);
}

void *rtsp_porting_create_client(int pos, struct sockaddr_in *video_addr, struct sockaddr_in *audio_addr)
{
	return stream_create_rtsp_client(pos, video_addr, audio_addr);
}

int rtsp_porting_get_ginfo(int *mult, char *ip, unsigned short *vport, unsigned short *aport)
{
	return stream_get_rtsp_ginfo(mult, ip, vport, aport);
}




