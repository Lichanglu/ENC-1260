#include "common.h"
#ifdef DSS_ENC_1100_1200
extern void GetDeviceType(char *dtype);
extern int SetDeviceType(char *config_file, const char *dtype);
extern int ReadDeviceType(char *config_file, int againFlag);
#endif
