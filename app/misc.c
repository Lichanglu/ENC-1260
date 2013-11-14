#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <xdc/std.h>
#include <ti/sdo/dmai/ce/Venc1.h>
/* Include codec specific header file for extended params */
#include <ti/sdo/codecs/h264fhdvenc/ih264fhdvenc.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/simplewidget/Font.h>
#include <ti/sdo/simplewidget/Text.h>
#include <ti/sdo/simplewidget/Screen.h>
#include <ti/sdo/dmai/Blend.h>
#include <iconv.h>
#include <errno.h>

#include "demo.h"
#include "common.h"
#include "tcpcom.h"
#include "log_common.h"
#include "sysparam.h"

static Char *aacExtensions[] = { ".aac", NULL };
/* NULL terminated list of audio encoders in the engine to use in the demo */
static Codec audioEncoders[] = {
	{
		"aacheenc",
		"AAC LC",
		aacExtensions,
		NULL,
		NULL

	},
	{ NULL }
};

/* Use extended dynamic parameters to allow tweaking of the QP value */
static IH264FHDVENC_DynamicParams extDynParams = {
	{
		sizeof(IH264FHDVENC_DynamicParams),   /* size */
		720,                              /* inputHeight */
		1280,                             /* inputWidth */
		30000,                            /* refFrameRate */
		30000,                            /* targetFrameRate */
		6000000,                          /* targetBitRate (override in app) */
		30,                               /* intraFrameInterval */
		XDM_DECODE_AU,                    /* generateHeader */
		0,                                /* captureWidth */
		IVIDEO_NA_FRAME,                  /* forceFrame */
		1,                                /* interFrameInterval */
		0                                 /* mbDataFlag */
	},
	{IH264_SLICECODING_DEFAULT},
	18,   /* QPISlice */
	20,   /* QPSlice */
	51,   /* RateCtrlQpMax */
	0,    /* RateCtrlQpMin */
	0,    /* NumRowsInSlice */
	0,    /* LfDisableIdc */
	0,    /* LFAlphaC0Offset */
	0,    /* LFBetaOffset */
	0,    /* ChromaQPOffset */
	0,    /* SecChromaQPOffset */
};

/* File extensions for H.264 */
static Char *h264Extensions[] = { ".264", NULL };

/* NULL terminated list of video encoders in the engine to use in the demo */
static Codec videoEncoders[] = {
	{
		"h264fhdvenc",
		"H.264 BP",
		h264Extensions,
		NULL,
		&extDynParams
	},
	{ NULL }
};


/* Declaration of the production engine and encoders shipped with the DVSDK */
static Engine encodeEngine = {
	"encode",           /* Engine string name used by CE to find the engine */
	NULL,               /* NULL terminated list of speech decoders in engine */
	NULL,               /* NULL terminated list of audio decoders in engine */
	NULL,               /* NULL terminated list of video decoders in engine */
	NULL,   			/* NULL terminated list of speech encoders in engine */
	audioEncoders,      /* NULL terminated list of audio encoders in engine */
	videoEncoders       /* NULL terminated list of video encoders in engine */
};

Engine *engine = &encodeEngine;


/*添加OSD相关*/
#define MSG_ADD_TEXT    33  //加入字幕
#define TEXT_BUFTYPE_NUMTYPES  2
#define TEXT_BITMAP_WIDTH		8 * 4 * 50
#define TEXT_BITMAP_HEIGHT		8 * 4 * 2

#define TIME_BITMAP_WIDTH		8 * 4 * 13
#define TIME_BITMAP_HEIGHT		8 * 4 * 2

#define TIME_XPOS		(32*2)
#define TIME_YPOS		(32*2)
#define OSD_FONT_SIZE		23
#define OSD_TRANS_VALUE   0xFF

typedef enum {
    TEXT_BufType_WORK = 0,
    TEXT_BufType_DISPLAY,
    TEXT_BufType_NUMTYPES
} TEXT_Buftype;

/* The palette to use while blending the bitmap */
/*
static Int8 palette[4][4] = {
    { 0x00, 0x00, 0x00, 0x00 },
    { 0x80, 0x80, 0xf0, 0x00 },
    { 0x80, 0x80, 0xf8, 0x00 },
    { 0x80, 0x80, 0xff, 0x00 },
};
*/
static Int8 palette[4][4] = {
	{ 0x80, 0x80, 0x01, 0xff },
	{ 0x80, 0x80, 0xf0, 0xff },
	{ 0x80, 0x80, 0xf8, 0xff },
	{ 0x80, 0x80, 0xff, 0xff },
};


static int display_text = 0;
static int g_display_time = 0;
typedef struct ___TextOsd_Object {
	Buffer_Handle       hBufs[TEXT_BUFTYPE_NUMTYPES];
	pthread_mutex_t     mutex;
	Int                 displayIdx;
	Int                 workingIdx;
} TextOsd_Object;

#define CHN_FONT_NAME "data/fonts/simkai.ttf"
/*OSD 相关参数*/
static	Blend_Handle					hTextBlend          = NULL;
static 	Blend_Handle					hTimeBlend          = NULL;
static 	TextOsd_Object					textOsd, timeOsd;
static  Font_Handle						chnfont 			= NULL;
//static char test1[1024] = {0};
static int 								textXpos 			= 0;
static int 								textYpos			= 0;
static int								g_text_width = 0;
//static char test2[1024] = {0};
textinfo text;
extern int capture_width, capture_height;

int setDisplayTextFlag(int flag)
{
	display_text = flag;
	return 0;
}
int setDisplayTimeFlag(int flag)
{
	g_display_time = flag;
	return 0;
}
static inline int checkCHN(char *text, int size)
{
	int i;

	for(i = 0; i < size; i++)
		if(text[i] > 127) {
			return 1;
		}

	return 0;
}

/*代码转换:从一种编码转为另一种编码*/
static inline int code_convert(char *from_charset, char *to_charset, char *inbuf, int inlen, char *outbuf, int outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;
	cd = iconv_open(to_charset, from_charset);

	if((iconv_t) - 1 == cd) {
		DEBUG(DL_ERROR, "open the char set failed,errno = %d,strerror(errno) = %s \n", errno, strerror(errno));
		return -1;
	}

	memset(outbuf, 0, outlen);

	if(iconv(cd, pin, (size_t *)&inlen, pout, (size_t *)&outlen) == (size_t) - 1) {
		DEBUG(DL_ERROR, "conver char set failed,errno = %d,strerror(errno) = %s \n", errno, strerror(errno));
		return -1;
	}

	iconv_close(cd);
	return 0;
}


/*OSD*/
static void drawTextBuffer(const char *text, int len)
{
	BufferGfx_Dimensions dim;

	Screen_clear(textOsd.hBufs[textOsd.workingIdx], 0, 0, TEXT_BITMAP_WIDTH, TEXT_BITMAP_HEIGHT);
	BufferGfx_getDimensions(textOsd.hBufs[textOsd.workingIdx], &dim);

	if(len != 0) {
		dim.width  =  16 * (len);
	}

	if(dim.width > TEXT_BITMAP_WIDTH) {
		dim.width = TEXT_BITMAP_WIDTH;
	}

	g_text_width = dim.width;
	PRINTF("dim.width = %d,len=%d\n", dim.width, len);
	BufferGfx_setDimensions(textOsd.hBufs[textOsd.workingIdx], &dim);
	DEBUG(DL_DEBUG, "draw Text buffer =%s , len = %d\n", text, len);
	Text_show(chnfont, (char *)text, OSD_FONT_SIZE, 32, 32, textOsd.hBufs[textOsd.workingIdx]);
	pthread_mutex_lock(&textOsd.mutex);
	textOsd.workingIdx = (textOsd.workingIdx + 1) % TEXT_BUFTYPE_NUMTYPES;
	textOsd.displayIdx = (textOsd.displayIdx + 1) % TEXT_BUFTYPE_NUMTYPES;
	pthread_mutex_unlock(&textOsd.mutex);
}


static void drawTimeBuffer(void)
{
	char  text[128] = {0};
	DATE_TIME_INFO info;
	getSysTime(&info);
	sprintf(text, "%04d/%02d/%02d %02d:%02d:%02d", info.year, info.month, info.mday, info.hours, info.min, info.sec);
	Screen_clear(timeOsd.hBufs[timeOsd.workingIdx], 0, 0, TEXT_BITMAP_WIDTH, TEXT_BITMAP_HEIGHT);
	Text_show(chnfont, text, 30, 32, 32, timeOsd.hBufs[timeOsd.workingIdx]);
	pthread_mutex_lock(&timeOsd.mutex);
	timeOsd.workingIdx = (timeOsd.workingIdx + 1) % TEXT_BUFTYPE_NUMTYPES;
	timeOsd.displayIdx = (timeOsd.displayIdx + 1) % TEXT_BUFTYPE_NUMTYPES;
	pthread_mutex_unlock(&timeOsd.mutex);
}

/*创建OSD*/
int CreateTextOsdBuffer(void)
{
	Blend_Attrs			blAttrs         = Blend_Attrs_DEFAULT;
	Int                 bufSize;
	BufferGfx_Attrs     gfxAttrs      = BufferGfx_Attrs_DEFAULT;
	pthread_mutex_init(&textOsd.mutex, NULL);
	pthread_mutex_init(&timeOsd.mutex, NULL);

	hTextBlend  = Blend_create(&blAttrs);

	if(hTextBlend == NULL) {
		DEBUG(DL_ERROR, "Failed to create blending job\n");
		return -1;
	}

	hTimeBlend  = Blend_create(&blAttrs);

	if(hTimeBlend == NULL) {
		DEBUG(DL_ERROR, "Failed to create blending job\n");
		return -1;
	}

	chnfont = Font_create(CHN_FONT_NAME);

	if(chnfont == NULL) {
		DEBUG(DL_ERROR, "Failed to create font %s\n", CHN_FONT_NAME);
		return -1;
	}

	textOsd.workingIdx = 0;
	textOsd.displayIdx = 1;
	timeOsd.workingIdx = 0;
	timeOsd.displayIdx = 1;
	/*创建text的Buffer*/
	gfxAttrs.dim.width      = TEXT_BITMAP_WIDTH;
	gfxAttrs.dim.height     = TEXT_BITMAP_HEIGHT;
	gfxAttrs.dim.x = 0 ;
	gfxAttrs.dim.y = 0;
	gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(TEXT_BITMAP_WIDTH, ColorSpace_2BIT);
	gfxAttrs.colorSpace     = ColorSpace_2BIT;
	bufSize = gfxAttrs.dim.lineLength * gfxAttrs.dim.height;
	textOsd.hBufs[TEXT_BufType_WORK] = Buffer_create(bufSize, (Buffer_Attrs *) &gfxAttrs);

	if(textOsd.hBufs[TEXT_BufType_WORK] == NULL) {
		ERR("Failed to allocate bitmap buffer of size %d\n", bufSize);
		return -1;
	}

	Screen_clear(textOsd.hBufs[TEXT_BufType_WORK], 0, 0, TEXT_BITMAP_WIDTH, TEXT_BITMAP_HEIGHT);
	textOsd.hBufs[TEXT_BufType_DISPLAY] = Buffer_create(bufSize, (Buffer_Attrs *) &gfxAttrs);

	if(textOsd.hBufs[TEXT_BufType_DISPLAY] == NULL) {
		ERR("Failed to allocate bitmap buffer of size %d\n", bufSize);
		return -1;
	}

	Screen_clear(textOsd.hBufs[TEXT_BufType_DISPLAY], 0, 0, TEXT_BITMAP_WIDTH, TEXT_BITMAP_HEIGHT);
	/*创建timer的Buffer*/
	gfxAttrs.dim.width      = TIME_BITMAP_WIDTH;
	gfxAttrs.dim.height     = TIME_BITMAP_HEIGHT;
	gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(TIME_BITMAP_WIDTH, ColorSpace_2BIT);
	gfxAttrs.colorSpace     = ColorSpace_2BIT;
	bufSize = gfxAttrs.dim.lineLength * gfxAttrs.dim.height;
	timeOsd.hBufs[TEXT_BufType_WORK] = Buffer_create(bufSize, (Buffer_Attrs *) &gfxAttrs);

	if(timeOsd.hBufs[TEXT_BufType_WORK] == NULL) {
		ERR("Failed to allocate bitmap buffer of size %d\n", bufSize);
		return -1;
	}

	Screen_clear(timeOsd.hBufs[TEXT_BufType_WORK], 0, 0, TIME_BITMAP_WIDTH, TIME_BITMAP_HEIGHT);
	timeOsd.hBufs[TEXT_BufType_DISPLAY] = Buffer_create(bufSize, (Buffer_Attrs *) &gfxAttrs);

	if(timeOsd.hBufs[TEXT_BufType_DISPLAY] == NULL) {
		ERR("Failed to allocate bitmap buffer of size %d\n", bufSize);
		return -1;
	}

	Screen_clear(timeOsd.hBufs[TEXT_BufType_DISPLAY], 0, 0, TIME_BITMAP_WIDTH, TIME_BITMAP_HEIGHT);
	return 0;
}

/*添加时间字幕*/
void drawtimebufferThread(void *pParam)
{
	PRINTF("get pid= %d\n", getpid());

	while(!gblGetQuit()) {
		if(g_display_time) {
			drawTimeBuffer();
		}

		usleep(1000000);
	}
}

/*添加字幕和时间*/
int AddOsdText(int num, unsigned char *data, int len)
{
	RecAVTitle recavtitle;

	if(gblGetOSDStatus() == 0) { //no font file
		return -1;
	}

	memset(&recavtitle, 0x00000, sizeof(RecAVTitle));
	memcpy(&recavtitle, data, len);
	DEBUG(DL_DEBUG, "display the text =%s \n", recavtitle.Text);
	g_display_time = 0;

	if(recavtitle.len > 0) {

		int len = GetTextLen(recavtitle.Text);
		int size_len = GetSizeLen(recavtitle.Text);
		int charlen = strlen(recavtitle.Text);

		if(len != 0) {
			display_text = 1;
		} else {
			display_text = 0;
		}

		if(checkCHN(recavtitle.Text, charlen)) {
			char temp[512] = {0};
			code_convert("gb2312", "utf-32", recavtitle.Text, charlen, temp, charlen * 4);
			drawTextBuffer(temp, size_len);
		} else {
			drawTextBuffer(recavtitle.Text, size_len);
		}

		if(len != recavtitle.len) {
			DEBUG(DL_DEBUG, "display the time \n");
			drawTimeBuffer();
			g_display_time = 1;
		}

		textXpos = recavtitle.x;
		textYpos = recavtitle.y;

		if(textXpos % 32) {
			textXpos -= (textXpos % 32);
		}

		PRINTF("textXpos=%d\n", textXpos);

		if(textYpos % 32) {
			textYpos -= (textYpos % 32);
		}

		DEBUG(DL_DEBUG, "textXpos = %d  textYpos = %d\n\n", textXpos, textYpos);

		if((H264Height() - textYpos) < 64) {
			textYpos = H264Height() - 64;
		}
	} else {
		display_text = 0;
	}

	PRINTF("textXpos = %d  textYpos = %d\n\n", textXpos, textYpos);
	return 0;
}

static int bleandConfig(Blend_Handle hBlend,
                        Buffer_Handle osdBuf, Buffer_Handle hDstBuf,
                        int xpos, int ypos, UInt8 trans)
{
	Blend_Config_Params bConfigParams = Blend_Config_Params_DEFAULT;
	Int i;

	if(hBlend) {
		bConfigParams.bmpXpos = xpos;
		bConfigParams.bmpYpos = ypos;

		for(i = 0; i < 4; i++) {
			bConfigParams.palette[i][0] = palette[i][0];
			bConfigParams.palette[i][1] = palette[i][1];
			bConfigParams.palette[i][2] = palette[i][2];
			/*			if(i  < 0||i == 0)
							bConfigParams.palette[i][3] = 0x00;
						else*/
			bConfigParams.palette[i][3] = trans;
		}

		/* Configure the OSD blending job */
		if(Blend_config(hBlend, NULL, osdBuf, hDstBuf, hDstBuf,
		                &bConfigParams) < 0) {
			DEBUG(DL_ERROR, "Failed to configure blending job\n");
			return -1;
		}
	}

	return 0;
}

/*OSD*/
void addtextosdtoender(Buffer_Handle hDstBuf)
{
	if(display_text == 0 && g_display_time == 0) {
		return;
	}

	int x = 0;
	int y = 0;
	int width, height;

	//PRINTF("text->xpos:%d,text->ypos:%d,capture_width:%d,capture_height:%d\n",
	//       textXpos, textYpos, capture_width, capture_height);
	float alpha = 0;
	alpha = ((100 - getTextAlpha()) * OSD_TRANS_VALUE / 100);
	//PRINTF("alpha = %x,getTextAlpha=%d\n", (int)alpha, getTextAlpha());

	//    DEBUG(DL_FLOW,"display_text = %d ,display_time = %d \n",display_text,g_display_time);
	/*显示文字*/
	if(display_text) {
		pthread_mutex_lock(&textOsd.mutex);
		//BufferGfx_Dimensions dim;

		//BufferGfx_getDimensions(textOsd.hBufs[textOsd.workingIdx], &dim);
		width = g_text_width ;
		height = 64;
		x = textXpos;
		y = textYpos;

		if(x >= 1920 - g_text_width) {
			x = 1920;
		}

		if(y >= 1080 - 64) {
			y = 1080;
		}

		x = (x * capture_width / 1920)  - width;
		y = (y * capture_height / 1080) - height;

		if(x < 0) {
			x = 0;
		}

		if(y < 0) {
			y = 0;
		}

		if(x % 16) {
			x -= (x % 16);
		}

		if(y % 16) {
			y -= (y % 16);
		}

		//	PRINTF("x = %d,y=%d,width=%d\n",x,y,width);

		if(bleandConfig(hTextBlend, textOsd.hBufs[textOsd.displayIdx], hDstBuf, x,
		                y, ((int)alpha & OSD_TRANS_VALUE)) != 0) {
			DEBUG(DL_ERROR, "Blend config failed \n");
			pthread_mutex_unlock(&textOsd.mutex);
			return;
		}

		if(Blend_execute(hTextBlend, textOsd.hBufs[textOsd.displayIdx], hDstBuf, hDstBuf) < 0) {
			DEBUG(DL_ERROR, "Blend execute failed \n");
		}

		pthread_mutex_unlock(&textOsd.mutex);
	}

	/*显示时间*/
	if(g_display_time) {
		//drawTimeBuffer();
		pthread_mutex_lock(&timeOsd.mutex);

		if(bleandConfig(hTimeBlend, timeOsd.hBufs[timeOsd.displayIdx], hDstBuf, TIME_XPOS,
		                TIME_YPOS, ((int)alpha & OSD_TRANS_VALUE)) != 0) {
			DEBUG(DL_ERROR, "Blend config failed \n");
			pthread_mutex_unlock(&timeOsd.mutex);
			return;
		}

		if(Blend_execute(hTimeBlend, timeOsd.hBufs[timeOsd.displayIdx], hDstBuf, hDstBuf) < 0) {
			DEBUG(DL_ERROR, "Blend execute failed \n");
		}

		pthread_mutex_unlock(&timeOsd.mutex);

	}

}
#if 0
/*添加字幕*/
int addtextdisplay(int xpos, int ypos, int showtime, char *text)
{
	int charlen = strlen(text);

	display_text = 1;

	if(charlen == 0) {
		display_text = 0;
	}

	if(checkCHN(text, charlen)) {
		char temp[512] = {0};
		code_convert("gb2312", "utf-32", text, charlen, temp, charlen * 4);
		drawTextBuffer(temp, charlen);
	} else {
		drawTextBuffer(text, charlen);
	}

	if(showtime) {
		DEBUG(DL_DEBUG, "display the time \n");
		drawTimeBuffer();
#if 0//displayframerate
		drawFramerateBuffer();
#endif
		display_time = 1;
	} else 	{
		DEBUG(DL_DEBUG, "Cancel Display The Time!!!\n");
		display_time = 0;
	}

	textXpos = xpos;
	textYpos = ypos;

	if(textXpos % 32) {
		textXpos -= (textXpos % 32);
	}

	if(textYpos % 32) {
		textYpos -= (textYpos % 32);
	}

	if((H264Height() - textYpos) < 64) {
		textYpos = H264Height() - 64;
	}

	return 0;
}
#else
/*添加字幕*/
int addtextdisplay(textinfo *text)
{
	PRINTF("x:%d,y:%d,alpha:%d,enable:%d,showtime:%d\n",
	       text->xpos, text->ypos, text->alpha, text->enable, text->showtime);
	int charlen = strlen(text->msgtext);
	int templen = 0;

	display_text = text->enable;

	if(charlen == 0) {
		display_text = 0;
	}

	textXpos = text->xpos;
	textYpos = text->ypos;

	if(textXpos < 0) {
		textXpos = 0;
	}

	if(textYpos < 0) {
		textYpos = 0;
	}

	templen = 1920 - textXpos;

	if(checkCHN(text->msgtext, charlen)) {
		char temp[512] = {0};

		//if(templen / 32 < charlen / 2) {
		//	charlen = 2 * templen / 32;
		//}

		code_convert("gb2312", "utf-32", text->msgtext, charlen, temp, charlen * 4);
		drawTextBuffer(temp, charlen);
		PRINTF("charlen = %d\n", charlen);
		//	if(text->xpos > H264Width() - 32) {
		//		text->xpos = (H264Width() - 16*charlen);
		//	}
	} else {
		//if(templen / 16 < charlen) {
		//	charlen = templen / 16;
		//}

		drawTextBuffer(text->msgtext, charlen);
		PRINTF("charlen = %d\n", charlen);
		//	if(text->xpos > H264Width() - 32) {
		//		text->xpos = (H264Width() - 16 * charlen);
		//	}

	}

	if(text->showtime) {
		DEBUG(DL_DEBUG, "display the time \n");
		drawTimeBuffer();
#if 0//displayframerate
		drawFramerateBuffer();
#endif
		g_display_time = text->showtime;
	} else 	{
		DEBUG(DL_DEBUG, "Cancel Display The Time!!!\n");
		g_display_time = 0;
	}

	PRINTF("H264Width =%d,H264Height() =%d,charlen=%d\n", H264Width(), H264Height(), charlen);

	//	if(text->ypos > (H264Height() - 32)) {
	//		text->ypos = (H264Height() - 32);
	//	}

	//	textXpos = text->xpos;
	//	textYpos = text->ypos;

	//	if(textXpos % 32) {
	//		textXpos -= (textXpos % 32);
	//	}


	//	if(textYpos % 32) {
	//		textYpos -= (textYpos % 32);
	//	}

	//	if((H264Height() - textYpos) < 64) {
	//		textYpos = H264Height() - 64;
	//	}

	PRINTF("x:%d,y:%d,alpha:%d,enable:%d,showtime:%d,text:%s\n",
	       textXpos, textYpos, text->alpha, text->enable, text->showtime, text->msgtext);
	return 0;
}

#endif
