//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"
#include "eeprom.h"

enum
{
    ISNO = 1,
    MANAGERCARD,
    COUNTNO,
    USERNO,
    SCANDONE,
    REPAIRMODE,
    REPAIRDONE,
    FIXITEM,
    NEEDPRIVILEGE
};

void *InputFunction(void *arguemtn);
