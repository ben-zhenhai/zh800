#include "standerInput2.h"

void * InputFunction(void *argument)
{
    struct timeval now;
    while(1)
    {
        switch(InputMode)
        {
            case 2:
            BarcodeIndex = USERNO;
            pthread_mutex_lock(&MutexInput);
            NewOrdering = 1;
            pthread_mutex_unlock(&MutexInput);
            DisableUpDown = 1;
            ScreenIndex = 7;
            UpdateScreenFunction(7, 0);
            BarcodeInputDone = 0;
            while(BarcodeInputDone == 0)
            {
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                if(BarcodeIndex != USERNO) break;
            }
            if(BarcodeIndex == USERNO)
            {
                InputMode = 0;
                BarcodeIndex = 0;
                InputDone = 1;
                DisableUpDown = 1;
                ScreenIndex = 0;
                UpdateScreenFunction(0, 0);
                pthread_mutex_lock(&MutexInput);
                NewOrdering = 0;
                pthread_mutex_unlock(&MutexInput);
            }
            break;
            //user change
            case 1:
            BarcodeIndex = USERNO;
            DisableUpDown = 1;
            ScreenIndex = 7;
            UpdateScreenFunction(7, 0);
            BarcodeInputDone = 0;
            while(BarcodeInputDone == 0)
            {
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                //if(BarcodeIndex != USERNO) break;
            }
            InputMode = 0;
            BarcodeIndex = 0;
            InputDone = 1;
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0, 0);
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
 
            printf("[%s]Ready to work!! the BarcodeIndex %d %p\n", __func__, BarcodeIndex, ZHNode);
            
            if(OrderInBox >= MAXORDER)
            {
                pthread_mutex_lock(&MutexLinklist);
                if(ZHNode != NULL)
                {
                    free(ZHNode);
                    ZHNode = NULL;
                }
                pthread_mutex_unlock(&MutexLinklist);
                printf("can not input over 2\n");
                sleep(10);
                continue;
            }
            BarcodeInputDone = 0;
            if(BarcodeIndex == 0)
            {
                BarcodeIndex = ISNO;
                while(BarcodeInputDone == 0)
                {
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                    if(BarcodeIndex != ISNO) break;
                }
            }

            BarcodeInputDone = 0;
            if(BarcodeIndex == ISNO)
            {
                pthread_mutex_lock(&MutexInput);
                NewOrdering = 1;
                pthread_mutex_unlock(&MutexInput);
                DisableUpDown = 1;
                ScreenIndex = 6;
                UpdateScreenFunction(6, 0);

                BarcodeIndex = MANAGERCARD;
                while(BarcodeInputDone == 0)
                {
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                    if(BarcodeIndex != MANAGERCARD) break;
                }
            }
            BarcodeInputDone = 0;
            if(BarcodeIndex == MANAGERCARD)
            {
                DisableUpDown = 1;
                ScreenIndex = 6;
                UpdateScreenFunction(6, 0);

                BarcodeIndex = COUNTNO;
                while(BarcodeInputDone == 0)
                {
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                    if(BarcodeIndex != COUNTNO) break;
                }
            }

            BarcodeInputDone = 0;
            if(BarcodeIndex == COUNTNO)
            {
                DisableUpDown = 1;
                ScreenIndex = 6;
                UpdateScreenFunction(6, 0);

                BarcodeIndex = USERNO;
                while(BarcodeInputDone == 0)
                {
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                    if(BarcodeIndex != USERNO) break; 
                }

                if(BarcodeIndex == USERNO)
                {
                    DisableUpDown = 1;
                    ScreenIndex = 6;
                    UpdateScreenFunction(6, 0);

                    BarcodeIndex = SCANDONE;
                } 
            }
            if(BarcodeIndex == SCANDONE)
            {
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
                    //BarcodeInputDone = 1;
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
                NewOrdering = 0;
                DisableUpDown = 1;
                ScreenIndex = 0;
                UpdateScreenFunction(0, 0);
                BarcodeIndex = 0;
            }else
            {
                //printf("hihihihi %p\n", ZHNode);
                pthread_mutex_lock(&MutexLinklist);
                if(ZHNode != NULL)
                {
                    free(ZHNode);
                    ZHNode = NULL;
                }
                pthread_mutex_unlock(&MutexLinklist);
            }
        }
    }
}

void * BarcodeInsertFunction(void * argument)
{
    char tempBarcodeInput[INPUTLENGTH];

    while(1)
    {
        memset(tempBarcodeInput, 0, sizeof(char)*INPUTLENGTH);
        gets(tempBarcodeInput);
        printf("get input string: %s\n", tempBarcodeInput);
        if(InputMode == 1 || InputMode == 2)
        {
            //user change
            if(strncmp(tempBarcodeInput, "XXXP", 4) == 0)
            {
                char *tempPtr = tempBarcodeInput+4;
                char assignSuccess = 0;
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
                    assignSuccess = 1;
                }
                pthread_mutex_unlock(&MutexLinklist);
                pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
                sleep(0);
                if(assignSuccess == 1)
                {
                    BarcodeInputDone = 1;
                }
            }else
            {
                printf("[%s|%d]scan UserNo. error!\n", __func__, __LINE__);
                DisableUpDown = 1;
                ScreenIndex = 7;
                UpdateScreenFunction(7, 0);
                UpdateScreenFunction(99,4); 
            }
        }else
        { 
            int arrayCount = 0;
            int stringLength;
            unsigned char flagFailPass = 0;
            stringLength = strlen(tempBarcodeInput);

            pthread_mutex_lock(&MutexInput);
            switch(BarcodeIndex)
            {
                case ISNO:
                    if(ZHNode != NULL && strlen(tempBarcodeInput) == 14)
                    {
                        memset(ZHNode->ISNo, 0, sizeof(char)*INPUTLENGTH);
                        memcpy(ZHNode->ISNo, tempBarcodeInput, sizeof(tempBarcodeInput));
                        BarcodeInputDone = 1;
                    }else
                    {
                        printf("scan ISNo. error\n");
                        DisableUpDown = 1;
                        ScreenIndex = 6;
                        UpdateScreenFunction(6, 0);
                        UpdateScreenFunction(99, 1); 
                        sleep(1);
                        DisableUpDown = 0;
                        ScreenIndex = 3;
                        UpdateScreenFunction(3, 0);
                    }
                break;
                case MANAGERCARD:
                    if(ZHNode != NULL && strlen(tempBarcodeInput) == 24)
                    {
                        memset(ZHNode->ManagerCard, 0, sizeof(char)*INPUTLENGTH);
                        memcpy(ZHNode->ManagerCard, tempBarcodeInput, sizeof(tempBarcodeInput));
                        BarcodeInputDone = 1;
                    }else
                    {
                        printf("Scan Part Number error\n");
                        DisableUpDown = 1;
                        ScreenIndex = 6;
                        UpdateScreenFunction(6, 0);
                        UpdateScreenFunction(99, 2); 
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
                    if(ZHNode != NULL && flagFailPass == 0 && stringLength > 0)
                    {
                        memset(ZHNode->CountNo, 0, sizeof(char)*INPUTLENGTH);
                        memcpy(ZHNode->CountNo, tempBarcodeInput, sizeof(char)*INPUTLENGTH);
                        long goodCount = (atol(ZHNode->CountNo));
                        if(goodCount > 0)
                        {
                            printf("need finish: %ld\n", goodCount);
                            BarcodeInputDone = 1;
                        }else
                        {
                            printf("scan Count error!\n");
                            DisableUpDown = 1;
                            ScreenIndex = 6;
                            UpdateScreenFunction(6, 0);
                            UpdateScreenFunction(99, 3); 
                        }
                    }else
                    {
                        printf("scan Count error!\n");
                        DisableUpDown = 1;
                        ScreenIndex = 6;
                        UpdateScreenFunction(6, 0);
                        UpdateScreenFunction(99, 3); 
                    }
                break;
                case USERNO:
                if(ZHNode != NULL && strncmp(tempBarcodeInput, "XXXP", 4) == 0)
                {
                    char *tempPtr = tempBarcodeInput + 4;
                    memset(ZHNode->UserNo, 0, sizeof(char)*INPUTLENGTH);
                    memcpy(ZHNode->UserNo, tempPtr, sizeof(char)*(INPUTLENGTH-4));
                    BarcodeInputDone = 1;
                }else
                {
                    printf("scan UserNo. error\n");
                    DisableUpDown = 1;
                    ScreenIndex = 6;
                    UpdateScreenFunction(6, 0);
                    UpdateScreenFunction(99, 4); 
                }
                break;
                default:
                    printf("nothing\n"); 
            }
            pthread_mutex_unlock(&MutexInput);
        }
    }
}
