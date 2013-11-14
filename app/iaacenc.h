/*! *************************************************************************
 * \file:     iaacenc.h
 * \brief     Header file for xDM interface of AAC Encoder
 * \date  :   19-Jan-2006
 * purpose:   This file gives the xDM interface files for aac-plus encoder
 * contents:  Interface structures for xDM interface of AAC Encoder
 ***************************************************************************/

#ifndef _IAACENC_H_
#define _IAACENC_H_

#include <ti/xdais/dm/xdm.h>
#include <ti/xdais/dm/iaudio.h>
#include <ti/xdais/dm/iaudenc1.h>



/*! Control Commands for AAC Encoder */
#define IAACENC_GETSTATUS   XDM_GETSTATUS
#define IAACENC_SETPARAMS   XDM_SETPARAMS
#define IAACENC_RESET       XDM_RESET
#define IAACENC_SETDEFAULT  XDM_SETDEFAULT
#define IAACENC_FLUSH       XDM_FLUSH
#define IAACENC_GETBUFINFO  XDM_GETBUFINFO
#define IAACENC_GETVERSION  XDM_GETVERSION

/*! \enum AACENC_OBJ_TYP
 *  \brief enumerated type for output format
*/
typedef enum {
    AACENC_OBJ_TYP_LC           = 2,   /*!< AAC Low Complexity  */
    AACENC_OBJ_TYP_HEAAC        = 5,   /*!< HE AAC              */
    AACENC_OBJ_TYP_PS           = 29   /*!< AAC LC + SBR + PS   */
} AACENC_OBJ_TYP;

/*! \enum  AACENC_TRANSPORT_TYPE
 *  \brief Enumerated type for output file format
 */
typedef enum {
    AACENC_TT_RAW    = 0,              /*!< Raw Output Format   */
    AACENC_TT_ADIF   = 1,              /*!< ADIF File Format    */
    AACENC_TT_ADTS   = 2               /*!< ADTS File Format    */
} AACENC_TRANSPORT_TYPE;

/*! \enum AACENC_BITRATE_MODE
 *  \brief Enumerated type for VBR bit-rate modes
 */
typedef enum {
    AACENC_BR_MODE_VBR_1  = 1,     /*!< VBR Mode 1            */
    AACENC_BR_MODE_VBR_2  = 2,     /*!< VBR Mode 2            */
    AACENC_BR_MODE_VBR_3  = 3,     /*!< VBR Mode 3            */
    AACENC_BR_MODE_VBR_4  = 4,     /*!< VBR Mode 4            */
    AACENC_BR_MODE_VBR_5  = 5      /*!< VBR Mode 5            */
} AACENC_VBR_MODE;

/*! \enum AACENC_BOOL_TYPE
 *  \brief Enumerated type for Boolean decision
 */
typedef enum {
    AACENC_FALSE = 0,     /*!< To indicate False  */
    AACENC_TRUE  = 1      /*!< To indicate True   */
} AACENC_BOOL_TYPE;

/* define constants to be used in AAC Encoder */
#define AACENC_FRAME_LEN_LONG 1024
#define AACENC_MAX_CHANNELS      2

/*!
 * \struct IAACENC_Obj
 * \brief  This structure must be the first field of all ENCODE instances.
 */
typedef struct IAACENC_Obj {
	struct IAACENC_Fxns *fxns;
} IAACENC_Obj;

typedef struct IAACENC_Obj *IAACENC_Handle;

/*!
 * \struct IAACENC_Status
 * \brief  This structure gives the current status of the AAC Encoder.
 */
typedef struct IAACENC_Status {
	IAUDENC1_Status  audenc_status;   /*!< Basic audio encoder status struct */
	AACENC_OBJ_TYP  outputObjectType; /*!< Output Object Type LC/HE/HEv2    */
	AACENC_TRANSPORT_TYPE outFileFormat; /*!< Output File Format            */

} IAACENC_Status;

/*!
 * \struct IAACENC_Params
 * \brief  This structure defines creation time parameters of AAC Encoder
 */
typedef struct IAACENC_Params {
	IAUDENC1_Params audenc_params;   /*!< Generic encoder creation parameters */
	AACENC_OBJ_TYP   outObjectType; /*!< Output Object Type LC/HE/HEv2       */
	AACENC_TRANSPORT_TYPE outFileFormat; /*!< Output File Format             */
	AACENC_BOOL_TYPE  useTns;       /*!< Flag for activating TNS feature     */
	AACENC_BOOL_TYPE  usePns;       /*!< Flag for activating PNS feature     */

	AACENC_BOOL_TYPE  downMixFlag;   /*!< downmixing Flag                   */
	AACENC_VBR_MODE bitRateMode; /*!< Flag for CBR / VBR coding          */

	/*! Replicated from Dynamic Params */

	XDAS_Int32        ancRate;      /*!< ancillary Data Rate                 */
} IAACENC_Params;

/*! Default Parameter values for AACENC instance objects */
extern IAACENC_Params IAACENC_PARAMS;

/*!
 * \struct IAACENC_DynamicParams
 * \brief  This structure defines run time parameters for AAC Encoder object
*/
typedef struct IAACENC_DynamicParams {
	IAUDENC1_DynamicParams audenc_dynamicparams;
	/*!< generic encoder dynamic parameters  */
	/* For RESET Command */
	AACENC_BOOL_TYPE  useTns;      /*!< Flag for activating TNS feature     */
	AACENC_BOOL_TYPE  usePns;      /*!< Flag for activating PNS feature     */

	AACENC_BOOL_TYPE  downMixFlag;   /*!< downmixing Flag                  */

	AACENC_BOOL_TYPE  ancFlag;      /*!< ancillary data Flag                */
	XDAS_Int32        ancRate;      /*!< ancillary Data Rate                */

} IAACENC_DynamicParams;

/*!
 * \struct IAACENC_InArgs
 * \brief  This structure gives the input parameters to the AAC Encoder
 * \brief  This will be passed to the encoder for each frame of input
 */
typedef struct IAACENC_InArgs {
	IAUDENC1_InArgs audenc_inArgs;  /*!< Generic Audio Encoder input Params */

} IAACENC_InArgs;

/*!
 * \struct IAACENC_OutArgs
 * \brief  This structure gives the output parameters of the AAC Encoder
 * \brief  This will be output by the encoder for each frame
 */
typedef struct IAACENC_OutArgs {
	IAUDENC1_OutArgs audenc_outArgs; /*!< Generic Audio Encoder output param */
} IAACENC_OutArgs;

/*!
 * \struct IAACENC_Fxns
 * \brief  This structure defines all of the operations on AAC Encoder object
 */
typedef struct IAACENC_Fxns {
	IAUDENC1_Fxns iaudenc;              /*!< must be first element of objects */
} IAACENC_Fxns;

typedef  IAUDENC1_Cmd  IAACENC_Cmd;

#endif /* _IAACENC_H_ */

