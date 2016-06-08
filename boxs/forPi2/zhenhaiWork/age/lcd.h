//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"

#define CSCHANNEL 0
#define SPICLOCK 100000

int ScreenIndex;
char DisableUpDown;

void * ChangeScreenEventListenFunction(void *argument);
int UpdateScreenFunction(int screenIndex, int status);
int SendCommandMessageFunction(unsigned char *massage, int arrayLength);
