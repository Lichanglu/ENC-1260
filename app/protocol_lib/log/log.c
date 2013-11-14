/*******************************************************************************************
  Copyright (C), 1988-2008, yuxing software. Co., Ltd.
  Author:           liujianhua
  refactor:	   skypig
  Description:    B100 项目log 模块,包含此文件夹中相关头文件
  使用说明:
                       1: 添加新模块时，在此文件夹中加入"moduename_log.h"头文件
                           在此文件中加入g_debug_modulename
                       2: 在已有模块中加入新文件时，包含moduename_log.h
                       3: 需要关闭某模块log时，将此文件中的g_debug_modulename 置位0
*******************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<stdarg.h>
#include <string.h>
//#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
//#include "common.h"
//#include "mid_mutex.h"

#include "log.h"

#include <pthread.h>
#include <sys/types.h>

struct mid_mutex {
	pthread_mutex_t id;
};
typedef struct mid_mutex *mid_mutex_t;



#define TRACE_MAXSIZE	1024*5
#define TRACE_LOGSIZE	1024*10
static mid_mutex_t g_mutex = NULL;
static char *g_log_buf = 0;
static int g_log_off = 0;
static int g_log_end = 0;
static int g_newline_flg = 0;
static int g_debug_out = 0;
static char g_test[16] = {0};


//设置各个模块的打印级别
//int g_log_capture = DEBUG_ENABLE_DEBUG;  //
//int g_log_lowvideo = DEBUG_ENABLE_DEBUG;  //
//设置是否开启打印
int g_log_common = DEBUG_ENABLE_NULL;  // for old debug changed


static void read_log_ini();
static void parse_log_line(char *buf);
unsigned int mid_clock(void);


int trace_get_debug_level()
{
	if(g_log_common == DEBUG_ENABLE_NULL) {
		return 0;
	} else {
		return 1;
	}
}
int trace_set_debug_level(int flag)
{
	if(flag == 1) {
		g_log_common = DEBUG_ENABLE_DEBUG;
	} else if(flag == 0) {
		g_log_common = DEBUG_ENABLE_NULL;
	}

	return 0;
}

void trace_init()
{

	if(g_mutex) {
		return;
	}

	g_mutex = (mid_mutex_t)malloc(sizeof(struct mid_mutex));

	if(g_mutex == NULL) {
		goto Err;
	}

	if(pthread_mutex_init(&g_mutex->id, NULL)) {
		free(g_mutex);
		goto Err;
	}

	if(g_mutex == NULL) {
		goto Err;
	}

	g_log_buf = malloc(TRACE_LOGSIZE + TRACE_MAXSIZE + 15 + 4);

	if(g_log_buf == NULL) {
		goto Err;
	}

	g_log_off = 0;
	g_log_end = 0;
	g_newline_flg = 1;

#ifdef DOUBLE_SD

	if(2 == sys_get_double_num()) {
		strcpy(g_test, "input 2 ");
	} else {
		strcpy(g_test, "input 1 ");
	}

#endif
	read_log_ini();
Err:
	return;
}


/*  说明：读取LOG配置文件INI
*
*/
static void read_log_ini()
{
	FILE *fp = NULL;
	char buf[1024] = {0};

	fp = fopen("log.ini", "r");

	if(fp == NULL) {
		return ;
	}

	while(fgets(buf, sizeof(buf), fp) != NULL) {
		printf("i read buf frome log.ini ,the line =++%s++\n", buf);
		parse_log_line(buf);
	}

	printf("i will set g_debug_common = %d\n", g_log_common);
	fclose(fp);
	fp = NULL;

	return ;
}

/* 判断各个模块的初始LOG值
*  根据需要，加入相应的值
*/

static void parse_log_line(char *buf)
{
	int flag = -1;
	char str[512] = {0};


	if(buf == NULL || strlen(buf) < strlen("DEBUG_=")) {
		printf("ERROR\n");
		return;
	}

	printf("^%s^\n", buf);

	//DEBUG_COMMON
	strcpy(str, "DEBUG_COMMON=");

	if(strncmp(buf, str, strlen(str)) == 0) {
		flag = atoi(buf + strlen(str));

		if(flag == 1) {
			g_log_common = DEBUG_ENABLE_DEBUG;
		} else {
			g_log_common = DEBUG_ENABLE_NULL;
		}

		return ;
	}

	//	//DEBUG_CAPTURE
	//	strcpy(str,"DEBUG_CAPTURE=");
	//	if(strncmp(buf,str,strlen(str)) == 0)
	//	{
	//		flag = atoi(buf + strlen(str));
	//		if(flag == 1)
	//			g_log_capture = DEBUG_ENABLE_DEBUG;
	//		else
	//			g_log_capture = DEBUG_ENABLE_ERROR;
	//		return ;
	//	}

	//	//DEBUG_LOWVIDEO
	//	strcpy(str,"DEBUG_LOWVIDEO=");
	//	if(strncmp(buf,str,strlen(str)) == 0)
	//	{
	//		flag = atoi(buf + strlen(str));
	//		if(flag == 1)
	//			g_log_lowvideo = DEBUG_ENABLE_DEBUG;
	//		else
	//			g_log_lowvideo = DEBUG_ENABLE_ERROR;
	//		return ;
	//	}
}




int TRACE_Print(const char *fmt, ...)
{
	int len, l;
	char *base;
	va_list args;
	//#ifdef OPEN_CAMERA
	//	return ;
	//#endif


	if(g_mutex == NULL || g_log_buf == NULL) {
		printf("Warnning>>>>>>>>>>>>>>>>>>>>>>>>must init first\n");
		goto Err;
	}

	pthread_mutex_lock(&g_mutex->id);

	va_start(args, fmt);




	base = g_log_buf + g_log_off;

	if(g_newline_flg) {
		int n, m, s, us;
		us = mid_clock() ;
		s = us / 1000;
		n = s / 3600;
		s = s % 3600;
		m = s / 60;
		s = s % 60;
		us = us % 1000;

		if(strlen(g_test) != 0) {
			sprintf(g_log_buf + g_log_off, "%s[%04d:%02d:%02d:%03d] : ", g_test, n, m, s, us);
			g_log_off += (15 + 4 + strlen(g_test));
			l = (15 + 4 + strlen(g_test));
		} else {
			sprintf(g_log_buf + g_log_off, "[%04d:%02d:%02d:%03d] : ", n, m, s, us);
			g_log_off += (15 + 4);
			l = (15 + 4);
		}
	} else {
		l = 0;
	}

	g_newline_flg = 0;
	len = vsnprintf(g_log_buf + g_log_off, TRACE_MAXSIZE, fmt, args);

	if(len < 0) {
		len = TRACE_MAXSIZE;
		g_log_off += TRACE_MAXSIZE;
		g_log_buf[g_log_off - 1] = '\n';
		g_log_buf[g_log_off] = 0;
		g_newline_flg = 1;
	} else {
		g_log_off += len;

		if(g_log_buf[g_log_off - 1] == '\n') {
			g_newline_flg = 1;
		}
	}

	if(g_log_off >= TRACE_LOGSIZE) {
		g_log_end = g_log_off;
		g_log_off = 0;
	}

	if(g_debug_out == 0) {
		printf("%s", base);
	}


	va_end(args);

	pthread_mutex_unlock(&g_mutex->id);
	return len;
Err:
	return -1;
}

//获取毫秒级别
unsigned int mid_clock(void)
{

	//	unsigned int msec;
	//	struct timeval    tv;

	//	gettimeofday(&tv,NULL);

	//	msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	unsigned int msec;
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	msec = tp.tv_sec;
	msec = msec * 1000 + tp.tv_nsec / 1000000;

	return msec;
}





