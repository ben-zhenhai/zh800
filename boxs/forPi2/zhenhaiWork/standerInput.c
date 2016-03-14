#include "standerInput.h"

void * InputFunction(void *argument)
{
    char tempBarcodeInput[INPUTLENGTH];
    while(1)
    {
        int arrayCount = 0;
        int stringLength;
        unsigned char flagFailPass = 0;
        memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
        gets(tempBarcodeInput);
        printf("get input string: %s\n", tempBarcodeInput);
        stringLength = strlen(tempBarcodeInput);

        //pthread_mutex_lock(&MutexInput);
        switch(BarcodeIndex)
        {
            case ISNO:
                if(strlen(tempBarcodeInput) == 14)
                {
                    memset(ISNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(ISNo, tempBarcodeInput, sizeof(char)*INPUTLENGTH);

                    int x = ReadLastFinishLotNo();
                    if(x == 0 && strcmp(ISNo, LastISNo) == 0)
                    {
                        printf("Need input managerment No\n");
                        BarcodeIndex = NEEDPRIVILEGE;
                    }else
                    {
                        memset(LastISNo, 0, sizeof(char)*INPUTLENGTH);
                    }
                    InputDone = 1;
                }
                else if(strncmp(tempBarcodeInput, "XXXM", 4) == 0 && CanChangeRepairModeFlag == 1)
                {
                    char *tempPtr = tempBarcodeInput + 4;
                    memset(RepairNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(RepairNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));
                    CanChangeRepairModeFlag = 2;
                    InputDone = 1;
                    printf("get repair event\n");
                }
                else
                {
                    printf("scan ISNo. error\n");
                }
                break;
            case MANAGERCARD:
                if(strlen(tempBarcodeInput) == 24)
                {
                        memset(ManagerCard, 0, sizeof(char)*INPUTLENGTH);
                        memcpy(ManagerCard, tempBarcodeInput, sizeof(char)*INPUTLENGTH);
                        InputDone = 1;
                }
                else
                {
                    printf("scan ManagerCard error\n");
                }

                break;
            case USERNO:
                if(strncmp(tempBarcodeInput, "XXXP", 4) == 0)
                {
                    char *tempPtr = tempBarcodeInput + 4;
                    memset(UserNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(UserNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));
                    InputDone = 1;
                }
                else if(strncmp(tempBarcodeInput, "XXXM", 4) == 0 && CanChangeRepairModeFlag == 1)
                {
                    char *tempPtr = tempBarcodeInput + 4;
                    memset(RepairNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(RepairNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));
                    CanChangeRepairModeFlag = 2;
                    InputDone = 1;
                }
                else
                {
                    printf("scan UserNo. error!\n");
                }

                break;
            case COUNTNO:
                while(arrayCount < stringLength)
                {
                    if(tempBarcodeInput[arrayCount] == '0') ;
                    else if(tempBarcodeInput[arrayCount] == '1');
                    else if(tempBarcodeInput[arrayCount] == '2');
                    else if(tempBarcodeInput[arrayCount] == '3');
                    else if(tempBarcodeInput[arrayCount] == '4');
                    else if(tempBarcodeInput[arrayCount] == '5');
                    else if(tempBarcodeInput[arrayCount] == '6');
                    else if(tempBarcodeInput[arrayCount] == '7');
                    else if(tempBarcodeInput[arrayCount] == '8');
                    else if(tempBarcodeInput[arrayCount] == '9');
                    else 
                    {
                        flagFailPass = 1;
                        break;
                    }
                    ++arrayCount;
                }
                if(flagFailPass == 0 && stringLength > 0)
                {
                    memset(CountNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(CountNo, tempBarcodeInput, sizeof(char)*INPUTLENGTH);
                    long goodCount = (atol(CountNo));
                    if(goodCount > 0)
                    {
                        printf("need finish: %ld\n", goodCount);
                        InputDone = 1;
                    }
                }
                else
                {
                    printf("scan Count error!\n");
                }
                break;
            case FIXITEM:
                if(strncmp(tempBarcodeInput, "UUU", 3) == 0)
                {
                    char *tempPtr = tempBarcodeInput + 3;
                    memset(FixItemNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(FixItemNo, tempPtr, sizeof(char)*(INPUTLENGTH-3));
                    CanChangeRepairModeFlag = 3;    
                    InputDone = 1;
                }else if(strncmp(tempBarcodeInput, "XXXM", 4) == 0 && CanChangeRepairModeFlag == 3)
                {
                        char *tempPtr = tempBarcodeInput + 4;
                        memset(RepairNo, 0, sizeof(char)*INPUTLENGTH);
                        memcpy(RepairNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));
                        CanChangeRepairModeFlag = 4;    
                        InputDone = 1;
                }else if(strncmp(tempBarcodeInput, "XXXM", 4) == 0 && CanChangeRepairModeFlag != 3)
                {
                    printf("need input Fix item first!\n");
                }
                else 
                {
                    printf("scan Fix Item NO. error!\n");
                }
                break;
            case  NEEDPRIVILEGE:
                if(strncmp(tempBarcodeInput, "XXXA", 4) == 0)
                {
                    char *tempPtr = tempBarcodeInput + 4;
                    memset(PrivilegeNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(PrivilegeNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));
                    BarcodeIndex = ISNO;
                    InputDone = 1;
                }else
                {
                    BarcodeIndex = 0;
                    memset(ISNo, 0, sizeof(char)*INPUTLENGTH);
                    InputDone = 1;
                }
                break;
            default:
                printf("nothing\n");
        }
        //pthread_mutex_unlock(&MutexInput);
    }
    printf("we leave\n\n\n");
}

