

#include "middle_control.h"
#define LOGOCONFIGNAME "logo_1260.ini"

#define LOGOFILE "/opt/reach/logo.png"


#define	LOGOHEIGHT		64
#define	LOGOWIDTH		64

typedef struct LogoInfo *Logo_Handle;

extern Logo_Handle initLogoMod();
extern void showLogo(Logo_Handle lh, Buffer_Handle hCapBuf, int mode);
extern int reloadLogo(Logo_Handle lh);
extern int setLogoParm(Logo_Handle lh, unsigned char *buf);
extern void delLogoMod(Logo_Handle lh);
extern int UploadImage(int sSocket, char *data, unsigned char *logoname);

