//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"
#include "lcd.h"
enum
{
    ISNO = 0,
    MANAGERCARD,
    USERNO,
    COUNTNO,
    SCANDONE,
    REPAIRMODE,
    REPAIRDONE,
    FIXITEM
};

unsigned char InputMode;
//unsigned char CanChangeRepairModeFlag;
// Can not into         : 0
// Can into             : 1
// in the repair mode   : 2
// Can leave            : 3

void *InputFunction(void *argument);
void *BarcodeFunction(void *argument);
