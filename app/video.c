/*
****************************************************
Copyright (C), 1988-1999, Reach Tech. Co., Ltd.

File name:     	video.c

Description:    将采集的编码数据H264编码并给write线程

Date:     		2010-11-17

Author:	  		yangshh

version:  		V1.0

*****************************************************
*/

#include <xdc/std.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sdo/dmai/Fifo.h>
#include <ti/sdo/dmai/Pause.h>
#include <ti/sdo/dmai/BufTab.h>
#include <ti/sdo/dmai/VideoStd.h>
#include <ti/sdo/dmai/BufferGfx.h>
#include <ti/sdo/dmai/Rendezvous.h>
#include <ti/sdo/dmai/ce/Venc1.h>
#include <ti/sdo/ce/topvision/extensions/scale/scale.h>
#include <ti/sdo/codecs/h264fhdvenc/ih264fhdvenc.h>
#include <ti/sdo/ce/video1/videnc1.h>
#include <ti/sdo/dmai/Resize.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"
#include "video.h"
#include "sysparam.h"
#include "demo.h"
#ifdef HAVE_QUALITY_MODULE
#include "app_quality.h"
#endif
#include "app_video_output.h"

#include "log_common.h"
/* Number of buffers in the pipe to the capture thread */
#define NUM_VIDEO_BUFS           3

#define MAX_FRAMES_RECREATE		1300000


/*视/音频编码参数表*/
extern VAENC_params gVAenc_p;
extern ResizeParam gLowRate;
extern ResizeParam gResizeP;
extern int capture_height;
extern int capture_width;
extern Buffer_Handle CreateBuffer(int width, int height, ColorSpace_Type csType);

extern pthread_mutex_t		gCapEnc_m;
extern PLock gSetP_m;
extern SavePTable  gSysParaT;




/*
*****************************************************************
** 蓝屏
*****************************************************************
*/
int ParseBlueScreen(Buffer_Handle hBuf, int width, int height)
{
	unsigned char *pY, *pU, *pV;
	unsigned int i;

	DEBUG(DL_DEBUG, "    width:%d      height:%d\n", width, height);
	pY = (unsigned char *)Buffer_getUserPtr(hBuf);
	pU = (unsigned char *)Buffer_getUserPtr(hBuf) + width * height;
	pV = (unsigned char *)Buffer_getUserPtr(hBuf) + width * height + 1;

	for(i = 0; i < height * width; i++)		{
		*pY = 0x36;
		pY++;
	}

	for(i = 0; i < height * width / 4; i++)	{
		*pU = 0xC0;
		pU++;
		pU++;
	}

	for(i = 0; i < height * width / 4; i++)	{
		*pV = 0x70;
		pV++;
		pV++;
	}

	return 0;
}

#if 0
/*set H264 Param Table*/
int SetH264ParamTable(IH264FHDVENC_Params *param,
                      IH264FHDVENC_DynamicParams *dynparam,
                      IH264FHDVENC_VUIDataStructure *vuidata)
{
	param->videncParams.size      = sizeof(IH264FHDVENC_Params);
	param->videncParams.encodingPreset        = XDM_DEFAULT;
	param->videncParams.rateControlPreset     = IVIDEO_LOW_DELAY;
	param->videncParams.maxHeight             = H264_MAX_ENCODE_HIGHT + 8;
	param->videncParams.maxWidth              = H264_MAX_ENCODE_WIDTH;
	param->videncParams.maxFrameRate          = 30000;
	param->videncParams.maxBitRate            = MAXBITRATE;
	param->videncParams.dataEndianness        = XDM_LE_32;
	param->videncParams.maxInterFrameInterval = 0;
	param->videncParams.inputChromaFormat     = XDM_YUV_420SP;
	param->videncParams.inputContentType      = IVIDEO_PROGRESSIVE;
	param->videncParams.reconChromaFormat     = XDM_CHROMA_NA;
	param->profileIdc =  66; //66; //BASELINE_PROFILE;
	param->levelIdc = 40;
	param->EntropyCodingMode = 0;

	dynparam->videncDynamicParams.size              = sizeof(IH264FHDVENC_DynamicParams);
	dynparam->videncDynamicParams.inputHeight          = 1088;
	dynparam->videncDynamicParams.inputWidth           = 1920;
	dynparam->videncDynamicParams.refFrameRate         = 30000;
	dynparam->videncDynamicParams.targetFrameRate      = 30000;
	dynparam->videncDynamicParams.targetBitRate        = MAXBITRATE;
	dynparam->videncDynamicParams.intraFrameInterval   = 30;
	dynparam->videncDynamicParams.generateHeader       = XDM_ENCODE_AU; //XDM_ENCODE_AU;
	dynparam->videncDynamicParams.captureWidth         = 0;
	dynparam->videncDynamicParams.forceFrame           = IVIDEO_NA_FRAME; //IVIDEO_NA_FRAME;
	dynparam->videncDynamicParams.interFrameInterval     = 0;
	dynparam->videncDynamicParams.mbDataFlag            = 0x0;
	dynparam->sliceCodingParams.sliceCodingPreset = IH264_SLICECODING_DEFAULT;
	//   dynparam->sliceCodingParams.sliceMode = IH264_SLICEMODE_BYTES;
	dynparam->sliceCodingParams.sliceMode = IH264_SLICEMODE_NONE;
	dynparam->sliceCodingParams.streamFormat = IH264_BYTE_STREAM;
	dynparam->sliceCodingParams.sliceUnitSize = 1500;   	//1500
	dynparam->sliceCodingParams.sliceStartOffset[0] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[1] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[2] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[3] = 0;

	dynparam->sliceCodingParams.streamFormat = IH264_SLICECODING_DEFAULT;

	dynparam->OutBufSize = -1;//4200000;//-1;//ver H264EncoderApp.c

	dynparam->QPISlice = 51;
	dynparam->QPSlice =  51;

	dynparam->RateCtrlQpMax = 51;
	dynparam->RateCtrlQpMin = 0;
	dynparam->NumRowsInSlice = 0;
	dynparam->LfDisableIdc = 0;
	dynparam->LFAlphaC0Offset = 0;
	dynparam->LFBetaOffset = 0;
	dynparam->ChromaQPOffset = 0;
	dynparam->SecChromaQPOffset = 0;
	dynparam->PicAFFFlag = 0; //??
	dynparam->PicOrderCountType = 0;
	dynparam->AdaptiveMBs = 0; //??
	dynparam->VUIData.buf = NULL; //ver xdm.h
	dynparam->VUIData.bufSize = 0;

	dynparam->SEIParametersFlag = 0;
	dynparam->VUIParametersFlag = 0;  //1;  modify ysh 20091230
	dynparam->NALUnitCallback = NULL; //callback_func;// NULL;
	dynparam->SkipStartCodesInCallback = 1;
	dynparam->Intra4x4EnableFlag = 0;
	dynparam->MESelect = 1; //IH264FHDVENC_ME1080i;
	dynparam->MVDataFlag = 0;
	dynparam->SkipStartCodesInCallback = 1;
	dynparam->BlockingCallFlag = 0;
	dynparam->ME1080iMode = 0;
	dynparam->MVDataFlag = 0;
	dynparam->Transform8x8DisableFlag = 1;
	dynparam->Intra8x8EnableFlag = 0;
	dynparam->InterlaceReferenceMode = 0;
	dynparam->ChromaConversionMode = 0;
	dynparam->maxDelay = 1000;
	dynparam->MaxSlicesSupported = 90;

	if(dynparam->VUIParametersFlag) {
		dynparam->VUIData.buf = (XDAS_Int8 *) vuidata;
		dynparam->VUIData.bufSize = sizeof(IH264FHDVENC_VUIDataStructure);
		vuidata->aspect_ratio_info_present_flag  = 0;
		vuidata->overscan_info_present_flag  = 0;
		vuidata->chroma_loc_info_present_flag    = 0;
		vuidata->timing_info_present_flag    = 1;
		vuidata->nal_hrd_parameters_present_flag = 1;
		vuidata->vcl_hrd_parameters_present_flag = 0;
		vuidata->bitstream_restriction_flag  = 0;
		vuidata->nal_hrd_parameters.cpb_cnt_minus1 = 0;
		vuidata->nal_hrd_parameters.bit_rate_scale = 0;
		vuidata->nal_hrd_parameters.cpb_size_scale = 3;
		vuidata->nal_hrd_parameters.cbr_flag[0] = 0;
		vuidata->nal_hrd_parameters.initial_cpb_removal_delay_length_minus1 = 16;
		vuidata->nal_hrd_parameters.cpb_removal_delay_length_minus1         = 6;
		vuidata->nal_hrd_parameters.dpb_output_delay_length_minus1          = 4;
		vuidata->nal_hrd_parameters.time_offset_length                      = 11;
	}

	return 0;
}
#endif
#ifdef NEW_ENCODE_LIB_HP
int SetH264ParamTable(IH264FHDVENC_Params *param,
                      IH264FHDVENC_DynamicParams *dynparam,
                      IH264FHDVENC_VUIDataStructure *vuidata)
{
	param->videncParams.size      = sizeof(IH264FHDVENC_Params);
	param->videncParams.encodingPreset        = XDM_DEFAULT;
	param->videncParams.rateControlPreset     = IVIDEO_LOW_DELAY;
	param->videncParams.maxHeight             = 1088;//1088;
	param->videncParams.maxWidth              = 1920;//1920;
	param->videncParams.maxFrameRate          = 30000;
	param->videncParams.maxBitRate            = 20000000;
	param->videncParams.dataEndianness        = XDM_LE_32;
	param->videncParams.maxInterFrameInterval = 0;
	param->videncParams.inputContentType      = IVIDEO_PROGRESSIVE;
	param->videncParams.reconChromaFormat     = XDM_CHROMA_NA;
	param->profileIdc = 100; //High_PROFILE;
	param->levelIdc = 40;
	param->EntropyCodingMode = 1;
	dynparam->videncDynamicParams.size              = sizeof(IH264FHDVENC_DynamicParams);
	dynparam->videncDynamicParams.inputHeight          = 1088;//1088;
	dynparam->videncDynamicParams.inputWidth           = 1920;//1920;
	dynparam->videncDynamicParams.refFrameRate         = 30000;
	dynparam->videncDynamicParams.targetFrameRate      = 30000;

	dynparam->videncDynamicParams.intraFrameInterval   = 150;
	dynparam->videncDynamicParams.generateHeader       = 0; //XDM_ENCODE_AU;
	dynparam->videncDynamicParams.captureWidth         = 0;
	dynparam->videncDynamicParams.forceFrame           = -1; //IVIDEO_NA_FRAME;
	dynparam->videncDynamicParams.interFrameInterval     = 0;
	dynparam->videncDynamicParams.mbDataFlag            = 0x0;
	dynparam->sliceCodingParams.sliceCodingPreset = 0x0;
	// dynparam->sliceCodingParams.sliceMode = IH264_SLICEMODE_BYTES;
	dynparam->sliceCodingParams.sliceMode = IH264_SLICEMODE_NONE;
	dynparam->sliceCodingParams.streamFormat = IH264_BYTE_STREAM;
	dynparam->sliceCodingParams.sliceUnitSize = 1300;    //1500

#ifdef QUICKTIME_RTSP

	dynparam->sliceCodingParams.sliceMode = IH264_SLICEMODE_BYTES;
	//the value <= rtsp playload len
	dynparam->sliceCodingParams.sliceUnitSize = 1300;    //1500
#endif
	dynparam->sliceCodingParams.sliceStartOffset[0] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[1] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[2] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[3] = 0;

	dynparam->sliceCodingParams.streamFormat = IH264_SLICECODING_DEFAULT;

	dynparam->OutBufSize = -1;//4200000;//-1;//ver H264EncoderApp.c



	dynparam->RateCtrlQpMax = 44;
	dynparam->RateCtrlQpMin = 10;
	dynparam->NumRowsInSlice = 0;
	dynparam->LfDisableIdc = 0;
	dynparam->LFAlphaC0Offset = 0;
	dynparam->LFBetaOffset = 0;
	dynparam->ChromaQPOffset = 0;
	dynparam->SecChromaQPOffset = 0;
	dynparam->PicAFFFlag = 0; //??
	dynparam->PicOrderCountType = 0;
	dynparam->AdaptiveMBs = 0; //??
	dynparam->VUIData.buf = NULL; //ver xdm.h
	dynparam->VUIData.bufSize = 0;

	dynparam->SEIParametersFlag = 0;
	dynparam->VUIParametersFlag = 0; // //1;  modify ysh 20091230
	dynparam->NALUnitCallback = NULL; //callback_func;// NULL;
	dynparam->SkipStartCodesInCallback = 1;

	dynparam->MESelect = 1; //IH264FHDVENC_ME1080i;
	dynparam->MVDataFlag = 0;
	dynparam->SkipStartCodesInCallback = 1;
	dynparam->BlockingCallFlag = 0;
	dynparam->ME1080iMode = 0;
	dynparam->MVDataFlag = 0;
	dynparam->Transform8x8DisableFlag = 1;
	dynparam->Intra8x8EnableFlag = 0;
	dynparam->InterlaceReferenceMode = 0;
	dynparam->ChromaConversionMode = 1;

	param->videncParams.inputChromaFormat     = XDM_YUV_420SP;
	dynparam->QPISlice = 28;
	dynparam->QPSlice =  28;
	dynparam->videncDynamicParams.targetBitRate        = 20000000;
	dynparam->Intra4x4EnableFlag = 0;
	dynparam->ChromaConversionMode = 0;
	dynparam->maxDelay = 8000;
	//dynparam->MaxSlicesSupported = 90;
	dynparam->MaxSlicesSupported_PFrame = 90;

	dynparam->MaxSlicesSupported_IFrame = 90;

	if(dynparam->VUIParametersFlag) {
		dynparam->VUIData.buf = (XDAS_Int8 *) vuidata;
		dynparam->VUIData.bufSize = sizeof(IH264FHDVENC_VUIDataStructure);
		vuidata->aspect_ratio_info_present_flag  = 0;
		vuidata->overscan_info_present_flag  = 0;
		vuidata->chroma_loc_info_present_flag    = 0;
		vuidata->timing_info_present_flag    = 1;
		vuidata->nal_hrd_parameters_present_flag = 1;
		vuidata->vcl_hrd_parameters_present_flag = 0;
		vuidata->bitstream_restriction_flag  = 0;
		vuidata->nal_hrd_parameters.cpb_cnt_minus1 = 0;
		vuidata->nal_hrd_parameters.bit_rate_scale = 0;
		vuidata->nal_hrd_parameters.cpb_size_scale = 3;
		vuidata->nal_hrd_parameters.cbr_flag[0] = 0;
		vuidata->nal_hrd_parameters.initial_cpb_removal_delay_length_minus1 = 16;
		vuidata->nal_hrd_parameters.cpb_removal_delay_length_minus1         = 6;
		vuidata->nal_hrd_parameters.dpb_output_delay_length_minus1          = 4;
		vuidata->nal_hrd_parameters.time_offset_length                      = 11;
	}

	return 0;
}

#else
int SetH264ParamTable(IH264FHDVENC_Params *param,
                      IH264FHDVENC_DynamicParams *dynparam,
                      IH264FHDVENC_VUIDataStructure *vuidata)
{
	param->videncParams.size      = sizeof(IH264FHDVENC_Params);
	param->videncParams.encodingPreset        = XDM_DEFAULT;
	param->videncParams.rateControlPreset     = IVIDEO_LOW_DELAY;
	param->videncParams.maxHeight             = 1088;
	param->videncParams.maxWidth              = 1920;
	param->videncParams.maxFrameRate          = 30000;
	param->videncParams.maxBitRate            = MAXBITRATE;
	param->videncParams.dataEndianness        = XDM_LE_32;
	param->videncParams.maxInterFrameInterval = 0;
	param->videncParams.inputChromaFormat     = XDM_YUV_420P;
	param->videncParams.inputContentType      = IVIDEO_PROGRESSIVE;
	param->videncParams.reconChromaFormat     = XDM_CHROMA_NA;
	param->profileIdc = 66; //BASELINE_PROFILE;
	param->levelIdc = 40;
	param->EntropyCodingMode = 0;

	dynparam->videncDynamicParams.size              = sizeof(IH264FHDVENC_DynamicParams);
	dynparam->videncDynamicParams.inputHeight          = 1088;
	dynparam->videncDynamicParams.inputWidth           = 1920;
	dynparam->videncDynamicParams.refFrameRate         = 30000;
	dynparam->videncDynamicParams.targetFrameRate      = 30000;
	dynparam->videncDynamicParams.targetBitRate        = 2048000;
	dynparam->videncDynamicParams.intraFrameInterval   = 30;
	dynparam->videncDynamicParams.generateHeader       = 0; //XDM_ENCODE_AU;
	dynparam->videncDynamicParams.captureWidth         = 0;
	dynparam->videncDynamicParams.forceFrame           = -1; //IVIDEO_NA_FRAME;
	dynparam->videncDynamicParams.interFrameInterval     = 0;
	dynparam->videncDynamicParams.mbDataFlag            = 0x0;
	dynparam->sliceCodingParams.sliceCodingPreset = 0x0;
	//dynparam->sliceCodingParams.sliceMode = IH264_SLICEMODE_BYTES;
	dynparam->sliceCodingParams.sliceMode = IH264_SLICEMODE_NONE;
	dynparam->sliceCodingParams.streamFormat = IH264_BYTE_STREAM;
	dynparam->sliceCodingParams.sliceUnitSize = 1500;   	//1500
	dynparam->sliceCodingParams.sliceStartOffset[0] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[1] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[2] = 0;
	dynparam->sliceCodingParams.sliceStartOffset[3] = 0;

	dynparam->sliceCodingParams.streamFormat = IH264_SLICECODING_DEFAULT;

	dynparam->OutBufSize = -1;//4200000;//-1;//ver H264EncoderApp.c

	dynparam->QPISlice = 28;
	dynparam->QPSlice = 28;

	dynparam->RateCtrlQpMax = 51;
	dynparam->RateCtrlQpMin = 0;
	dynparam->NumRowsInSlice = 0;
	dynparam->LfDisableIdc = 0;
	dynparam->LFAlphaC0Offset = 0;
	dynparam->LFBetaOffset = 0;
	dynparam->ChromaQPOffset = 0;
	dynparam->SecChromaQPOffset = 0;
	dynparam->PicAFFFlag = 0; //??
	dynparam->PicOrderCountType = 0;
	dynparam->AdaptiveMBs = 0; //??
	dynparam->VUIData.buf = NULL; //ver xdm.h
	dynparam->VUIData.bufSize = 0;

	dynparam->SEIParametersFlag = 0;
	dynparam->VUIParametersFlag = 0;  //1;  modify ysh 20091230
	dynparam->NALUnitCallback = NULL; //callback_func;// NULL;
	dynparam->SkipStartCodesInCallback = 1;
	dynparam->Intra4x4EnableFlag = 3;
	dynparam->MESelect = 1; //IH264FHDVENC_ME1080i;
	dynparam->MVDataFlag = 0;
	dynparam->SkipStartCodesInCallback = 1;
	dynparam->BlockingCallFlag = 0;
	dynparam->ME1080iMode = 0;
	dynparam->MVDataFlag = 0;
	dynparam->Transform8x8DisableFlag = 1;
	dynparam->Intra8x8EnableFlag = 0;
	dynparam->InterlaceReferenceMode = 0;
	dynparam->ChromaConversionMode = 1;

	if(dynparam->VUIParametersFlag) {
		dynparam->VUIData.buf = (XDAS_Int8 *) vuidata;
		dynparam->VUIData.bufSize = sizeof(IH264FHDVENC_VUIDataStructure);
		vuidata->aspect_ratio_info_present_flag  = 0;
		vuidata->overscan_info_present_flag  = 0;
		vuidata->chroma_loc_info_present_flag    = 0;
		vuidata->timing_info_present_flag    = 1;
		vuidata->nal_hrd_parameters_present_flag = 1;
		vuidata->vcl_hrd_parameters_present_flag = 0;
		vuidata->bitstream_restriction_flag  = 0;
		vuidata->nal_hrd_parameters.cpb_cnt_minus1 = 0;
		vuidata->nal_hrd_parameters.bit_rate_scale = 0;
		vuidata->nal_hrd_parameters.cpb_size_scale = 3;
		vuidata->nal_hrd_parameters.cbr_flag[0] = 0;
		vuidata->nal_hrd_parameters.initial_cpb_removal_delay_length_minus1 = 16;
		vuidata->nal_hrd_parameters.cpb_removal_delay_length_minus1         = 6;
		vuidata->nal_hrd_parameters.dpb_output_delay_length_minus1          = 4;
		vuidata->nal_hrd_parameters.time_offset_length                      = 11;
	}

	return 0;
}
#endif

/*填充黑图*/
int __BlackFill(Buffer_Handle hBuf, int width , int height)
{

	Int8  *yPtr     = Buffer_getUserPtr(hBuf);
	Int32  ySize    = width * height;
	Int8  *cbcrPtr  = yPtr + ySize;
	Int32  cbCrSize = (width * height) / 2;
	Int    i;

	/* Fill the Y plane */
	for(i = 0; i < ySize; i++) {
		yPtr[i] = 0x0;
	}

	for(i = 0; i < cbCrSize; i++) {
		cbcrPtr[i] = 0x80;
	}

	return 0;
}

/*填充黑图*/
int __BlackFill_422(Buffer_Handle hBuf, int width , int height)
{

	Int8  *yPtr     = Buffer_getUserPtr(hBuf);
	Int32  ySize    = width * height;
	Int8  *cbcrPtr  = yPtr + ySize;
	Int32  cbCrSize = (width * height) ;
	Int    i;

	/* Fill the Y plane */
	for(i = 0; i < ySize; i++) {
		yPtr[i] = 0x0;
	}

	for(i = 0; i < cbCrSize; i++) {
		cbcrPtr[i] = 0x80;
	}

	return 0;
}

/*系统启动时初始化参数表*/
int StartH264ParamTable(IH264FHDVENC_Params *params,
                        IH264FHDVENC_DynamicParams *dynParams)
{
	unsigned int NowFrame = GetCtrlFrame();
	int bitrate = gVAenc_p.venc.bitrate;
	int quality = gVAenc_p.venc.quality;

	if(NowFrame < 1) {
		NowFrame = 1;
	}

	gVAenc_p.venc.quality = (quality * 30) / (NowFrame);
	bitrate = (bitrate * 30) / (NowFrame);
	gVAenc_p.venc.bitrate = (UINT16)bitrate;

	if(gVAenc_p.venc.cbr == 0)  { //quality VBR
		params->videncParams.rateControlPreset = IVIDEO_STORAGE ;
		dynParams->videncDynamicParams.targetBitRate = gVAenc_p.venc.quality * 100 * 1000;

		if(params->videncParams.maxBitRate > MAXBITRATE || dynParams->videncDynamicParams.targetBitRate > MAXBITRATE) {
			params->videncParams.maxBitRate = MAXBITRATE;
			dynParams->videncDynamicParams.targetBitRate = MAXBITRATE;
		}

	} else { //bitrate CBR
		params->videncParams.rateControlPreset  = IVIDEO_LOW_DELAY;
		dynParams->videncDynamicParams.targetBitRate = gVAenc_p.venc.bitrate * 1000;

		if(params->videncParams.maxBitRate > MAXBITRATE || dynParams->videncDynamicParams.targetBitRate > MAXBITRATE) {
			params->videncParams.maxBitRate = MAXBITRATE;
			dynParams->videncDynamicParams.targetBitRate = MAXBITRATE;
		}
	}

	dynParams->videncDynamicParams.intraFrameInterval   = app_get_video_gop(HIGH_VIDEO);


	DEBUG(DL_DEBUG, "[StartH264ParamTable]pVenc->bitrate:%d\n", gVAenc_p.venc.bitrate);
	return 0;
}

/*
##
##动态设置编码参数
##ret(1) 表示编码参数发生改变
##ret(0) 表示编码参数未改变
##
*/
int VencRunTimeFxn(VENC_params *pVenc, IH264FHDVENC_Params *params , IH264FHDVENC_DynamicParams *dynParams)
{

	if(H264_ENC_PARAM_FLAG == pVenc->uflag) {
		StartH264ParamTable(params, dynParams);
		pVenc->uflag = 0x0;
		return 1;
	}

	return 0;
}
int SetScaleParam(SCALE_DynamicParams *scaleparams, int inwidth, int inheight, int offsetx, int offsety, int outwidth, int outheight)
{
	int flag = 0;

	if(scaleparams == NULL) {
		return -1;
	}

	//初始化
	scaleparams->size = sizeof(SCALE_DynamicParams);
	scaleparams->iwidth = 1920;
	scaleparams->iheight = 1088;
	scaleparams->cropirow = 0;
	scaleparams->cropicol = 0;
	scaleparams->cropiwidth = inwidth;
	scaleparams->cropiheight = inheight;
	scaleparams->cropvuoffset = 1920 * 1088;
	scaleparams->owidth = outwidth;
	//if(scaleparams->owidth > 720)scaleparams->owidth=720;
	scaleparams->oheight = outheight;
	scaleparams->desktoprow = offsety; //默认都是从第一行，无偏移显示
	scaleparams->desktopcol = offsetx;
	scaleparams->desktopwidth = 1920; //依赖原始输出显示的宽高 ，解码器DEC1000使用
	scaleparams->desktopheight = 1088;
	scaleparams->upsampleout = 0;
	scaleparams->sampleMode	= 1;
	//PRINTF("in w=%d,h=%d,o w=%d =%d\n",scaleparams->cropiwidth,scaleparams->cropiheight,scaleparams->owidth,scaleparams->oheight );
	return flag ;
}
/*计算编码的动态参数*/
static int CalcEncodeDynParam(int *recreate, IH264FHDVENC_Params *params , IH264FHDVENC_DynamicParams *dynParams)
{
	static int pre_level = -1;
	int level = -1, indata = 0;

	if(gblGetH264pixel() == VIDEO_1080_PIXEL) {
		dynParams->videncDynamicParams.inputHeight = H264_MAX_ENCODE_HIGHT;
		dynParams->videncDynamicParams.inputWidth  = H264_MAX_ENCODE_WIDTH;
#ifdef HAVE_QUALITY_MODULE

		if(AppIsResizeFlag() && AppISresize()) {
			dynParams->videncDynamicParams.inputWidth = AppGetResizeWidth();
			dynParams->videncDynamicParams.inputHeight = AppGetResizeHeight();
		}

#endif
		*recreate = 1;   //re create H264 lib
		gblSetH264pixel(VIDEO_INVALID);
	} else if(gblGetH264pixel() == VIDEO_720_PIXEL) {
		dynParams->videncDynamicParams.inputHeight = 720;
		dynParams->videncDynamicParams.inputWidth  = 1280;
#ifdef HAVE_QUALITY_MODULE

		if(AppIsResizeFlag() && AppISresize()) {
			dynParams->videncDynamicParams.inputWidth = AppGetResizeWidth();
			dynParams->videncDynamicParams.inputHeight = AppGetResizeHeight();
		}

#endif
		*recreate = 1;   //re create H264 lib
		gblSetH264pixel(VIDEO_INVALID);
	} else if(gblGetH264pixel() == VIDEO_VGA_PIXEL)	{
		dynParams->videncDynamicParams.inputHeight = H264Height();
		dynParams->videncDynamicParams.inputWidth  = H264Width();
#ifdef HAVE_QUALITY_MODULE

		if(AppIsResizeFlag() && AppISresize()) {
			dynParams->videncDynamicParams.inputWidth = AppGetResizeWidth();
			dynParams->videncDynamicParams.inputHeight = AppGetResizeHeight();
		}

#endif
		*recreate = 1;   //re create H264 lib
		gblSetH264pixel(VIDEO_INVALID);
	}  else if(gblGetH264pixel() == VIDEO_480P_PIXEL) {
		dynParams->videncDynamicParams.inputHeight = 480;
		dynParams->videncDynamicParams.inputWidth  = 720;
#ifdef HAVE_QUALITY_MODULE

		if(AppIsResizeFlag() && AppISresize()) {
			dynParams->videncDynamicParams.inputWidth = AppGetResizeWidth();
			dynParams->videncDynamicParams.inputHeight = AppGetResizeHeight();
		}

#endif
		*recreate = 1;   //re create H264 lib
		gblSetH264pixel(VIDEO_INVALID);
	}  else if(gblGetH264pixel() == VIDEO_576P_PIXEL) {
		dynParams->videncDynamicParams.inputHeight = 576;
		dynParams->videncDynamicParams.inputWidth  = 720;
#ifdef HAVE_QUALITY_MODULE

		if(AppIsResizeFlag() && AppISresize()) {
			dynParams->videncDynamicParams.inputWidth = AppGetResizeWidth();
			dynParams->videncDynamicParams.inputHeight = AppGetResizeHeight();
		}

#endif
		*recreate = 1;   //re create H264 lib
		gblSetH264pixel(VIDEO_INVALID);
	}  else if(gblGetH264pixel() == VIDEO_NO_SOURCE) {		//no source

		dynParams->videncDynamicParams.inputHeight = H264Height();
		dynParams->videncDynamicParams.inputWidth  = H264Width();
#ifdef HAVE_QUALITY_MODULE

		if(AppIsResizeFlag() && AppISresize()) {
			dynParams->videncDynamicParams.inputWidth = AppGetResizeWidth();
			dynParams->videncDynamicParams.inputHeight = AppGetResizeHeight();
		}

#endif
		*recreate = 1;   //re create H264 lib
		gblSetH264pixel(VIDEO_INVALID);
	}

	//PRINTF("=%d,=%d,=%d=%d\n",H264Height(),H264Width(),AppGetResizeWidth(),AppGetResizeHeight());
	getEncodelevel(indata, &level);

	if(level == 0 && pre_level != level) {
		pre_level = level;
		PRINTF("baseline profile.............\n");
		params->videncParams.inputChromaFormat     = XDM_YUV_420SP;
		params->profileIdc = 66; //BASELINE_PROFILE;
		params->EntropyCodingMode = 0;
		//dynParams->sliceCodingParams.sliceUnitSize = 1500;   	//1500
		//dynParams->RateCtrlQpMax = 51;
		//dynParams->RateCtrlQpMin = 0;
		*recreate = 1;
		setEncodelevel(level, &indata);
	}

	if(level == 1 && pre_level != level) {
		pre_level = level;
		PRINTF("high profile.............\n");
		params->videncParams.inputChromaFormat     = XDM_YUV_420SP;
		params->profileIdc = 100; //BASELINE_PROFILE;
		params->EntropyCodingMode = 1;
		//dynParams->sliceCodingParams.sliceUnitSize = 1300;   	//1500
		//dynParams->RateCtrlQpMax = 44;
		//dynParams->RateCtrlQpMin = 10;
		*recreate = 1;
		setEncodelevel(level, &indata);
	}

#ifdef HAVE_QUALITY_MODULE

	/*is resize*/
	//if(AppIsResizeFlag() && AppISresize()&& (RECREATE_OK == AppGetResizeReset()))
	if((RECREATE_OK == AppGetResizeReset())) {
		dynParams->videncDynamicParams.inputHeight = AppGetResizeHeight();
		dynParams->videncDynamicParams.inputWidth  = AppGetResizeWidth();
		AppSetResizeReset(RECREATE_INVALID);
		*recreate = 1;   //re create H264 lib
	}

	if(*recreate == 1) {
		DEBUG(DL_DEBUG, "video encode input width = %d,height= %d\n", (int)(dynParams->videncDynamicParams.inputWidth), (int)(dynParams->videncDynamicParams.inputHeight));
	}

#endif
	return (*recreate);
}

/*是否强制I frames*/
int IISForceIframe(VENC_params *pVenc)
{
	if(AVIIF_KEYFRAME == pVenc->req_I) {
		DEBUG(DL_DEBUG, "Force Request I frames!!\n");
		pVenc->req_I = 0;
		return 1;
	}

	return 0;
}

int gVideoProcessFlag = 0;

/*
 *****************************************************************************
 * videoThrFxn
 *****************************************************************************
*/
Void *videoThrFxn(Void *arg)
{
	PRINTF("get pid= %d\n", getpid());
	DEBUG(DL_DEBUG, "[videoThrFxn] start ...\n");
	VideoEnv               *envp                = (VideoEnv *) arg;
	Void                   *status              = THREAD_SUCCESS;
	BufferGfx_Attrs         gfxAttrs            = BufferGfx_Attrs_DEFAULT;
	Venc1_Handle            hVe1                = NULL;
	Engine_Handle           hEngine             = NULL;
	BufTab_Handle           hBufTab             = NULL;
	Int                     frameCnt            = 0;
	Buffer_Handle           hBlueBuffer			= NULL;
	Int 					paramRet			= 0;
	int 					isFirstFrame 		= 1;
	int 					mode =  gblGetMode();
	int 					pre_mode = gblGetMode();
	Buffer_Handle           hCapBuf, hDstBuf;
	Int                     fifoRet;
	Int                     bufIdx,	recreate = 1, iis_force = 0;
	IH264FHDVENC_Params				Ih264params;
	IH264FHDVENC_VUIDataStructure	Ih264vui_params;
	IH264FHDVENC_DynamicParams		Ih264dynamicparams;
	BufferGfx_Dimensions	srcDim, dim;//dstDim;
	int blue_w = 1920, blue_h = 1088, ret = 0;
	Buffer_Handle       hBuftmp;

	//BufferGfx_Dimensions indim,outdim;
	SCALE_Params DefaultScaleParam = {
		sizeof(SCALE_Params),
		800,   		//owidthd//输出宽
		600,   		//oheight//输出高1001075510010100
		1920, 			//iwdith//输入buf的linelength
		1088,     	 	//iheight//输入buf的高
		0,			//cropirow//输入图像在buf中的左边dim值的x，y
		0,			//cropicol//
		1024,		//cropiwidth//输入的实际有效像素宽高
		768,		//cropiheight//
		1920 * 1088, 	//cropvuoffset//输入buf的uv数据相对y数据的偏移值
		0,			//desktoprow//输出图像在输出buf中的坐标相当于dim值的x，y
		0,			//desktopcol//
		1920,		//desktopwidth//输出图像buf的linelength
		1088,		//desktopheight//输出图像buf的高
		1,			//samplemode//设置输入/出的数据采样率，=1 为4：2：0；=0 为4:2:2
		0,			//packetmode	////      设置数据封装格式，=0 为SEMI

	};
	SCALE_Handle 	scalerHandle = NULL;
	SCALE_DynamicParams     scale_parames;
	SCALE_InArgs	scaleInArgs;
	SCALE_OutArgs	scaleOutArgs;
	OutputVideoInfo  *pre_outpar = NULL ;

	pre_outpar = malloc(sizeof(OutputVideoInfo));

	pre_outpar->encodelevel = 0;
	pre_outpar->preset = 0;
	pre_outpar->IFrameinterval = 150;
	//pre_outpar->logotext = 0;
	pre_outpar->nFrameRate = 30;
	pre_outpar->logo_show = 0;
	pre_outpar->text_show = 0;
	pre_outpar->resizeMode = 0;
	pre_outpar->resolution = 0;
	hBlueBuffer = CreateBuffer(1920, 1088, ColorSpace_YUV420PSEMI);

	if(hBlueBuffer == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	ParseBlueScreen(hBlueBuffer, blue_w, blue_h);
	//ParseBlueScreen(hBufTabResize,blue_w,blue_h);
	/*  创建临时buf   */
	hBuftmp = CreateBuffer(blue_w, blue_h, ColorSpace_YUV420PSEMI);

	if(hBuftmp == NULL) {
		ERR("Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	/* Open the codec engine */
	hEngine = Engine_open(envp->engineName, NULL, NULL);

	if(hEngine == NULL) {
		DEBUG(DL_ERROR, "Failed to open codec engine %s\n", envp->engineName);
		cleanup(THREAD_FAILURE);
	}

	SetH264ParamTable(&Ih264params, &Ih264dynamicparams, &Ih264vui_params);
	/*系统参数表*/
	StartH264ParamTable(&Ih264params, &Ih264dynamicparams);
	Ih264dynamicparams.videncDynamicParams.intraFrameInterval   = app_get_video_gop(HIGH_VIDEO);
	/* Create the video encoder */
	DEBUG(DL_DEBUG, "videoThrFxn before Venc1_create\n");
	hVe1 = Venc1_create(hEngine, envp->videoEncoder,
	                    (VIDENC1_Params *)(&Ih264params),
	                    (VIDENC1_DynamicParams *)(&Ih264dynamicparams));
	isFirstFrame = 1;
	DEBUG(DL_DEBUG, "videoThrFxn after Venc1_create\n");

	if(hVe1 == NULL) {
		DEBUG(DL_ERROR, "Failed to create video encoder: %s\n", envp->videoEncoder);
		cleanup(THREAD_FAILURE);
	}

	writeWatchDog();
	scalerHandle = SCALE_create(hEngine, "scale", &DefaultScaleParam);
	/* Store the output buffer size in the environment */
	envp->outBufSize = Venc1_getOutBufSize(hVe1);
	/* Signal that the codec is created and output buffer size available */
	Rendezvous_meet(envp->hRendezvousWriter);
	gfxAttrs.colorSpace = ColorSpace_YUV420PSEMI;
	gfxAttrs.dim.width = H264_MAX_ENCODE_WIDTH;
	gfxAttrs.dim.height = H264_MAX_ENCODE_HIGHT + 8;
	gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(gfxAttrs.dim.width,
	                          gfxAttrs.colorSpace);
	hBufTab = BufTab_create(NUM_VIDEO_BUFS, Venc1_getInBufSize(hVe1),
	                        BufferGfx_getBufferAttrs(&gfxAttrs));

	if(hBufTab == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	/* Send buffers to the capture thread to be ready for main loop */
	for(bufIdx = 0; bufIdx < NUM_VIDEO_BUFS; bufIdx++) {
		if(Fifo_put(envp->to_capture,
		            BufTab_getBuf(hBufTab, bufIdx)) < 0) {
			DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

		__BlackFill(BufTab_getBuf(hBufTab, bufIdx), H264_MAX_ENCODE_WIDTH, H264_MAX_ENCODE_HIGHT + 8);
	}

	BufferGfx_getDimensions(BufTab_getBuf(hBufTab, 0), &srcDim);
	Rendezvous_meet(envp->hRendezvousInit);
	writeWatchDog();
	DEBUG(DL_DEBUG, "***************pre_mode = %d,mode=%d***************\n", pre_mode, mode);

	unsigned int sound_time = 0;


	while(!gblGetQuit()) {
		/* Get a buffer to encode from the capture thread */
		fifoRet = Fifo_get(envp->from_capture, &hCapBuf);

		if(fifoRet < 0) {
			DEBUG(DL_ERROR, "Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		//BufferGfx_getDimensions(hCapBuf, &indim);
		//PRINTF("hCapBuf 2 w=%d,h=%d\n",indim.width,indim.height);
		//capture_time = Buffer_getInterval(hCapBuf);

		/* Did the capture thread flush the fifo? */
		if(fifoRet == Dmai_EFLUSH) {
			cleanup(THREAD_SUCCESS);
		}

		BufferGfx_getDimensions(hCapBuf, &dim);
		/* Get a buffer to encode to from the writer thread */
		fifoRet = Fifo_get(envp->from_writer, &hDstBuf);

		if(fifoRet < 0) {
			DEBUG(DL_ERROR, "Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		//Buffer_setInterval(hDstBuf, capture_time);

		/* Did the writer thread flush the fifo? */
		if(fifoRet == Dmai_EFLUSH) {
			cleanup(THREAD_SUCCESS);
		}

		mode = gblGetMode();
#if 0

		//DEBUG(DL_DEBUG, "@@@@@@@@@@@@@@@@@pre_mode = %d,mode=%d@@@@@@@@@@@@@@@@@@\n",pre_mode,mode);
		if(pre_mode != mode) {
			pre_mode = mode;
			DEBUG(DL_DEBUG, "restart video encode================pre_mode = %d,mode=%d\n", pre_mode, mode);
			writeWatchDog();
			gblEncodeLcok();

			if(hVe1) {
				Venc1_delete(hVe1);
				hVe1 = NULL;
			}

			writeWatchDog();
			//Ih264dynamicparams.videncDynamicParams.inputHeight          = gResizeP.dst_h;//1088;
			//Ih264dynamicparams.videncDynamicParams.inputWidth           = gResizeP.dst_w;//1920;
			//DEBUG(DL_DEBUG,"video encode input width = %d,height= %d,gResizeP.dst_h=%d,gResizeP.dst_w=%d\n",Ih264dynamicparams.videncDynamicParams.inputWidth,Ih264dynamicparams.videncDynamicParams.inputHeight,gResizeP.dst_h,gResizeP.dst_w);
			hVe1 = Venc1_create(hEngine, envp->videoEncoder,
			                    (VIDENC1_Params *)(&Ih264params),
			                    (VIDENC1_DynamicParams *)(&Ih264dynamicparams));
			writeWatchDog();

			if(hVe1 == NULL) {
				gblEncodeunLcok();
				ERR_PRN("Failed to create video encoder: %s\n", envp->videoEncoder);
				cleanup(THREAD_FAILURE);
			}

			isFirstFrame = 1;
			DEBUG(DL_DEBUG, "restart create encode libary!!!\n");
			gblEncodeunLcok();
		}

#endif
		iis_force = IISForceIframe(&gVAenc_p.venc);

		if(iis_force) {
			PRINTF("I will control the big video encode,the rate = %ld.old=%ld\n", gVAenc_p.venc.bitrate * 1000, Ih264dynamicparams.videncDynamicParams.targetBitRate);

			if(gVAenc_p.venc.bitrate * 1000 > MAXBITRATE) {
				Ih264dynamicparams.videncDynamicParams.targetBitRate = MAXBITRATE;
			}	else {
				Ih264dynamicparams.videncDynamicParams.targetBitRate =  gVAenc_p.venc.bitrate * 1000;
			}

			Ih264dynamicparams.videncDynamicParams.intraFrameInterval   = app_get_video_gop(HIGH_VIDEO);
			Ih264dynamicparams.videncDynamicParams.forceFrame = IVIDEO_I_FRAME ;
			Venc1_Control(hVe1, (VIDENC1_DynamicParams *)(&Ih264dynamicparams));
			iis_force = 0;
		}

		CalcEncodeDynParam(&recreate, &Ih264params, &Ih264dynamicparams);
		//cur_outpar = getOutputvideohandle();
		//PRINTF("Iframeinterval = %d,%d\n", pre_outpar->IFrameinterval, cur_outpar->IFrameinterval);
		//setOutputParame(cur_outpar, pre_outpar);
		/*real time check video encode param changes*/
		paramRet = VencRunTimeFxn(&gVAenc_p.venc, &Ih264params, &Ih264dynamicparams);

		//PRINTF("gVAenc_p.venc.rate = %u,=%u\n", gVAenc_p.venc.bitrate, gVAenc_p.aenc.bitrate);
	revenc:

		if(--recreate >= 0 || paramRet == 1 || (pre_mode != mode && mode != -1)) {
			PRINTF("recreate=%d,paramRet=%d,pre_mode=%d,=%d\n", recreate, paramRet, pre_mode, mode);
			pre_mode = mode;
			DEBUG(DL_DEBUG, "step 1, I will begin to restart video encode================%d,%d\n", recreate, paramRet);
			writeWatchDog();
			gblEncodeLcok();

			if(hVe1) {
				Venc1_delete(hVe1);
				hVe1 = NULL;
			}

			writeWatchDog();

			DEBUG(DL_DEBUG, "video encode input width = %d,height= %d,gResizeP.dst_h=%d,gResizeP.dst_w=%d\n", (int)(Ih264dynamicparams.videncDynamicParams.inputWidth), (int)(Ih264dynamicparams.videncDynamicParams.inputHeight), gResizeP.dst_h, gResizeP.dst_w);
			hVe1 = Venc1_create(hEngine, envp->videoEncoder,
			                    (VIDENC1_Params *)(&Ih264params),
			                    (VIDENC1_DynamicParams *)(&Ih264dynamicparams));
			writeWatchDog();

			if(hVe1 == NULL) {
				gblEncodeunLcok();
				ERR_PRN("Failed to create video encoder: %s\n", envp->videoEncoder);
				cleanup(THREAD_FAILURE);
			}

			isFirstFrame = 1;
			DEBUG(DL_DEBUG, "step 2,restart create encode libary!!!\n");
			gblEncodeunLcok();
		}

		mode = gblGetMode();

		if((mode < 0))  {		//source is connected!
			gVideoProcessFlag = 2;
			pthread_mutex_lock(&gCapEnc_m);
			gVideoProcessFlag = 7;

			if(Venc1_process(hVe1, hBlueBuffer, hDstBuf) < 0) {
				pthread_mutex_unlock(&gCapEnc_m);
				gVideoProcessFlag = 3;
				DEBUG(DL_ERROR, "[Video High] Failed to encode video buffer\n");
				//cleanup(THREAD_FAILURE);
				recreate = 1;
				goto revenc;
			}

			pthread_mutex_unlock(&gCapEnc_m);
			gVideoProcessFlag = 3;
			ret = BufferGfx_getFrameType(hBlueBuffer);
		}	 else {
			if(AppIsResizeFlag() && ISZOOMIN) {
				scaleInArgs.inBufSize  = 1920 * 1088 * 3 / 2;
				scaleInArgs.outBufSize = 1920 * 1088 * 3 / 2;


			//	static int g_test= 0;
			//	g_test ++;
			//	if(g_test%30 == 0)
			//	write_yuv_422(hCapBuf,1920,1080);

				dim.width = H264Width();
				dim.height = H264Height();
				BufferGfx_setDimensions(hCapBuf, &dim);
				
				SetScaleParam(&scale_parames, capture_width, capture_height, gResizeP.x, gResizeP.y, gResizeP.dst_w, gResizeP.dst_h);
				SCALE_control(scalerHandle, ISCALE_SETPARAMS, &(scale_parames));

				if(gResizeP.black > 0) {
					__BlackFill(hBuftmp, blue_w, blue_h);

					gResizeP.black--;
					PRINTF("gResizeP.black=%d,\n", gResizeP.black);
				}

				gblprocessLock();
			//	BufferGfx_getDimensions(hCapBuf, &indim);
			//	BufferGfx_getDimensions(hBuftmp, &outdim);
				
				ret = SCALE_process(scalerHandle, Buffer_getUserPtr(hCapBuf), Buffer_getUserPtr(hBuftmp), &scaleInArgs, &scaleOutArgs);
				gblprocessunLock();

				gVideoProcessFlag = 4;
				pthread_mutex_lock(&gCapEnc_m);
				BufferGfx_getDimensions(hBuftmp, &dim);
				
		//		PRINTF("=%d=%d=%d=%d,H264Width =%d, H264Height = %d\n", indim.width, indim.height, outdim.width, outdim.height, dim.width, dim.height);

				if(Venc1_process(hVe1, hBuftmp, hDstBuf) < 0) {
					pthread_mutex_unlock(&gCapEnc_m);
					gVideoProcessFlag = 5;
					DEBUG(DL_ERROR, "[Video High] mode : [%d] Failed to encode video buffer\n", mode);
					//cleanup(THREAD_FAILURE);
					recreate = 1;
					goto revenc;
				}


				pthread_mutex_unlock(&gCapEnc_m);
				gVideoProcessFlag = 6;
				ret = BufferGfx_getFrameType(hBuftmp);



			} else {
				gVideoProcessFlag = 4;
				pthread_mutex_lock(&gCapEnc_m);
				BufferGfx_getDimensions(hCapBuf, &dim);




				dim.width = H264Width();
				dim.height = H264Height();
				BufferGfx_setDimensions(hCapBuf, &dim);
			//	PRINTF("H264Width =%d, H264Height = %d\n", dim.width, dim.height);

				if(Venc1_process(hVe1, hCapBuf, hDstBuf) < 0) {
					pthread_mutex_unlock(&gCapEnc_m);
					gVideoProcessFlag = 5;
					DEBUG(DL_ERROR, "[Video High] mode : [%d] Failed to encode video buffer\n", mode);
					//cleanup(THREAD_FAILURE);
					recreate = 1;
					goto revenc;
				}

				pthread_mutex_unlock(&gCapEnc_m);
				gVideoProcessFlag = 6;
				ret = BufferGfx_getFrameType(hCapBuf);
			}

		}




		Buffer_setInterval(hDstBuf, sound_time);

		Buffer_setTimeStamp(hDstBuf, ret);

		//PRINTF("Ih264dynamicparams.videncDynamicParams.inputWidth =%d,=%d\n",Ih264dynamicparams.videncDynamicParams.inputWidth,Ih264dynamicparams.videncDynamicParams.inputHeight);
		Buffer_setWidthHeight(hDstBuf, Ih264dynamicparams.videncDynamicParams.inputWidth, Ih264dynamicparams.videncDynamicParams.inputHeight);

		if(ret == IVIDEO_I_FRAME) {
			//printf("[videoThrFxn] IVIDEO_I_FRAME IVIDEO_I_FRAMEIVIDEO_I_FRAMEIVIDEO_I_FRAME\n");
			if(isFirstFrame) {
				Buffer_setTimeStamp(hDstBuf, IVIDEO_IDR_FRAME);
				isFirstFrame = 0;
			}
		}

		/* Send encoded buffer to writer thread for filesystem output */
		if(Fifo_put(envp->to_writer, hDstBuf) < 0) {
			DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

		/* Return buffer to capture thread */
		if(Fifo_put(envp->to_capture, hCapBuf) < 0) {
			DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

		gblIncVideoBytesProcessed(Buffer_getNumBytesUsed(hDstBuf));
#if 1
		frameCnt++;

		if(frameCnt >= MAX_FRAMES_RECREATE)		{
			recreate  = 1;
			frameCnt = 0;
		}

#endif
	}

cleanup:
	DEBUG(DL_DEBUG, "[videoThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX fifoRet [%d]\n", fifoRet);
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	Rendezvous_force(envp->hRendezvousWriter);
	Fifo_flush(envp->from_writer);
	Fifo_flush(envp->to_capture);
	/* Make sure the other threads aren't waiting for init to complete */
	Rendezvous_meet(envp->hRendezvousCleanup);

	/* Clean up the thread before exiting */
	if(hBufTab) {
		BufTab_delete(hBufTab);
	}

	if(hVe1) {
		Venc1_delete(hVe1);
		hVe1 = NULL;
	}

	if(hEngine) {
		Engine_close(hEngine);
		hEngine = NULL;
	}

	if(hBlueBuffer) {
		Buffer_delete(hBlueBuffer);
	}

	if(pre_outpar) {
		free(pre_outpar);
		pre_outpar = NULL;
	}

	DEBUG(DL_DEBUG, "[videoThrFxn] cleanup XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX end\n");
	return status;
}

/*是否重建低码流*/
int IISRecreateEncode(IH264FHDVENC_DynamicParams *dynparams, BufferGfx_Dimensions *dim)
{
	int height = dynparams->videncDynamicParams.inputHeight;
	int width = dynparams->videncDynamicParams.inputWidth;

	if(dim->width != width || dim->height != height || H264_ENC_PARAM_FLAG == gVAenc_p.lowvenc.uflag) {
		gVAenc_p.lowvenc.uflag = 0;
		DEBUG(DL_DEBUG, "dim->width = %d dim->height = %d\n", (int)dim->width, (int)dim->height);

		if(dim->width > LOWRATE_MAX_WIDTH || dim->height > LOWRATE_MAX_HEIGHT) {
			DEBUG(DL_DEBUG, "[MAX] width = %d height = %d \n ", (int)dynparams->videncDynamicParams.inputWidth,
			      (int)dynparams->videncDynamicParams.inputHeight);
			return 0;
		} else if(dim->width  < LOWRATE_MIN_WIDTH || dim->height < LOWRATE_MIN_HEIGHT) {
			DEBUG(DL_DEBUG, "[MIN] width = %d height = %d \n ", (int)dynparams->videncDynamicParams.inputWidth,
			      (int)dynparams->videncDynamicParams.inputHeight);
			return 0;
		} else {
			dynparams->videncDynamicParams.inputHeight = dim->height;
			dynparams->videncDynamicParams.inputWidth = dim->width;
			DEBUG(DL_DEBUG, "width = %d height = %d \n ", (int)dynparams->videncDynamicParams.inputWidth,
			      (int)dynparams->videncDynamicParams.inputHeight);
		}

		dynparams->videncDynamicParams.targetBitRate = gVAenc_p.lowvenc.bitrate * 1024;

		if(dynparams->videncDynamicParams.targetBitRate > 5000000) {
			dynparams->videncDynamicParams.targetBitRate = 5000000;
		}

		//       DEBUG(DL_DEBUG,"dynparams->videncDynamicParams.targetBitRate = %d\n",dynparams->videncDynamicParams.targetBitRate);
		return 1;
	}

	return 0;

}

/*
 *****************************************************************************
 * videoLowRateThrFxn
 *****************************************************************************
*/
Void *videoLowRateThrFxn(Void *arg)
{
	PRINTF("get pid= %d\n", getpid());
	DEBUG(DL_DEBUG, "[videoLowRateThrFxn] start ...\n");
	VideoEnv               *envp                = (VideoEnv *) arg;
	Void                   *status              = THREAD_SUCCESS;
	BufferGfx_Attrs         gfxAttrs            = BufferGfx_Attrs_DEFAULT;
	Venc1_Handle            hVe1                = NULL;
	Engine_Handle           hEngine             = NULL;
	BufTab_Handle           hBufTab             = NULL;
	Int                     frameCnt            = 0;
	Buffer_Handle           hCapBuf, hDstBuf;
	Int                     fifoRet;
	Int                     bufIdx, recreate = -1;
	IH264FHDVENC_Params				Ih264params;
	IH264FHDVENC_VUIDataStructure	Ih264vui_params;
	IH264FHDVENC_DynamicParams		Ih264dynamicparams;
	int                     isFirstFrame = 1, iis_force = 0;
	BufferGfx_Dimensions	srcDim;//dstDim;
	int ret = 0;
	Buffer_Handle           hBufTabResize	= NULL;

	/*  创建蓝屏buf   */
	int blue_w = LOWRATE_MAX_WIDTH, blue_h = LOWRATE_MAX_HEIGHT;
	hBufTabResize = CreateBuffer(blue_w, blue_h, ColorSpace_YUV420PSEMI);

	if(hBufTabResize == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	ParseBlueScreen(hBufTabResize, blue_w, blue_h);
	DEBUG(DL_DEBUG, "[videoLowRateThrFxn] befor Engine_open\n");
	hEngine = Engine_open(envp->engineName, NULL, NULL);

	if(hEngine == NULL)  {
		DEBUG(DL_ERROR, "Failed to open codec engine %s\n", envp->engineName);
		cleanup(THREAD_FAILURE);
	}

	DEBUG(DL_DEBUG, "[videoLowRateThrFxn] befor Engine_open\n");
	/* 设置默认编码参数 */
	SetH264ParamTable(&Ih264params, &Ih264dynamicparams, &Ih264vui_params);
	StartH264ParamTable(&Ih264params, &Ih264dynamicparams);

	//修改编码buffer
	envp->imageWidth  = LOWRATE_MAX_WIDTH;
	envp->imageHeight = LOWRATE_MAX_HEIGHT;
	Ih264params.videncParams.rateControlPreset  = IVIDEO_LOW_DELAY;
	Ih264dynamicparams.videncDynamicParams.targetBitRate = 1024000;

	Ih264params.videncParams.maxHeight = envp->imageHeight;
	Ih264params.videncParams.maxWidth  = envp->imageWidth;
	Ih264dynamicparams.videncDynamicParams.inputHeight = envp->imageHeight;
	Ih264dynamicparams.videncDynamicParams.inputWidth  = envp->imageWidth;
	Ih264dynamicparams.videncDynamicParams.intraFrameInterval   = app_get_video_gop(LOW_VIDEO);

	DEBUG(DL_DEBUG, "[videoLowRateThrFxn] height = %d  width = %d\n",
	      (int)Ih264dynamicparams.videncDynamicParams.inputHeight,
	      (int)Ih264dynamicparams.videncDynamicParams.inputWidth);

	/*  创建H264 编码库  */
	hVe1 = Venc1_create(hEngine, envp->videoEncoder,
	                    (VIDENC1_Params *)(&Ih264params),
	                    (VIDENC1_DynamicParams *)(&Ih264dynamicparams));
	isFirstFrame = 1;

	if(hVe1 == NULL) {
		DEBUG(DL_ERROR, "[videoLowRateThrFxn] Failed to create video encoder: %s\n", envp->videoEncoder);
		cleanup(THREAD_FAILURE);
	}

	// 给writer线程用的
	envp->outBufSize = Venc1_getOutBufSize(hVe1);
	Rendezvous_meet(envp->hRendezvousWriter);    /* 同步write线程*/

	gfxAttrs.colorSpace = ColorSpace_YUV420PSEMI;
	gfxAttrs.dim.width = envp->imageWidth;
	gfxAttrs.dim.height = envp->imageHeight;
	gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(gfxAttrs.dim.width,
	                          gfxAttrs.colorSpace);
	hBufTab = BufTab_create(NUM_VIDEO_BUFS, Venc1_getInBufSize(hVe1),
	                        BufferGfx_getBufferAttrs(&gfxAttrs));

	if(hBufTab == NULL) {
		DEBUG(DL_ERROR, "Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	/* for循环把创建的Buf依次发送到采集线程*/
	for(bufIdx = 0; bufIdx < NUM_VIDEO_BUFS; bufIdx++) {
		fifoRet = Fifo_put(envp->to_resize, BufTab_getBuf(hBufTab, bufIdx));

		if(fifoRet < 0) {
			DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

		//		__BlackFill(BufTab_getBuf(hBufTab, bufIdx),LOWRATE_MAX_WIDTH,LOWRATE_MAX_HEIGHT);
	}

	BufferGfx_getDimensions(BufTab_getBuf(hBufTab, 0), &srcDim);
	Rendezvous_meet(envp->hRendezvousInit);

	/*线程主循环*/
	while(!gblGetQuit()) {
		//       DEBUG(DL_DEBUG,"videoLowRateThrFxn :step 000\n");
		/*从采集线程获取一个buf*/
		fifoRet = Fifo_get(envp->from_resize, &hCapBuf);

		if(fifoRet < 0) {
			DEBUG(DL_ERROR, "Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		/* Did the capture thread flush the fifo? */
		if(fifoRet == Dmai_EFLUSH) {
			cleanup(THREAD_SUCCESS);
		}

		//        DEBUG(DL_DEBUG,"videoLowRateThrFxn :step 001\n");
		/* 从发送线程获取一个buf */
		fifoRet = Fifo_get(envp->from_writer, &hDstBuf);

		if(fifoRet < 0) {
			DEBUG(DL_ERROR, "Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		/* Did the writer thread flush the fifo? */
		if(fifoRet == Dmai_EFLUSH) {
			cleanup(THREAD_SUCCESS);
		}

		iis_force = IISForceIframe(&gVAenc_p.lowvenc);

		if(iis_force) {
			PRINTF("I will control the low video encode,the rate = %ld.old=%ld\n", gVAenc_p.lowvenc.bitrate * 1024, Ih264dynamicparams.videncDynamicParams.targetBitRate);
			Ih264dynamicparams.videncDynamicParams.targetBitRate =  gVAenc_p.lowvenc.bitrate * 1024;
			Ih264dynamicparams.videncDynamicParams.forceFrame = IVIDEO_I_FRAME ;
			Venc1_Control(hVe1, (VIDENC1_DynamicParams *)(&Ih264dynamicparams));
			iis_force = 0;
		}

		BufferGfx_getDimensions(hCapBuf, &srcDim);
		recreate = IISRecreateEncode(&Ih264dynamicparams, &srcDim);
	revenc:

		if(recreate) {
			PRINTF("step 1,[low] Restart create start!!! bitrate = %d \n\n",
			       (int)Ih264dynamicparams.videncDynamicParams.targetBitRate);
			writeWatchDog();
			gblEncodeLcok();
			PRINTF("step 2,[low] Restart create start!!! bitrate = %d \n\n",
			       (int)Ih264dynamicparams.videncDynamicParams.targetBitRate);

#if 1

			if(hVe1) {
				Venc1_delete(hVe1);
				hVe1 = NULL;
			}

			writeWatchDog();
			PRINTF("step 3,[low] Restart create start!!! bitrate = %d \n\n",
			       (int)Ih264dynamicparams.videncDynamicParams.targetBitRate);
			hVe1 = Venc1_create(hEngine, envp->videoEncoder,
			                    (VIDENC1_Params *)(&Ih264params),
			                    (VIDENC1_DynamicParams *)(&Ih264dynamicparams));
			writeWatchDog();

			if(hVe1 == NULL) {
				gblEncodeunLcok();
				ERR_PRN("Failed to create video encoder: %s\n", envp->videoEncoder);
				cleanup(THREAD_FAILURE);
			}

#endif
			/*
			            ret = Venc1_SetDynParam(hVe1,(VIDENC1_DynamicParams*)(&Ih264dynamicparams));
			            if(ret != Dmai_EOK) {
			            	gblEncodeunLcok();
			                DEBUG(DL_ERROR,"Failed to Set Dynamic Params : %s\n", envp->videoEncoder);
			                cleanup(THREAD_FAILURE);
			            }
			*/
			isFirstFrame = 1;
			recreate = 0;
			PRINTF("step 4,[low] restart create encode libary!!!\n");
			gblEncodeunLcok();
			PRINTF("step 5,[low] restart create encode libary!!!\n");
		}

		int mode = gblGetMode();
		gblprocessLock();

		if(mode < 0)  	{		//无源
			if(Venc1_process(hVe1, hBufTabResize, hDstBuf) < 0)	{
				//	DEBUG(DL_ERROR,"[Video Low] Failed to encode video buffer\n");
				//eanup(THREAD_FAILURE);
				recreate = 1;
				gblprocessunLock();
				goto revenc;
			}

			ret = BufferGfx_getFrameType(hBufTabResize);//--蓝屏类型
		} else 	{
			//DEBUG(DL_DEBUG,"[Video Low]###################################### [%d]:[%d]\n", Ih264dynamicparams.videncDynamicParams.inputHeight ,Ih264dynamicparams.videncDynamicParams.inputWidth);
			//有源
			if(Venc1_process(hVe1, hCapBuf, hDstBuf) < 0)	{
				DEBUG(DL_ERROR, "[Video Low] Failed to encode video buffer\n");
				//eanup(THREAD_FAILURE);
				recreate = 1;
				gblprocessunLock();
				goto revenc;
			}

			ret = BufferGfx_getFrameType(hCapBuf);//--采集类型
		}

		gblprocessunLock();
#if 0

		//--设置buffer类型
		if(mode < 0)	{
			ret = BufferGfx_getFrameType(hBufTabResize);//--蓝屏类型
		} else	{
			ret = BufferGfx_getFrameType(hCapBuf);//--采集类型
		}

#endif
		//		DEBUG(DL_DEBUG,"videoLowRateThrFxn :step 004\n");
		Buffer_setTimeStamp(hDstBuf, ret);

		if(ret == IVIDEO_I_FRAME) 	{
			if(isFirstFrame)	{
				Buffer_setTimeStamp(hDstBuf, IVIDEO_IDR_FRAME);
				isFirstFrame = 0;
			}
		}

		//        DEBUG(DL_DEBUG,"videoLowRateThrFxn :step 005\n");
		/* 发送编码后的Buf到发送线程*/
		fifoRet = Fifo_put(envp->to_writer, hDstBuf) ;

		if(fifoRet < 0) {
			DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

		//        DEBUG(DL_DEBUG,"videoLowRateThrFxn :step 006\n");
		/* 还回Buf给采集线程 */
		fifoRet = Fifo_put(envp->to_resize, hCapBuf) ;

		if(fifoRet < 0) {
			DEBUG(DL_ERROR, "Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

		//        DEBUG(DL_DEBUG,"videoLowRateThrFxn :step 007\n");
		/* Increment statistics for the user interface */
		//gblIncVideoBytesProcessed(Buffer_getNumBytesUsed(hDstBuf));
		frameCnt++;

		if(frameCnt >= MAX_FRAMES_RECREATE)	 {
			recreate  = 1;
			frameCnt = 0;
		}
	}

cleanup:
	DEBUG(DL_ERROR, "[Video Low] cleanup -------------------------------!!!\n");
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	Fifo_flush(envp->to_writer);
	Fifo_flush(envp->to_capture);
	/* Make sure the other threads aren't waiting for init to complete */
	Rendezvous_meet(envp->hRendezvousCleanup);

	/* Clean up the thread before exiting */
	if(hBufTab) {
		BufTab_delete(hBufTab);
	}

	if(hVe1) {
		Venc1_delete(hVe1);
		hVe1 = NULL;
	}

	if(hEngine) {
		Engine_close(hEngine);
		hEngine = NULL;
	}

	if(hBufTabResize) {
		Buffer_delete(hBufTabResize);
	}

	return status;
}

/*resize 420 data*/
int Resize420YUV(Resize_Handle hRsz, Buffer_Handle hSrcBuf, Buffer_Handle hDstResize)
{
	BufferGfx_Dimensions    srcDim, dstDim, oldDim;
	ResizeParam *resize = &gLowRate;
	int width  = 0 , height = 0 ;

	width = resize->dst_w ;
	height = resize->dst_h;
	BufferGfx_getDimensions(hSrcBuf, &srcDim);
	srcDim.width = H264Width();
	srcDim.height = H264Height();
	BufferGfx_setDimensions(hSrcBuf, &srcDim);

	BufferGfx_getDimensions(hDstResize, &dstDim);
	BufferGfx_getDimensions(hDstResize, &oldDim);

	/*width and height */
	if(width >= srcDim.width) {
		width = srcDim.width;//--不变
		height = srcDim.height;
	}

	if(height >= srcDim.height) {
		width = srcDim.width;//--不变
		height = srcDim.height;
	}

	dstDim.width = width & (~1);
	dstDim.height = height & (~1);
	BufferGfx_setDimensions(hDstResize, &dstDim);//-设置buffer的宽高

	//PRINTF("source width=%d,he=%d,ds w=%d,h=%d\n",srcDim.width ,srcDim.height, dstDim.width,dstDim.height );

	/*resize the buffer to width height*/
	if(Resize_config(hRsz, hSrcBuf, hDstResize) < 0)  {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to configure resize job\n");
		return -1;
	}

	//    DEBUG(DL_DEBUG,"Enter Resize---------------------execute width = %d height= %d\n",dstDim.width,dstDim.height);
	/* Resize the captured frame to the resolution of the display frame */
	if(Resize_execute(hRsz, hSrcBuf, hDstResize) < 0) {
		DEBUG(DL_ERROR, "[captureThrFxn] Failed to execute resize job\n");
		return -1;
	}

	BufferGfx_getDimensions(hDstResize, &dstDim);//-设置buffer的宽高
	//    DEBUG(DL_DEBUG,"Exit Resize---------------------execute width = %d height= %d\n",dstDim.width,dstDim.height);
	return 0;
}

/*
 *****************************************************************************
 * ResizeLowThrFxn
 *****************************************************************************
*/
Void *ResizeLowThrFxn(Void *arg)
{
	PRINTF("get pid= %d\n", getpid());
	DEBUG(DL_DEBUG, "[ResizeLowThrFxn] start ...\n");
	VideoEnv               *envp                = (VideoEnv *) arg;
	Void                   *status              = THREAD_SUCCESS;
	BufferGfx_Attrs         gfxAttrs            = BufferGfx_Attrs_DEFAULT;
	Resize_Handle   		hRsz				= NULL;
	Resize_Attrs            rszAttrs        	= Resize_Attrs_DEFAULT;
	BufTab_Handle           hBufTab             = NULL;
	Int                     bufIdx				= -1;
	Int                     fifoRet;
	Buffer_Handle           hCapBuf, hDstBuf;


	/* Create the resize job */
	hRsz = Resize_create(&rszAttrs);

	if(hRsz == NULL) {
		ERR("[ResizeThread] Failed to create resize job\n");
		cleanup(THREAD_FAILURE);
	}

	envp->imageWidth = H264_MAX_ENCODE_WIDTH;
	envp->imageHeight = H264_MAX_ENCODE_HIGHT + 8;;
	gfxAttrs.colorSpace = ColorSpace_YUV420PSEMI;
	gfxAttrs.dim.width = envp->imageWidth;
	gfxAttrs.dim.height = envp->imageHeight;
	gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(gfxAttrs.dim.width,
	                          gfxAttrs.colorSpace);
	hBufTab = BufTab_create(NUM_VIDEO_BUFS, envp->imageWidth * envp->imageHeight * 1.5,
	                        BufferGfx_getBufferAttrs(&gfxAttrs));

	if(hBufTab == NULL) {
		ERR("[ResizeThread] Failed to allocate contiguous buffers\n");
		cleanup(THREAD_FAILURE);
	}

	/* Send buffers to the capture thread to be ready for main loop */
	for(bufIdx = 0; bufIdx < NUM_VIDEO_BUFS; bufIdx++) {
		fifoRet = Fifo_put(envp->to_capture, BufTab_getBuf(hBufTab, bufIdx));

		if(fifoRet < 0) {
			ERR("[ResizeThread] Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}
	}

	DEBUG(DL_DEBUG, "[Low Resize] =====================================\n");
	/* Signal that initialization is done and wait for other threads */
	Rendezvous_meet(envp->hRendezvousInit);

	while(!gblGetQuit()) {
		/* Get a buffer to encode from the capture thread */
		fifoRet = Fifo_get(envp->from_capture, &hCapBuf);//--hCapBuf是capture里来的数据

		if(fifoRet < 0) {
			ERR("[ResizeThread] Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		/* Did the capture thread flush the fifo? */
		if(fifoRet == Dmai_EFLUSH) {
			cleanup(THREAD_SUCCESS);
		}

		/* Get a buffer to encode to from the writer thread */
		fifoRet = Fifo_get(envp->from_videoresize_c, &hDstBuf);

		if(fifoRet < 0) {
			ERR("[ResizeThread] Failed to get buffer from video thread\n");
			cleanup(THREAD_FAILURE);
		}

		/* Did the writer thread flush the fifo? */
		if(fifoRet == Dmai_EFLUSH) {
			cleanup(THREAD_SUCCESS);
		}

		int ret_resize = 0;
		/*resize 420 YUV data*/
		ret_resize = Resize420YUV(hRsz, hCapBuf, hDstBuf);

		if(ret_resize < 0) {
			cleanup(THREAD_FAILURE);
		}

		/* Send encoded buffer to writer thread for filesystem output */
		fifoRet = Fifo_put(envp->to_videoresize_c, hDstBuf) ;

		if(fifoRet < 0) {
			ERR("[ResizeThread] Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}

		/* Return buffer to capture thread */
		fifoRet = Fifo_put(envp->to_capture, hCapBuf);

		if(fifoRet < 0) {
			ERR("[ResizeThread] Failed to send buffer to display thread\n");
			cleanup(THREAD_FAILURE);
		}
	}

cleanup:
	DEBUG(DL_DEBUG, "[ResizeThread] cleanup ------------------------------!!!\n");
	/* Make sure the other threads aren't waiting for us */
	Rendezvous_force(envp->hRendezvousInit);
	Rendezvous_force(envp->hRendezvousWriter);
	Fifo_flush(envp->to_videoresize_c);
	Fifo_flush(envp->to_capture);

	/* Make sure the other threads aren't waiting for init to complete */
	Rendezvous_meet(envp->hRendezvousCleanup);

	/* Clean up the thread before exiting */
	if(hBufTab) {
		BufTab_delete(hBufTab);
	}

	return status;
}

int app_get_encode_rate(int *vrate, int *arate , int *quality)
{
	*vrate = (gVAenc_p.venc.bitrate) ;
	*arate = gVAenc_p.aenc.bitrate / 1000;
	* quality = (gVAenc_p.venc.cbr);
	return 0;
}

