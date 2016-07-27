#include "lock.h"

//鎖機 method
//因為機台在gpio 持續拉HIGH 的情況下, 會造成機台異常而需斷電
//因此我們作法是一直送Pulse

// gpio 拉 high
int LockMachineFunction()
{
    //printf("into Lock function\n");
    digitalWrite(ZHPIN15, HIGH);
    digitalWrite(ZHPIN16, HIGH);
    //printf("leave Lock function\n");
    return 0;
}

// gpio 拉 low
int UnlockMachineFunction()
{
    //printf("into unlock function\n");
    digitalWrite(ZHPIN15, LOW);
    digitalWrite(ZHPIN16, LOW);
    //printf("leave unlock function\n");
    return 0;
}
