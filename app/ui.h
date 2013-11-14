/*
 * ui.h
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2009
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

#ifndef _UI_H
#define _UI_H

#include <xdc/std.h>

#include <ti/sdo/dmai/Buffer.h>

typedef enum {
    UI_Row_NONE  = -1,
    UI_Row_1     = 10,
    UI_Row_2     = 35,
    UI_Row_3     = 60,
    UI_Row_4     = 85,
    UI_Row_5     = 110,
    UI_Row_6     = 135,
    UI_Row_7     = 160,
    UI_Row_8     = 185,
    UI_Row_9     = 210,
    UI_Row_10    = 235,
    UI_Row_11    = 260,
    UI_Row_12    = 285,
} UI_Row;

typedef enum {
    UI_Value_DemoName = 0,
    UI_Value_ArmLoad,
    UI_Value_DspLoad,
    UI_Value_DisplayType,
    UI_Value_VideoCodec,
    UI_Value_Fps,
    UI_Value_VideoKbps,
    UI_Value_ImageResolution,
    UI_Value_SoundCodec,
    UI_Value_SoundKbps,
    UI_Value_SoundFrequency,
    UI_Value_Time,
    UI_Num_Values
} UI_Value;

typedef struct UI_Attrs {
	Int             osd;
	Int             width;
	Int             height;
} UI_Attrs;

typedef struct UI_Object *UI_Handle;

#if defined (__cplusplus)
extern "C" {
#endif

	extern UI_Handle UI_create(UI_Attrs *attrs);

	extern Int UI_init(UI_Handle hUI);

	extern Void UI_updateValue(UI_Handle hUI, UI_Value type, Char *valString);

	extern Void UI_setRow(UI_Handle hUI, UI_Value type, UI_Row row);

	extern Void UI_show(UI_Handle hUI);

	extern Void UI_hide(UI_Handle hUI);

	extern Void UI_toggleVisibility(UI_Handle hUI);

	extern UInt8 UI_getTransparency(UI_Handle hUI);

	extern Void UI_decTransparency(UI_Handle hUI);

	extern Void UI_incTransparency(UI_Handle hUI);

	extern Buffer_Handle UI_lockScreen(UI_Handle hUI);

	extern Void UI_unlockScreen(UI_Handle hUI);

	extern Void UI_eraseData(UI_Handle hUI);

	extern Void UI_update(UI_Handle hUI);

	extern Int UI_delete(UI_Handle hUI);

#if defined (__cplusplus)
}
#endif

#endif /* _UI_H */
