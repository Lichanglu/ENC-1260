#include <ti/sdo/ce/topvision/extensions/scale/scale.h>

extern int scale_create(Engine_Handle hEngine, Char *codecName, SCALE_Params *params);

extern int scale_setparam(int iwidth, int iheight, int cropirow, int cropicol, int cropiwidth, int cropiheight, int cropvuoffset, int owidth, int oheight, int desktoprow , int desktopcol, int desktopwidth, int desktopheight, int upsampleout , int sampleMode);

extern int scale_process(Buffer_Handle hInBuf, Buffer_Handle hOutBuf, SCALE_Params *params);

extern int scale_delete();
