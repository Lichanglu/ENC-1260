#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include "netcom.h"
#include "common.h"
#include "sysparam.h"
#ifdef DSS_ENC_1100_1200
extern PLock  gSetP_m;

#if (defined(DSS_ENC_1200) || defined(DSS_ENC_1260))
char					gDeviceType[16] = {"ENC1260"};
#else
char					gDeviceType[16] = {"ENC1100"};
#endif

void GetDeviceType(char *dtype)
{
	strcpy(dtype, gDeviceType);
}

int SetDeviceType(char *config_file, const char *dtype)
{
	char 			    temp[512];
	char 			    cmd[1124];
	int 				ret = 0;

	pthread_mutex_lock(&gSetP_m.save_dtype_m);
	memset(temp, 0, 512);
	sprintf(temp, "%s", dtype);
	ret =  ConfigSetKey(config_file, "sysParam", "dtype", temp);

	if(ret != 0) {
		DEBUG(DL_FLOW, "Set Key devidetype failed\n");
		pthread_mutex_unlock(&gSetP_m.save_dtype_m);
		return -1;
	}

	pthread_mutex_unlock(&gSetP_m.save_dtype_m);

	strncpy(gDeviceType, dtype, 16);

	memset(cmd, 0, 256);
	sprintf(cmd, "cp %s %s.bak", config_file, config_file);
	DEBUG(DL_FLOW, "cmd = %s\n", cmd);
	system(cmd);
	return 0;
}



int ReadDeviceType(char *config_file, int againFlag)
{
	int 			ret = 0;
	char 		    temp[512];
	char			cmd[1124];

	pthread_mutex_lock(&gSetP_m.save_dtype_m);

	/*DeviceType*/
	memset(temp, 0, 512);
	ret =  ConfigGetKey(config_file, "sysParam", "dtype", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Get Key devidetype failed\n");
		goto EXIT;
	}

	DEBUG(DL_DEBUG, "devidetype = %s\n", temp);
	strncpy(gDeviceType, temp, 16);
	pthread_mutex_unlock(&gSetP_m.save_dtype_m);
	return 0;

EXIT:

	if(0 == againFlag) {
		pthread_mutex_unlock(&gSetP_m.save_dtype_m);
		return -1;
	}

	sprintf(cmd, "cp %s.bak %s", config_file, config_file);
	system(cmd);
	pthread_mutex_unlock(&gSetP_m.save_dtype_m);

	return -2;
}
#endif

