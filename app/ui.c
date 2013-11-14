/*
 * ui.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#include <stdlib.h>

#include <xdc/std.h>

#include <ti/sdo/dmai/Buffer.h>

#include <ti/sdo/simplewidget/Font.h>
#include <ti/sdo/simplewidget/Text.h>
#include <ti/sdo/simplewidget/Screen.h>

#include "demo.h"
#include "ui.h"

#define COL_1                   10
#define COL_2                   310

#define FONT_NAME               "data/fonts/Amiga_Forever.ttf"
#define FONT_SIZE               12

#define MIN_TRANSPARENCY        0
#define MAX_TRANSPARENCY        0xff
#define NORMAL_TRANSPARENCY     0xaa
#define STEP_TRANSPARENCY       0x8
#define INC_TRANSPARENCY        0x22

#define MAX_STRING_LENGTH       50

typedef enum {
    UI_BufType_WORK = 0,
    UI_BufType_DISPLAY,
    UI_BufType_NUMTYPES
} UI_Buftype;

typedef struct UI_Object {
	Buffer_Handle       hBufs[UI_BufType_NUMTYPES];
	pthread_mutex_t     mutex;
	Int                 displayIdx;
	Int                 workingIdx;
	Font_Handle         hFont;
	UInt8               transparency;
	Int                 fadeUp;
	Int                 fadeDown;
	Int                 osd;
} UI_Object;

typedef struct UI_String_Object {
	Int                 y;
	Int                 valid;
	Char                col1String[MAX_STRING_LENGTH];
	Char                col2String[MAX_STRING_LENGTH];
} UI_String_Object;

static UI_String_Object stringAttrs[UI_Num_Values] = {
	{ UI_Row_1,  FALSE, "Demo Name"         }, /* Demo name */                \
	{ UI_Row_2,  FALSE, "ARM Load:"         }, /* ARM load */                 \
	{ UI_Row_3,  FALSE, "DSP Load:"         }, /* DSP load */                 \
	{ UI_Row_4,  FALSE, "Display Type:"     }, /* Display type */             \
	{ UI_Row_5,  FALSE, "Video Codec:"      }, /* Video Codec */              \
	{ UI_Row_6,  FALSE, "Video fps:"        }, /* Video FPS */                \
	{ UI_Row_7,  FALSE, "Video bit rate:"   }, /* Video bitrate */            \
	{ UI_Row_8,  FALSE, "Video resolution:" }, /* Image resolution */         \
	{ UI_Row_9,  FALSE, "Sound codec:"      }, /* Sound codec */              \
	{ UI_Row_10, FALSE, "Sound bit rate:"   }, /* Sound bitrate */            \
	{ UI_Row_11, FALSE, "Sampling freq:"    }, /* Sound sampling frequency */ \
	{ UI_Row_12, FALSE, "Time:"             }, /* Time */                     \
};

/******************************************************************************
 * drawValue
 ******************************************************************************/
static Void drawValue(UI_Handle hUI, UI_Value type, Buffer_Handle hBuf)
{
	Int x;
	Char *ptr;

	if(type == UI_Value_DemoName) {
		x = COL_1;
		ptr = stringAttrs[type].col1String;
	} else {
		x = COL_2;
		ptr = stringAttrs[type].col2String;
	}

	if(Text_show(hUI->hFont, ptr, FONT_SIZE, x,
	             stringAttrs[type].y, hBuf) < 0) {
		ERR("Failed to show text\n");
	}
}

/******************************************************************************
 * cleanupUI
 ******************************************************************************/
static Int cleanupUI(UI_Handle hUI)
{
	Int ret;

	if(hUI->hFont) {
		ret = Font_delete(hUI->hFont);
	}

	if(hUI->hBufs[UI_BufType_WORK]) {
		ret = Buffer_delete(hUI->hBufs[UI_BufType_WORK]);
	}

	if(hUI->hBufs[UI_BufType_DISPLAY]) {
		ret = Buffer_delete(hUI->hBufs[UI_BufType_DISPLAY]);
	}

	pthread_mutex_destroy(&hUI->mutex);

	free(hUI);

	return ret;
}

/******************************************************************************
 * UI_create
 ******************************************************************************/
UI_Handle UI_create(UI_Attrs *attrs)
{
	BufferGfx_Attrs     gfxAttrs      = BufferGfx_Attrs_DEFAULT;
	UI_Handle           hUI;
	Int                 bufSize;

	hUI = calloc(1, sizeof(UI_Object));

	if(hUI == NULL) {
		ERR("Failed to allocate space for UI Object\n");
		return NULL;
	}

	pthread_mutex_init(&hUI->mutex, NULL);
	hUI->workingIdx = 0;
	hUI->displayIdx = 1;
	hUI->osd = attrs->osd;

	if(attrs->osd) {
		hUI->hFont = Font_create(FONT_NAME);

		if(hUI->hFont == NULL) {
			ERR("Failed to create UI font\n");
			cleanupUI(hUI);
			return NULL;
		}

		gfxAttrs.dim.width      = attrs->width;
		gfxAttrs.dim.height     = attrs->height;
		gfxAttrs.dim.lineLength = BufferGfx_calcLineLength(attrs->width,
		                          ColorSpace_2BIT);
		gfxAttrs.colorSpace     = ColorSpace_2BIT;

		bufSize = gfxAttrs.dim.lineLength * gfxAttrs.dim.height;

		hUI->hBufs[UI_BufType_WORK] =
		    Buffer_create(bufSize, (Buffer_Attrs *) &gfxAttrs);

		if(hUI->hBufs[UI_BufType_WORK] == NULL) {
			ERR("Failed to allocate bitmap buffer of size %d\n", bufSize);
			cleanupUI(hUI);
			return NULL;
		}

		Screen_clear(hUI->hBufs[UI_BufType_WORK], 0, 0,
		             attrs->width, attrs->height);

		hUI->hBufs[UI_BufType_DISPLAY] =
		    Buffer_create(bufSize, (Buffer_Attrs *) &gfxAttrs);

		if(hUI->hBufs[UI_BufType_DISPLAY] == NULL) {
			ERR("Failed to allocate bitmap buffer of size %d\n", bufSize);
			cleanupUI(hUI);
			return NULL;
		}

		Screen_clear(hUI->hBufs[UI_BufType_DISPLAY],
		             0, 0, attrs->width, attrs->height);
	}

	return hUI;
}

/******************************************************************************
 * UI_init
 ******************************************************************************/
Int UI_init(UI_Handle hUI)
{
	Int screenNbr;
	Int cnt;

	if(hUI->osd) {
		for(screenNbr = 0; screenNbr < UI_BufType_NUMTYPES; screenNbr++) {
			for(cnt = 0; cnt < UI_Num_Values; cnt++) {
				if(stringAttrs[cnt].y >= 0) {
					if(Text_show(hUI->hFont,
					             stringAttrs[cnt].col1String,
					             FONT_SIZE,
					             COL_1,
					             stringAttrs[cnt].y,
					             hUI->hBufs[screenNbr]) < 0) {
						ERR("Failed to show text\n");
						return FAILURE;
					}
				}
			}
		}
	}

	UI_show(hUI);

	return SUCCESS;
}

/******************************************************************************
 * UI_show
 ******************************************************************************/
Void UI_show(UI_Handle hUI)
{
	hUI->fadeUp = TRUE;
	hUI->fadeDown = FALSE;
}

/******************************************************************************
 * UI_hide
 ******************************************************************************/
Void UI_hide(UI_Handle hUI)
{
	hUI->fadeUp = FALSE;
	hUI->fadeDown = TRUE;
}

/******************************************************************************
 * UI_incTransparency
 ******************************************************************************/
Void UI_incTransparency(UI_Handle hUI)
{
	if(hUI->transparency < MAX_TRANSPARENCY) {
		if(hUI->transparency + INC_TRANSPARENCY > MAX_TRANSPARENCY) {
			hUI->transparency = MAX_TRANSPARENCY;
		} else {
			hUI->transparency += INC_TRANSPARENCY;
		}
	}

	hUI->fadeUp = FALSE;
	hUI->fadeDown = FALSE;
}

/******************************************************************************
 * UI_decTransparency
 ******************************************************************************/
Void UI_decTransparency(UI_Handle hUI)
{
	if(hUI->transparency > MIN_TRANSPARENCY) {
		if(hUI->transparency - INC_TRANSPARENCY < MIN_TRANSPARENCY) {
			hUI->transparency = MIN_TRANSPARENCY;
		} else {
			hUI->transparency -= INC_TRANSPARENCY;
		}
	}

	hUI->fadeUp = FALSE;
	hUI->fadeDown = FALSE;
}

/******************************************************************************
 * UI_toggleVisibility
 ******************************************************************************/
Void UI_toggleVisibility(UI_Handle hUI)
{
	if(hUI->fadeUp || hUI->transparency > MIN_TRANSPARENCY) {
		hUI->fadeUp = FALSE;
		hUI->fadeDown = TRUE;
	} else {
		hUI->fadeUp = TRUE;
		hUI->fadeDown = FALSE;
	}
}

/******************************************************************************
 * UI_getTransparency
 ******************************************************************************/
UInt8 UI_getTransparency(UI_Handle hUI)
{
	return hUI->transparency;
}

/******************************************************************************
 * UI_updateValue
 ******************************************************************************/
Void UI_updateValue(UI_Handle hUI, UI_Value type, Char *valString)
{
	Char *ptr;

	if(type == UI_Value_DemoName) {
		ptr = stringAttrs[type].col1String;
	} else {
		ptr = stringAttrs[type].col2String;
	}

	if(stringAttrs[type].y >= 0) {
		strncpy(ptr, valString, MAX_STRING_LENGTH);
		stringAttrs[type].valid = TRUE;
	}
}

/******************************************************************************
 * UI_setRow
 ******************************************************************************/
Void UI_setRow(UI_Handle hUI, UI_Value type, UI_Row row)
{
	stringAttrs[type].y = row;
}

/******************************************************************************
 * UI_lockScreen
 ******************************************************************************/
Buffer_Handle UI_lockScreen(UI_Handle hUI)
{
	pthread_mutex_lock(&hUI->mutex);
	return hUI->hBufs[hUI->displayIdx];
}

/******************************************************************************
 * UI_unlockScreen
 ******************************************************************************/
Void UI_unlockScreen(UI_Handle hUI)
{
	if(hUI->fadeUp) {
		if(hUI->transparency < NORMAL_TRANSPARENCY) {
			if(hUI->transparency + STEP_TRANSPARENCY > NORMAL_TRANSPARENCY) {
				hUI->transparency = NORMAL_TRANSPARENCY;
			} else {
				hUI->transparency += STEP_TRANSPARENCY;
			}
		} else {
			hUI->transparency = NORMAL_TRANSPARENCY;
			hUI->fadeUp = FALSE;
		}
	}

	if(hUI->fadeDown) {
		if(hUI->transparency > MIN_TRANSPARENCY) {
			if(hUI->transparency - STEP_TRANSPARENCY < MIN_TRANSPARENCY) {
				hUI->transparency = MIN_TRANSPARENCY;
			} else {
				hUI->transparency -= STEP_TRANSPARENCY;
			}
		} else {
			hUI->transparency = MIN_TRANSPARENCY;
			hUI->fadeDown = FALSE;
		}
	}

	pthread_mutex_unlock(&hUI->mutex);
}

/******************************************************************************
 * UI_eraseData
 ******************************************************************************/
Void UI_eraseData(UI_Handle hUI)
{
	BufferGfx_Dimensions dim;

	if(hUI->osd) {
		BufferGfx_getDimensions(hUI->hBufs[hUI->workingIdx], &dim);

		Screen_clear(hUI->hBufs[hUI->workingIdx], COL_2, 0,
		             dim.width - COL_2, dim.height);
	}
}

/******************************************************************************
 * UI_update
 ******************************************************************************/
Void UI_update(UI_Handle hUI)
{
	Int i;

	for(i = 0; i < UI_Num_Values; i++) {
		if(stringAttrs[i].y >= 0 && stringAttrs[i].valid) {
			if(hUI->osd) {
				drawValue(hUI, i, hUI->hBufs[hUI->workingIdx]);
			} else {
				if(i == 0) {
					printf("%s ", stringAttrs[i].col1String);
				} else {
					printf("%s %s ", stringAttrs[i].col1String,
					       stringAttrs[i].col2String);
				}
			}
		}
	}

	if(hUI->osd) {
		pthread_mutex_lock(&hUI->mutex);
		hUI->workingIdx = (hUI->workingIdx + 1) % UI_BufType_NUMTYPES;
		hUI->displayIdx = (hUI->displayIdx + 1) % UI_BufType_NUMTYPES;
		pthread_mutex_unlock(&hUI->mutex);
	} else {
		printf("\n\n");
	}
}

/******************************************************************************
 * UI_delete
 ******************************************************************************/
Int UI_delete(UI_Handle hUI)
{
	Int ret = SW_EOK;

	if(hUI) {
		ret = cleanupUI(hUI);
	}

	return ret;
}
