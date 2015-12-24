#include "standerInput.h"

void * InputFunction(void *argument)
{
    char tempBarcodeInput[INPUTLENGTH];
    struct timeval now;

    while(1)
    {
        switch(InputMode)
        {
            //user change
            case 1:
            DisableUpDown = 1;
            ScreenIndex = 7;
            UpdateScreenFunction(7);
            while(1)
            {
                memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
                gets(tempBarcodeInput);
                if(strncmp(tempBarcodeInput, "XXXP", 4) == 0)
                {
                    char *tempPtr = tempBarcodeInput+4;
                    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
                    pthread_mutex_lock(&MutexLinklist);
                    InputNode *tempNode = ZHList;
                    while(tempNode != NULL)
                    {
                        memset(tempNode->UserNo, 0, sizeof(char)*INPUTLENGTH);
                        memcpy(tempNode->UserNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));

                        printf("%s %s %s %s %s\n", tempNode->ISNo, tempNode->ManagerCard, tempNode->UserNo, tempNode->CountNo, 
                                                           tempNode->UploadFilePath);    
                        tempNode = tempNode->link;
                    }
                    pthread_mutex_unlock(&MutexLinklist);
                    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
                    sleep(0);
                    break;
                }
                else printf("[%s|%d]scan UserNo. error!\n", __func__, __LINE__);
            }
            InputDone = 1;
            InputMode = 0;
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);
            break;
 
            default:
            ZHNode = (InputNode *)malloc(sizeof(InputNode));
            if(ZHNode == NULL)
            {
                sleep(1);
                continue;
            }
            memset(ZHNode->ISNo, 0, sizeof(char)*INPUTLENGTH);
            memset(ZHNode->ManagerCard, 0, sizeof(char)*INPUTLENGTH);
            memset(ZHNode->UserNo, 0, sizeof(char)*INPUTLENGTH);
            memset(ZHNode->CountNo, 0, sizeof(char)*INPUTLENGTH);
            memset(ZHNode->UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
            memset(ZHNode->GoodNo, 0, sizeof(char)*INPUTLENGTH);
            ZHNode->link = NULL;
 
            printf("Ready to work!!\n");
            memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
            gets(tempBarcodeInput);

            if(OrderInBox >= 2)
            {
                sleep(1);
                free(ZHNode);
                printf("can not input over 2\n");
                continue;
            }
            while(1)
            {
                //memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
                //gets(tempBarcodeInput);

                if(strlen(tempBarcodeInput) == 14)
                {
                    memset(ZHNode->ISNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(ZHNode->ISNo, tempBarcodeInput, sizeof(tempBarcodeInput));
                    //pthread_mutex_lock(&MutexNetworkStatus);
                    //StopUpdateNetworkStatus = 1;
                    //pthread_mutex_unlock(&MutexNetworkStatus);

                    break;
                }
                printf("Scan Lot Number error\n");
                memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
                gets(tempBarcodeInput);
            }
            DisableUpDown = 1;
            ScreenIndex = 6;
            UpdateScreenFunction(6);
            while(1)
            {
                memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
                gets(tempBarcodeInput);

                if(strlen(tempBarcodeInput) == 24)
                {
                    memset(ZHNode->ManagerCard, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(ZHNode->ManagerCard, tempBarcodeInput, sizeof(tempBarcodeInput));
                    break;
                }
                DisableUpDown = 1;
                ScreenIndex = 6;
                UpdateScreenFunction(6);

                printf("Scan Part Number error\n");
            }
            DisableUpDown = 1;
            ScreenIndex = 6;
            UpdateScreenFunction(6);
            while(1)
            {
                int arrayCount = 0;
                int stringLength;
                unsigned char flagFailPass = 0;
                memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
                gets(tempBarcodeInput);
                stringLength = strlen(tempBarcodeInput);
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
                    memset(ZHNode->CountNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(ZHNode->CountNo, tempBarcodeInput, sizeof(char)*INPUTLENGTH);
                    long goodCount = (atol(ZHNode->CountNo));
                    if(goodCount > 0)
                    {
                        printf("need finish: %ld\n", goodCount);
                        //LED
                        break;
                    }
                }
                DisableUpDown = 1;
                ScreenIndex = 6;
                UpdateScreenFunction(6);
                printf("scan Count error!\n");
            }
            DisableUpDown = 1;
            ScreenIndex = 6;
            UpdateScreenFunction(6);
            while(1)
            {
                memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
                gets(tempBarcodeInput);
                if(strncmp(tempBarcodeInput, "XXXP", 4) == 0)
                {
                    char *tempPtr = tempBarcodeInput + 4;
                    memset(ZHNode->UserNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(ZHNode->UserNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));
                    //LED
                    break;
                }
                DisableUpDown = 1;
                ScreenIndex = 6;
                UpdateScreenFunction(6);

                printf("scan UserNo. error!\n");
            }
            DisableUpDown = 1;
            ScreenIndex = 6;
            UpdateScreenFunction(6);

            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            pthread_mutex_lock(&MutexLinklist);
            if(ZHList == NULL)
            {
                ZHList = ZHNode;
                OrderInBox = OrderInBox + 1;
                //first node need create file first
                memset(ZHList->UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
                gettimeofday(&now, NULL);
                sprintf(ZHList->UploadFilePath,"%ld%s.txt",(long)now.tv_sec, MachineNo);
                printf("[%s|%d]new file name:%s\n", __func__, __LINE__, ZHList->UploadFilePath);
                InputDone = 1;
            }else
            {
                InputNode *tempNode = ZHList;
                while(tempNode->link != NULL)
                {
                    tempNode = tempNode->link;
                }
                tempNode->link = ZHNode;
                OrderInBox = OrderInBox +1;
            }
            pthread_mutex_unlock(&MutexLinklist);
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            sleep(0);
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);
        }
    }
}

