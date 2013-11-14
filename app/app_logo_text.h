#ifndef _APP_LOGO_TEXT_H_
#define _APP_LOGO_TEXT_H_

typedef enum __DisplayLogo_Text {
    NOdisplay = 0,
    OnlyShowText,
    OnlyShowLogo,
    BothShow,
} DisplayLogo_Text;



extern Logo_Handle getLogoInfoHandle(void);
extern int setLogoInfoHandle(Logo_Handle newLogoInfo);
extern int getLogoInfo(char *data, int vallen, char *outdata);
extern int setLogoInfo(char *data, int vallen, char *outdata);
extern int webupLoadLogo(char *indata, char *outdata);
extern int displayLogo(int data, int *out);
//extern int getShowLogoTextFlag(void);
//extern int setShowLogoTextFlag(DisplayLogo_Text in);
extern int webgetShowLogoTextFlag(DisplayLogo_Text in, DisplayLogo_Text *out);
extern int websetShowLogoTextFlag(DisplayLogo_Text in, DisplayLogo_Text *out);
#endif
