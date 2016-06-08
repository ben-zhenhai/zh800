#include "lock.h"


int LockMachineFunction()
{
    //printf("into Lock function\n");
    digitalWrite(ZHPIN15, HIGH);
    digitalWrite(ZHPIN16, HIGH);
    //printf("leave Lock function\n");
    return 0;
}

int UnlockMachineFunction()
{
    //printf("into unlock function\n");
    digitalWrite(ZHPIN15, LOW);
    digitalWrite(ZHPIN16, LOW);
    //printf("leave unlock function\n");
    return 0;
}
