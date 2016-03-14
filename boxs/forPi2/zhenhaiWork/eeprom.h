//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"
#define EEPROMADDRESS 0x50

int ZHEarseRepairStage();
int WriteRepairStageEEPROMDATA();
int IsResumeFromRepair();
int ReadLastFinishLotNo();
int WriteLastFinishLotNo();
int ZHEarseEEPEOMData();
int ReadEEPROMData();
int WriteEEPROMData();
