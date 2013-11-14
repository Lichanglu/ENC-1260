#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ti/sdo/dmai/Dmai.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/Capture.h>
#include <ti/sdo/dmai/ColorSpace.h>
#include <ti/sdo/dmai/BufferGfx.h>

#include <ti/sdo/simplewidget/Screen.h>
#include <ti/sdo/simplewidget/Png.h>
#include "logo.h"
#include "common.h"
#include "log_common.h"
#include "sysparam.h"


extern PLock            gSetP_m;
static Png_Handle hPng = NULL;
pthread_mutex_t logomutex;




void app_png_show(int x, int y, Buffer_Handle hBuf, int isThrough, int Filt, int bl)
{
	//pthread_mutex_lock(&logomutex);
	if(hPng == NULL) {
		//pthread_mutex_unlock(&logomutex);
		PRINTF("Error\n");
		return ;
	}

	Png_showTransparency(hPng, x,  y, hBuf, isThrough, Filt, bl);
	//pthread_mutex_unlock(&logomutex);
	return ;
}




int ResetLogoinfo()
{
	LogoInfo info;
	memset(&info, 0, sizeof(info));
	WriteLogoinfo(LOGOCONFIGNAME, &info);
	return 0;
}
int ReadLogoinfo(const char *config_file, LogoInfo *logo)
{
	PRINTF("logo =%p\n", logo);
	char 			temp[512] = {0};
	int 			ret  = 0 ;
	int 			enable = 0;
	int 			rst = 0;
	pthread_mutex_lock(&gSetP_m.save_sys_m);

	ret =  ConfigGetKey(config_file, "logo", "x", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get logo x\n");
		goto EXIT;
	}

	logo->x = atoi(temp);
	ret =  ConfigGetKey(config_file, "logo", "y", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get logo y\n");
		goto EXIT;
	}

	logo->y = atoi(temp);
	ret =  ConfigGetKey(config_file, "logo", "enable", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get logo enable\n");
		goto EXIT;
	}

	logo->enable = atoi(temp);
	ret =  ConfigGetKey(config_file, "logo", "filename", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get logo filename\n");
		goto EXIT;
	}

	temp[15] = 0;
	strcpy(logo->filename, temp);
	ret =  ConfigGetKey(config_file, "logo", "alpha", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get logo alpha\n");
		goto EXIT;
	}

	logo->alpha = atoi(temp);
	ret =  ConfigGetKey(config_file, "logo", "isThrough", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get logo isThrough\n");
		goto EXIT;
	}

	logo->isThrough = atoi(temp);


	ret =  ConfigGetKey(config_file, "logo", "postype", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Get logo isThrough\n");
		goto EXIT;
	}

	logo->postype = atoi(temp);


	rst = 1;
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);

	return rst;



}

int WriteLogoinfo(const char *config_file, LogoInfo *logo)
{
	char 			temp[512] = {0};
	int 			ret  = 0 ;
	int 			enable = 0;
	int				rst = 0;
	pthread_mutex_lock(&gSetP_m.save_sys_m);

	sprintf(temp, "%d", logo->x);
	ret =  ConfigSetKey(config_file, "logo", "x", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Set logo x\n");
		goto EXIT;
	}

	sprintf(temp, "%d", logo->y);
	ret =  ConfigSetKey(config_file, "logo", "y", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Set logo y\n");
		goto EXIT;
	}

	sprintf(temp, "%d", logo->enable);
	ret =  ConfigSetKey(config_file, "logo", "enable", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Set logo enable\n");
		goto EXIT;
	}

	ret =  ConfigSetKey(config_file, "logo", "filename", logo->filename);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Set filename\n");
		goto EXIT;
	}

	sprintf(temp, "%d", logo->alpha);
	ret =  ConfigSetKey(config_file, "logo", "alpha", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Set alpha \n");
		goto EXIT;
	}

	sprintf(temp, "%d", logo->isThrough);
	ret =  ConfigSetKey(config_file, "logo", "isThrough", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Set isThrough \n");
		goto EXIT;
	}

	sprintf(temp, "%d", logo->postype);
	ret =  ConfigSetKey(config_file, "logo", "postype", temp);

	if(ret != 0) {
		DEBUG(DL_ERROR, "Failed to Set isThrough \n");
		goto EXIT;
	}


	rst = 1;
EXIT:
	pthread_mutex_unlock(&gSetP_m.save_sys_m);
	return rst;


}

Logo_Handle initLogoMod()
{
	Logo_Handle lh = NULL;
	int h, w;
	char filename[256] = {0};
	lh = calloc(1, sizeof(LogoInfo));

	if(lh == NULL) {
		DEBUG(DL_DEBUG, "Can not calloc LogoInfo == NULL!!\n");
		return NULL;
	}

	if(0 == ReadLogoinfo(LOGOCONFIGNAME, lh)) {
		lh->enable = 0;
		lh->errcode = 2;         //invaild show config
	} else {
		sprintf(filename, "%s", lh->filename);

		hPng = Png_create(filename);

		if(hPng) {
			Png_getWH(hPng, &w, &h);

			if(h > LOGOHEIGHT || w > LOGOWIDTH) {
				Png_delete(hPng);
				hPng = NULL;
			}
		}

		if(hPng == NULL) {
			lh->errcode = 1;       //invaild png file
			lh->enable = 0;
		};

		if(hPng) {
			lh->width = w;
			lh->height = h;
		}
	}

	pthread_mutex_init(&logomutex, NULL);
	return lh;

}

#if 0
void showLogo(Logo_Handle lh, Buffer_Handle hCapBuf, int mode)
{
	BufferGfx_Dimensions dim;
	int x, y;
	pthread_mutex_lock(&logomutex);

	if(lh->enable && hPng != NULL) {
		if(lh->enable && hPng != NULL) {
			BufferGfx_getDimensions(hCapBuf, &dim);
			x = lh->x;
			y = lh->y;

			if(x < 0) {
				x = 0;
			}

			if(y < 0) {
				y = 0;
			}

			if(mode == 29) {
				x = x * 688 / 720 + 16;
				y = y * 448 / 480 + 16;

				if(y > 464 - lh->height) {
					y = 464 - lh->height;
				}

				if(x > 704 - lh->width) {
					x = 704 - lh->width;
				}
			} else if(mode == 30) {
				x = x * 656 / 704 + 24;
				y = y * 528 / 576 + 24;

				if(y > 552 - lh->height) {
					y = 552 - lh->height;
				}

				if(x > 696 - lh->width) {
					x = 696 - lh->width;
				}
			} else {
				if(x > dim.width - lh->width) {
					x = dim.width - lh->width;
				}

				if(y > dim.height - lh->height) {
					y = dim.height - lh->height;
				}
			}

			Png_showTransparency(hPng, x, y, hCapBuf, lh->isThrough, 0, lh->alpha); //默认过滤掉黑色
		}
	}

	pthread_mutex_unlock(&logomutex);
}
#endif

//error code -4 invaild file -3 size to big
int checklogo(char *filename)
{
	int error_code = 0;
	int w = 0;
	int h = 0;
	Png_Handle temp = NULL;
	temp = Png_create(filename);

	if(temp == NULL) {
		error_code = -4;
	}

	if(temp) {
		Png_getWH(temp, &w, &h);

		if(h > LOGOHEIGHT || w > LOGOWIDTH) {
			error_code = -3;
		}
	}

	if(temp != NULL) {
		Png_delete(temp);
	}

	return error_code;

}

int  reloadLogo(Logo_Handle lh)
{
	int w, h;
	char filename[256] = {0};
	pthread_mutex_lock(&logomutex);

	if(hPng != NULL) {
		Png_delete(hPng);
	}

	sprintf(filename, "%s", lh->filename);
	hPng = Png_create(filename);

	if(hPng) {
		Png_getWH(hPng, &w, &h);

		if(h > LOGOHEIGHT || w > LOGOWIDTH) {
			Png_delete(hPng);
			hPng = NULL;
		}
	}

	if(hPng == NULL) {
		lh->errcode = 1;
		lh->enable = 0;
	};

	if(hPng) {
		lh->width = w;
		lh->height = h;
	}

	pthread_mutex_unlock(&logomutex);
}

#if 0
int setLogoParm(Logo_Handle lh, unsigned char *buf)
{
	LogoInfo logo;
	int ret = 1; //成功1，失败0
	memcpy(&logo, buf, sizeof(LogoInfo));

	if(logo.alpha > 100 || logo.alpha < 0 || logo.x < 0 || logo.x > 1920 || logo.y < 0 || logo.y > 1080 || logo.isThrough > 3 || logo.isThrough < 0) {
		return 0;
	}

	logo.filename[15] = 0;
	strcpy(lh->filename, logo.filename);
	lh->x = logo.x;
	lh->y = logo.y;
	lh->alpha = logo.alpha;
	lh->enable = logo.enable;
	lh->filter = logo.filter;
	lh->isThrough = logo.isThrough;
	WriteLogoinfo(LOGOCONFIGNAME, lh);
	return ret;
}


void delLogoMod(Logo_Handle lh)
{
	pthread_mutex_lock(&logomutex);

	if(hPng) {
		Png_delete(hPng);
	}

	pthread_mutex_unlock(&logomutex);

	if(lh) {
		free(lh);
	}
}


/*升级png图片过程*/
int UploadImage(int sSocket, char *data, unsigned char *logoname)
{
	unsigned long filesize;
	unsigned char *p;
	int nLen;
	unsigned char temp[20];
	unsigned char filename[20];
	int ret;
	FILE *fp = NULL;
	//Logo_Handle lhd = NULL;
	p = NULL;

	p = malloc(8 * 1024);
	temp[0] = 0;
	temp[1] = 3;
	temp[2] = 1;
	temp[3] = 1;
	DEBUG(DL_DEBUG, "The File Updata Buffer Is 8KB!\n");

	if(!p) {
		DEBUG(DL_ERROR, "Malloc 8KB Buffer Failed!\n");
	} else {
		DEBUG(DL_DEBUG, "The Buffer Addr 0x%x\n", (unsigned int)p);
	}

	sprintf(filename, "%s", logoname);
	nLen = recv(sSocket, data, 4, 0);
	DEBUG(DL_DEBUG, "nLennLen=%d!\n", nLen);

	if(nLen < 4) {
		free(p);
		return 0;
	}

	filesize = ((unsigned char)data[0]) | ((unsigned char)data[1]) << 8 |
	           ((unsigned char)data[2]) << 16 | ((unsigned char)data[3]) << 24;
	DEBUG(DL_DEBUG, "Updata file size = 0x%x! \n", (unsigned int)filesize);

	if(filesize > 10 * 1024) {
		free(p);
		return 0;
	}

	//if(strcpy(filename,"logo.png")){
	//free(p);
	//return 0;
	//}
	if((fp = fopen(filename, "w+")) == NULL) {
		DEBUG(DL_ERROR, "open %s error \n", filename);
		return 0;
	}

	while(filesize) {
		nLen = recv(sSocket, p, 8 * 1024, 0);

		if(nLen < 1) {
			return nLen;
		}

		DEBUG(DL_DEBUG, "recv Updata File 0x%x Bytes!\n", nLen);
		ret = fwrite(p, 1, nLen, fp);

		if(ret < 0) {
			DEBUG(DL_ERROR, "product update faile!\n");
			free(p);
			fclose(fp);
			p = NULL;
			temp[3] = 0;
			WriteData(sSocket, temp, 12);
			return 0;
		}

		DEBUG(DL_DEBUG, "write data ...........................:0x%x\n", (unsigned int)filesize);
		filesize = filesize - nLen;
	}

	DEBUG(DL_DEBUG, "recv finish......................\n");
	free(p);
	fclose(fp);
	p = NULL;
	DEBUG(DL_DEBUG, "upload image succeed !! \n");
	system("sync");
	return 1;
}

#endif

