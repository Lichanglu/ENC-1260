/*********************************************************************************
*             设置保存绿屏相关的函数
*
*														add by zm
*																2011-2-22
**********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if.h>
#include <ctype.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "kernel_common.h"
#include "common.h"
#include "log_common.h"

#ifndef PRINTF
#define PRINTF printf
#endif

#define MAX_RES_STRING_LEN  18  //strlen("[1920x1080Ix60]")+4
typedef struct GREEN_TABLE_T {
	char res_string[MAX_RES_STRING_LEN];
	int res;
	unsigned int digital_val;
	unsigned int analog_val;
} GREEN_TABLE_S;

static GREEN_TABLE_S g_green_table[APP_VIDEO_MAX_COUNT] = {
	{"640x480x60",		APP_VGA_640X480X60 ,		1, 1},
	{"640x480x72",		APP_VGA_640X480X72 ,		1, 1},
	{"640x480x75",		APP_VGA_640X480X75 ,		1, 1},
	{"640x480x85",		APP_VGA_640X480X85 ,		1, 1},

	{"800x600x60",		APP_VGA_800X600X60 ,		1, 1},
	{"800x600x72",		APP_VGA_800X600X72 ,		1, 1},
	{"800x600x75",		APP_VGA_800X600X75 ,		1, 1},
	{"800x600x85",		APP_VGA_800X600X85 ,		1, 1},
	{"1024x768x60",		APP_VGA_1024X768X60 ,		1, 1},
	{"1024x768x72",		APP_VGA_1024X768X72 ,		1, 1},
	{"1024x768x75",		APP_VGA_1024X768X75 ,		1, 1},
	{"1024x768x85",		APP_VGA_1024X768X85 ,		1, 1},
	{"1280x768x60",		APP_VGA_1280X768X60 ,		1, 1},
	{"1280x800x60",		APP_VGA_1280X800X60 ,		1, 1},
	{"1280x960x60",		APP_VGA_1280X960X60 ,		1, 1},
	{"1440x900x60",		APP_VGA_1440X900X60 ,		1, 1},
	{"1400x1050x60",	APP_VGA_1400X1050X60 ,		1, 1},
	{"1280x1024x60",	APP_VGA_1280X1024X60 ,		1, 1},
	{"1280x1024x75",	APP_VGA_1280X1024X75 ,		1, 1},
	{"1600x1200x60",	APP_VGA_1600X1200X60 ,		1, 1},
	{"1280x720Vx60",	APP_VGA_1280X720VX60 ,		1, 1},
	{"1280x720Px50",	APP_VIDEO_1280X720X50P ,		1, 0},
	{"1280x720Px60",	APP_VIDEO_1280X720X60P ,		1, 0},
	{"1920x1080Ix50",	APP_VIDEO_1920X1080X50I ,	1, 0},
	{"1920x1080Ix60",	APP_VIDEO_1920X1080X60I ,	1, 0},
	{"1920x1080Px25",	APP_VIDEO_1920X1080X25P ,	1, 0},
	{"1920x1080Px30",	APP_VIDEO_1920X1080X30P ,	1, 0},
	{"1920x1080Px50",	APP_VIDEO_1920X1080X50P ,	1, 0},
	{"1920x1080Px60",	APP_VIDEO_1920X1080X60P ,	1, 0},
	{"720x480Ix60",		APP_VIDEO_720X480X60I ,		1, 0},
	{"720x576Ix50",		APP_VIDEO_720X576X50I ,		1, 0},
	{"720x480px60",		APP_VIDEO_720X480X60P ,		1, 0},
	{"720x576px50",		APP_VIDEO_720X576X50P ,		1, 0},
	{"1366x768x60",		APP_VGA_1366X768X60 ,		1, 0},
};

static GREEN_TABLE_S g_screen_table[APP_VIDEO_MAX_COUNT] = {
	{"640x480x60",		APP_VGA_640X480X60 ,		1, 1},
	{"640x480x72",		APP_VGA_640X480X72 ,		1, 1},
	{"640x480x75",		APP_VGA_640X480X75 ,		1, 1},
	{"640x480x85",		APP_VGA_640X480X85 ,		1, 1},

	{"800x600x60",		APP_VGA_800X600X60 ,		1, 1},
	{"800x600x72",		APP_VGA_800X600X72 ,		1, 1},
	{"800x600x75",		APP_VGA_800X600X75 ,		1, 1},
	{"800x600x85",		APP_VGA_800X600X85 ,		1, 1},
	{"1024x768x60",		APP_VGA_1024X768X60 ,		1, 1},
	{"1024x768x72",		APP_VGA_1024X768X72 ,		1, 1},
	{"1024x768x75",		APP_VGA_1024X768X75 ,		1, 1},
	{"1024x768x85",		APP_VGA_1024X768X85 ,		1, 1},
	{"1280x768x60",		APP_VGA_1280X768X60 ,		1, 1},
	{"1280x800x60",		APP_VGA_1280X800X60 ,		1, 1},
	{"1280x960x60",		APP_VGA_1280X960X60 ,		1, 1},
	{"1440x900x60",		APP_VGA_1440X900X60 ,		1, 1},
	{"1400x1050x60",	APP_VGA_1400X1050X60 ,		1, 1},
	{"1280x1024x60",	APP_VGA_1280X1024X60 ,		1, 1},
	{"1280x1024x75",	APP_VGA_1280X1024X75 ,		1, 1},
	{"1600x1200x60",	APP_VGA_1600X1200X60 ,		1, 1},
	{"1280x720Vx60",	APP_VGA_1280X720VX60 ,		1, 1},
	{"1280x720Px50",	APP_VIDEO_1280X720X50P ,		1, 1},
	{"1280x720Px60",	APP_VIDEO_1280X720X60P ,		1, 1},
	{"1920x1080Ix50",	APP_VIDEO_1920X1080X50I ,	1, 1},
	{"1920x1080Ix60",	APP_VIDEO_1920X1080X60I ,	1, 1},
	{"1920x1080Px25",	APP_VIDEO_1920X1080X25P ,	1, 1},
	{"1920x1080Px30",	APP_VIDEO_1920X1080X30P ,	1, 1},
	{"1920x1080Px50",	APP_VIDEO_1920X1080X50P ,	1, 1},
	{"1920x1080Px60",	APP_VIDEO_1920X1080X60P ,	1, 1},
	{"720x480Ix60",		APP_VIDEO_720X480X60I ,		1, 1},
	{"720x576Ix50",		APP_VIDEO_720X576X50I ,		1, 1},
	{"720x480px60",		APP_VIDEO_720X480X60P ,		1, 1},
	{"720x576px50",		APP_VIDEO_720X576X50P ,		1, 1},
	{"1366x768x60",		APP_VGA_1366X768X60 ,		1, 1},
};


#define GREEN_ADJUST_FILE "green_adjust.ini"
static void read_green_adjust_table()
{
	int i = 0;
	int ret = 0;
	char temp[512] = {0};
	int val = 0;
	char config_file[64] = GREEN_ADJUST_FILE;

	for(i = 0; i < (APP_VIDEO_MAX_COUNT); i++) {
		if(g_green_table[i].res_string == NULL || strlen(g_green_table[i].res_string) == 0) {
			PRINTF("ERROR! the table %d is error.\n", i);
			break;
		}

		memset(temp, 0, sizeof(temp));
		ret = ConfigGetKey(config_file, g_green_table[i].res_string, "digitalvalue", temp); // 0

		if(ret == 0 && strlen(temp) == 1) {
			val = atoi(temp);

			if(val >= 0) {
				g_green_table[i].digital_val = val;
			}

			//PRINTF("i read the green adjust success,the %s digital_val= %d\n",g_green_table[i].res_string,g_green_table[i].digital_val);
		} else {
			;//PRINTF("ERROR,i read the green adjust failed ,read digital value the string is %s.\n",g_green_table[i].res_string);
		}


		memset(temp, 0, sizeof(temp));
		ret = ConfigGetKey(config_file, g_green_table[i].res_string, "analogvalue", temp); // 0

		if(ret == 0 && strlen(temp) == 1) {
			val = atoi(temp);

			if(val >= 0) {
				g_green_table[i].analog_val = val;
			}

			//PRINTF("i read the green adjust success,the %s ,nalogvalue= %d\n",g_green_table[i].res_string,g_green_table[i].analog_val);
		} else {
			;//PRINTF("ERROR,i read the green adjust failed ,read analog the string is %s.\n",g_green_table[i].res_string);
		}
	}

	return ;
}

#define SCREEN_ADJUST_FILE "screen_adjust.ini"
static void read_screen_adjust_table()
{
	int i = 0;
	int ret = 0;
	char temp[512] = {0};
	int val = 0;
	char config_file[64] = SCREEN_ADJUST_FILE;

	for(i = 0; i < (APP_VIDEO_MAX_COUNT); i++) {
		if(g_screen_table[i].res_string == NULL || strlen(g_screen_table[i].res_string) == 0) {
			PRINTF("ERROR! the table %d is error.\n", i);
			break;
		}

		memset(temp, 0, sizeof(temp));
		ret = ConfigGetKey(config_file, g_screen_table[i].res_string, "digitalvalue", temp); // 0

		if(ret == 0 && strlen(temp) == 1) {
			val = atoi(temp);

			if(val >= 0) {
				g_screen_table[i].digital_val = val;
			}

			//PRINTF("i read the green adjust success,the %s digital_val= %d\n",g_green_table[i].res_string,g_green_table[i].digital_val);
		} else {
			;//PRINTF("ERROR,i read the green adjust failed ,read digital value the string is %s.\n",g_green_table[i].res_string);
		}


		memset(temp, 0, sizeof(temp));
		ret = ConfigGetKey(config_file, g_screen_table[i].res_string, "analogvalue", temp); // 0

		if(ret == 0 && strlen(temp) == 1) {
			val = atoi(temp);

			if(val >= 0) {
				g_screen_table[i].analog_val = val;
			}

			//PRINTF("i read the green adjust success,the %s ,nalogvalue= %d\n",g_green_table[i].res_string,g_green_table[i].analog_val);
		} else {
			;//PRINTF("ERROR,i read the green adjust failed ,read analog the string is %s.\n",g_green_table[i].res_string);
		}
	}

	return ;
}

static void save_green_adjust_table()
{
	int i = 0;
	int ret = 0;
	char temp[512] = {0};
	//int val = 0;
	char config_file[512] = GREEN_ADJUST_FILE;

	for(i = 0; i < (APP_VIDEO_MAX_COUNT); i++) {
		if(g_green_table[i].res_string == NULL || strlen(g_green_table[i].res_string) == 0) {
			PRINTF("ERROR! the table %d is error.\n", i);
			break;
		}

		sprintf(temp, "%d", g_green_table[i].digital_val);
		ret = ConfigSetKey(config_file, g_green_table[i].res_string, "digitalvalue", temp);

		if(ret != 0) {
			PRINTF("ERROR!save the file is failed,the digital string is %s .\n", g_green_table[i].res_string);
			break;
		}

		sprintf(temp, "%d", g_green_table[i].analog_val);
		ret = ConfigSetKey(config_file, g_green_table[i].res_string, "analogvalue", temp);

		if(ret != 0) {
			PRINTF("ERROR!save the file is failed,the analog string is %s .\n", g_green_table[i].res_string);
			break;
		}
	}


	sync();
	return ;

}

static void save_screen_adjust_table()
{
	int i = 0;
	int ret = 0;
	char temp[512] = {0};
	//int val = 0;
	char config_file[512] = SCREEN_ADJUST_FILE;

	for(i = 0; i < (APP_VIDEO_MAX_COUNT); i++) {
		if(g_screen_table[i].res_string == NULL || strlen(g_screen_table[i].res_string) == 0) {
			PRINTF("ERROR! the table %d is error.\n", i);
			break;
		}

		sprintf(temp, "%d", g_screen_table[i].digital_val);
		ret = ConfigSetKey(config_file, g_screen_table[i].res_string, "digitalvalue", temp);

		if(ret != 0) {
			PRINTF("ERROR!save the file is failed,the digital string is %s .\n", g_screen_table[i].res_string);
			break;
		}

		sprintf(temp, "%d", g_screen_table[i].analog_val);
		ret = ConfigSetKey(config_file, g_screen_table[i].res_string, "analogvalue", temp);

		if(ret != 0) {
			PRINTF("ERROR!save the file is failed,the analog string is %s .\n", g_screen_table[i].res_string);
			break;
		}
	}


	sync();
	return ;

}

void app_init_green_adjust_module()
{
	if(access(GREEN_ADJUST_FILE, F_OK) != 0) {
		save_green_adjust_table();
	}

	read_green_adjust_table();
}

void app_init_screen_adjust_module()
{
	if(access(SCREEN_ADJUST_FILE, F_OK) != 0) {
		save_screen_adjust_table();
	}

	read_screen_adjust_table();
}

void app_set_green_adjust_value(int digval, int mode, int value)
{
	if(mode >= APP_VIDEO_MAX_COUNT || digval > 1) {
		PRINTF("ERROR!!!app_set_green_adjust_value \n");
		return ;
	}

	int writeflag = 0;

	if(digval == 0 && g_green_table[mode].analog_val != value) {
		writeflag = 1;
		g_green_table[mode].analog_val = value;
	} else if(digval == 1 && (g_green_table[mode].digital_val != value)) {
		writeflag = 1;
		g_green_table[mode].digital_val = value;
	}

	if(writeflag == 1) {
		save_green_adjust_table();
	}
}

void app_set_screen_adjust_value(int digval, int mode, int value)
{
	if(mode >= APP_VIDEO_MAX_COUNT || digval > 1) {
		PRINTF("ERROR!!!app_set_green_adjust_value \n");
		return ;
	}

	int writeflag = 0;

	if(digval == 0 && g_screen_table[mode].analog_val != value) {
		writeflag = 1;
		g_screen_table[mode].analog_val = value;
	} else if(digval == 1 && (g_screen_table[mode].digital_val != value)) {
		writeflag = 1;
		g_screen_table[mode].digital_val = value;
	}

	if(writeflag == 1) {
		save_screen_adjust_table();
	}
}

int app_get_green_adjust_value(int digval, int mode, int *value)
{
	if(mode >= APP_VIDEO_MAX_COUNT || digval > 1) {
		PRINTF("ERROR!!!app_get_green_adjust_value \n");
		return -1;
	}

	if(digval == 0) {
		*value = g_green_table[mode].analog_val ;
	} else if(digval == 1) {
		*value = g_green_table[mode].digital_val ;
	}

	PRINTF("digval=%d,mode=%d,*vlaue=%d\n", digval, mode, *value);
	return 0;
}

int app_get_screen_adjust_value(int digval, int mode, int *value)
{
	if(mode >= APP_VIDEO_MAX_COUNT || digval > 1) {
		PRINTF("ERROR!!!app_get_green_adjust_value \n");
		return -1;
	}

	if(digval == 0) {
		*value = g_screen_table[mode].analog_val ;
	} else if(digval == 1) {
		*value = g_screen_table[mode].digital_val ;
	}

	return 0;
}

