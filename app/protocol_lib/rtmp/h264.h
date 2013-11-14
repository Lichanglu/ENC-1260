#define TCP_BUF_LEN                     1500  
#define TCP_SEND_DATA_LEN               1400
#define HAND_SHAKE_FIX_LEN              1536
#define CHUNK_HEAD_LEN_TYPE_ZERO        12
#define CHUNK_HEAD_LEN_TYPE_ONE         8
#define CHUNK_HEAD_LEN_TYPE_TWO         4
#define CHUNK_HEAD_LEN_TYPE_THREE       1
#define RTMP_TCP_LEN                    1460
#define CHUNK_LEN                       0x4000
#define CLIENT_MAX                      6
#define TRUE		                    1
#define FALSE		                    0


#define RTMP_VERSION                    "1.0.2" //rtmp∞Ê±æ∫≈


typedef unsigned short WORD ;
typedef unsigned int   DWORD ;
typedef unsigned char  BYTE;

typedef struct strRtmpDataParam
{
    unsigned char                       aucOnMetaData[8];
    char                                aucOnMetaDataName[0x10];
    unsigned char                       aucTrackInfo[8];
    char                                aucTrackInfoName[0x10];
    unsigned char                       aucTimeScale[8];
    char                                aucTimeScaleName[0x10];
    unsigned char                       aucLength[8];
    char                                aucLengthName[0x10];
    unsigned short                      ucLanguageLen;
    char                                aucLanguage[0x20];
    char                                aucLanguageName[0x10];
    unsigned char                       aucSampleDescription[8];
    char                                aucSampleDescriptionName[0x20];
    unsigned short                      ucSampleTypeLen;
    char                                aucSampleType[0x20];
    char                                aucSampleTypeName[0x10];
    unsigned char                       aucVideoFramerate[8];
    char                                aucVideoFraterateName[0x20];
    unsigned char                       aucAacAot[8];
    char                                aucAacAotName[0x10];
    unsigned char                       aucAvcLevel[8];
    char                                aucAvcLevelName[0x10];
    unsigned char                       aucAvcProfile[8];
    char                                aucAvcProfileName[0x10];
    unsigned short                      ucVideoCodecIdLen;
    char                                aucVideoCodecId[0x20];
    char                                aucVideoCodecIdName[0x10];
    unsigned char                       aucWidth[8];
    char                                aucWidthName[0x10];
    unsigned char                       aucHeight[8];
    char                                aucHeightName[0x10];
    unsigned char                       aucMoovPositon[8];
    char                                aucMoovPosionName[0x10];
    unsigned char                       aucDuration[8];
    char                                aucDurationName[0x10];
    unsigned char                       aucBytes[8];
    char                                aucBytesName[0x10];
}STR_RTMP_DATA_PARAM;


typedef struct strRtmpParam
{
    char                                    aucResult[8];
    char                                    aucResultName[0x10];
    unsigned short                          ucFmsVerLen;
    char                                    aucFmsVer[0x20];
    char                                    aucFmsVerName[0x10];
    unsigned char                           aucCapabilities[8];
    char                                    aucCapabilitiesName[0x10];
    unsigned char                           aucMode[8];
    char                                    aucModeName[0x10];
    unsigned char                           ucLevelLen;
    char                                    aucLevel[0x10];
    char                                    aucLevelName[0x10];
    unsigned short                          ucCodeLen;
    char                                    aucCode[0x20];
    char                                    aucCodeName[0x20];
    unsigned short                          ucDescriptionLen;
    char                                    aucDescription[0x30];
    char                                    aucDescriptionName[0x10];
    char                                    aucData[8];
    char                                    aucDataName[0x10];
    unsigned short                          ucVersionLen;
    char                                    aucVersion[0x10];
    char                                    aucVersionName[0x10];
#ifdef WOWZA
    unsigned char                           aucClientId[8];
#else
    char                                    aucClientId[0x20];
    unsigned char                           ucClientidLen;
#endif
    char                                    aucClientIdName[0x10];
    unsigned char                           aucObjectEnc[8];
    char                                    aucObjectEncName[0x10];
    unsigned char                           aucOnStatus[8];
    char                                    aucOnStatusName[0x10];
    unsigned char                           boolIsFastPlay;
    char                                    aucIsFastPlayName[0x10];
    unsigned char                           boolRtmpSampleAccess;
    char                                    aucRtmpSampleAccessName[0x20];
    STR_RTMP_DATA_PARAM                     strData;
    unsigned char                           ucReasonLen;
    char                                    aucReason[0x10];
    char                                    aucReasonName[0x10];
    unsigned char                           ucDetailsLen;
    char                                    aucDetails[0x40];
    char                                    aucDetailsName[0x10];
    unsigned char                           ucServerLen;
    char                                    aucServer[0x40];
    char                                    aucServerName[0x10];
}STR_RTMP_PARAM;

enum{
    ChunkSize=1,
    Abort,
    ByteRead,
    Ping,
    ServerBw,
    ClientBw,
    Unknow,
    AudioData,
    VideoData,
    FlexStreamSend=0xf,
    FlexSharedObject,
    FlexMessage,
    Notify,
    ShareObject,
    Invoke,   
    };

enum AMF
{
    /**
     * Boolean value marker constant
     */
    TYPE_NUMBER = 0,

    /**
     * Boolean value marker constant
     */
    TYPE_BOOLEAN = 0x01,

    /**
     * String marker constant
     */
    TYPE_STRING = 0x02,

    /**
     * Object marker constant
     */
    TYPE_OBJECT = 0x03,

    /**
     * Movieclip marker constant
     */
    TYPE_MOVIECLIP = 0x04 ,

    /**
     * Null marker constant
     */
    TYPE_NULL = 0x05,     //∏√¿‡–Õ∫ÛΩ”2∏ˆ8BYTESÂ?

    /**
     * Undefined marker constant
     */
    TYPE_UNDEFINED = 0x06,

    /**
     * Object reference marker constant
     */
    TYPE_REFERENCE = 0x07,

    /**
     * Mixed array marker constant
     */
    TYPE_MIXED_ARRAY = 0x08,

    /**
     * End of object marker constant
     */
    TYPE_END_OF_OBJECT = 0x09,

    /**
     * Array marker constant
     */
    TYPE_ARRAY = 0x0A,

    /**
     * Date marker constant
     */
    TYPE_DATE = 0x0B,

    /**
     * Long string marker constant
     */
    TYPE_LONG_STRING = 0x0C,

    /**
     * Unsupported type marker constant
     */
    TYPE_UNSUPPORTED = 0x0D,

    /**
     * Recordset marker constant
     */
    TYPE_RECORDSET = 0x0E,

    /**
     * XML marker constant
     */
    TYPE_XML = 0x0F,

    /**
     * Class marker constant
     */
    TYPE_CLASS_OBJECT = 0x10,

    /**
     * Object marker constant (for AMF3)
     */
    TYPE_AMF3_OBJECT = 0x11,

    /**
     * true marker constant
     */
    VALUE_TRUE = 0x01,
    /**
     * false marker constant
     */
    VALUE_FALSE = 0x00
};

enum
{
	CHUNK_HEAD_12_BYTES=0,
	CHUNK_HEAD_8_BYTES,
	CHUNK_HEAD_4_BYTES,
	CHUNK_HEAD_1_BYTES
};

enum
{
	VIDEO_RESOLUTION_480P=0,
	VIDEO_RESOLUTION_720P,
	VIDEO_RESOLUTION_1080P,
	VIDEO_RESOLUTION_VGA,
	VIDEO_RESOLUTION_DEFAULT
};


typedef struct _RMSPClientData
{
	int bUsed;
    int login;
	int ServerSocket; //tcp serversocket
	int ClientSocket; //tcp clientsocket
    struct sockaddr_in Serveraddr;  //tcp serveraddr
    struct sockaddr_in Clientaddr;  //tcp clientaddr
}RTMPClientData;

typedef struct __RTMPCliParam
{
	RTMPClientData rtmpcliDATA[CLIENT_MAX]; //client param infomation
}RTMPCliParam;

//--------------------------------------------rtsp--------------------------------------------------
/*to see if rtmp socket is used or not*/
#define ISRTMPUSED(dsp,cli)				    (gRTMPCliPara[dsp].rtmpcliDATA[cli].bUsed == TRUE)

/*set rtmp client used*/
#define SETRTMPUSED(dsp,cli,val)		        (gRTMPCliPara[dsp].rtmpcliDATA[cli].bUsed = val)

/*to see if rtmp socket is ready or not*/
#define ISRTMPREADY(dsp,cli)				    (gRTMPCliPara[dsp].rtmpcliDATA[cli].login== TRUE)

/*set rtmp client ready*/
#define SETRTMPREADY(dsp,cli,val)		        (gRTMPCliPara[dsp].rtmpcliDATA[cli].login = val)

/*get rtmp socket of server*/
#define GETRTMPSERVERSOCK(dsp,cli)			(gRTMPCliPara[dsp].rtmpcliDATA[cli].ServerSocket)

/*set trmp socket of server*/
#define SETRTMPSERVERSOCK(dsp,cli,val)		(gRTMPCliPara[dsp].rtmpcliDATA[cli].ServerSocket = val)

/*get rtmp sockaddr of server*/
#define GETRTMPSERVERADDR(dsp,cli)			(gRTMPCliPara[dsp].rtmpcliDATA[cli].Serveraddr)

/*set rtmp sockaddr of server*/
#define SETRTMPSERVERADDR(dsp,cli,val)		(gRTMPCliPara[dsp].rtmpcliDATA[cli].Serveraddr= val)

/*get rtmp socket of server*/
#define GETRTMPCLIENTSOCK(dsp,cli)			(gRTMPCliPara[dsp].rtmpcliDATA[cli].ClientSocket)

/*set trmp socket of server*/
#define SETRTMPCLIENTSOCK(dsp,cli,val)		(gRTMPCliPara[dsp].rtmpcliDATA[cli].ClientSocket= val)

/*get rtmp sockaddr of client*/
#define GETRTMPCLIENTADDR(dsp,cli)			(gRTMPCliPara[dsp].rtmpcliDATA[cli].Clientaddr)

/*set rtmp sockaddr of client*/
#define SETRTMPCLIENTADDR(dsp,cli,val)		(gRTMPCliPara[dsp].rtmpcliDATA[cli].Clientaddr= val)
