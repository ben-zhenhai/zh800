//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"

#define CSCHANNEL 0
//#define SPICLOCK 1000
#define SPICLOCK 100000

int ScreenIndex;
//only in Menu can use button up and down
char DisableUpDown;
char isInPairMode;

void * ChangeScreenEventListenFunction(void *argument);
int UpdateScreenFunction(int screenIndex);
int SendCommandMessageFunction(unsigned char *massage, int arrayLength);
