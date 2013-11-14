#ifndef _APP_ADJUST_GREEN_H__
#define _APP_ADJUST_GREEN_H__

int app_get_green_adjust_value(int digval, int mode, int *value);
void app_set_green_adjust_value(int digval, int mode, int value);
void app_init_green_adjust_module(void);

int app_get_screen_adjust_value(int digval, int mode, int *value);
void app_set_screen_adjust_value(int digval, int mode, int value);
void app_init_screen_adjust_module(void);

#endif
