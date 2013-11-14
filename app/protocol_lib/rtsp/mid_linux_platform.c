/*********************************************************************
*      封装标准的windows platform相关的一些函数
*
*
*
**********************************************************************/
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

#include "mid_linux_platform.h"
#include "../log/log_common.h"
//using namespace std;


/*****************************多线程 fun****************************************/
//int mid_plat_task_create(const char* name, int stack_size, int priority,
//							void* func, void *arg,mid_plat_pthread_t *threadid)
//{
//	if(name == NULL)
//	{
//		PRINTF("ERROR!!");
//		return -1;
//	}
//	int err = 0;
//
//	PRINTF("begin : i will create the new thread,the thread name is %s.\n",name);
//	err = pthread_create(threadid, NULL, (void *)func, arg);
//	if (err)
//		ERR_PRN("ERROR!!!pthread_create\n");
//	else
//		PRINTF("end : i will create the new thread,the thread name is %s.\n",name);
//	return err;
//
//}

//
//void mid_plat_task_init(void)
//{
//	printf("mid_task_init \n");
//	//mid_task_init();
//	return ;
//}
//


/*****************************socket fun****************************************/
//
//int mid_plat_set_sockaddr_in(mid_plat_sockaddr_in *addrsrv,char *ip, unsigned short port)
//{
//	printf("\n");
//	if(ip == NULL)
//	{
//		addrsrv->sin_addr.s_addr = htonl(INADDR_ANY);
//	}
//	else
//	{
//		addrsrv->sin_addr.s_addr = htonl(ip);
//	}
//	addrsrv->sin_family = AF_INET;
//	addrsrv->sin_port = htons(port);
//	return 0;
//}


int mid_plat_set_socket_stimeout(mid_plat_socket sSocket, unsigned long time)
{
	struct timeval timeout ;
	int ret = 0;

	timeout.tv_sec = time / 1000; //3
	timeout.tv_usec = time % 1000;

	ret = setsockopt(sSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	if(ret == -1) {
		PRINTF("setsockopt() Set Send Time Failed\n");
	}

	return ret;
}


//set socket recv timeout
int mid_plat_set_socket_rtimeout(mid_plat_socket sSocket, unsigned long time)
{

	struct timeval timeout ;
	int ret = 0;

	timeout.tv_sec = time / 1000; //3
	timeout.tv_usec = time % 1000;

	ret = setsockopt(sSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

	if(ret == -1) {
		PRINTF("setsockopt() Set Send Time Failed\n");
	}

	return ret;
}


//close socket
int mid_plat_close_socket(mid_plat_socket socket)
{
	close(socket);
	return 0;
}


/****************************时间 fun****************************************/

static unsigned long mid_clock(void)
{
	//return mid_time_get_current_time();
	unsigned long msec;
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	msec = tp.tv_sec;
	msec = msec * 1000 + tp.tv_nsec / 1000000;

	return msec;
}
//获取开机到现在的运行时间，毫秒级别
unsigned long mid_plat_get_runtime(void)
{
	return mid_clock();
}

//sleep
void mid_plat_sleep(int ms)
{
	usleep(ms * 1000);
}


int mid_plat_compare_sockaddr_in(mid_plat_sockaddr_in *temp1, mid_plat_sockaddr_in *temp2)
{
	if(temp1 == NULL || temp2 == NULL) {
		return -1;
	}

	PRINTF("temp1->sin_port =%d,temp2->sin_port=%d\n", temp1->sin_port, temp2->sin_port);

	if((temp1->sin_port == temp2->sin_port && temp2->sin_port != 0) && temp1->sin_addr.s_addr == temp2->sin_addr.s_addr) {
		return 0;
	}

	return -1;
}

#endif


