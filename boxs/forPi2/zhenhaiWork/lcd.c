#include "lcd.h"

int UpdateScreenFunction(int screenIndex)
{
    unsigned char infoScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0a, 0x00, 0x0d};
    unsigned char countScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0a, 0x00, 0x0d};
    unsigned char powerOffScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0a, 0x00, 0x0d};
    unsigned char menuScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0a, 0x00, 0x0d};
    unsigned char configScreen[13] = {0x31, 0x04, 0x32, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0a, 0x00, 0x0d};
    unsigned char popUpScreen[18] = {0x31, 0x04, 0x31, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0a, 0x00, 0x0d};
    unsigned char endString[3] = {0x0a, 0x00, 0x0d};
    unsigned char startString[5] = {0x31, 0x04, 0x31, 0x10, 0x00};
    unsigned char startString2[5] = {0x31, 0x04, 0x31, 0x00, 0x00};
   
    if(screenIndex == 1)
    {   
        //count p2    
        unsigned char countNoPositionColorString[10] = {0x00, 0x69, 0x00, 0x5a, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char goodNoPositionColorString[10] = {0x00, 0x69, 0x00, 0x89, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};   
        unsigned char totalBadNoPositionColorString[10] = {0x00, 0x69, 0x00, 0xb9, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};   

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr, * countPtr;
        int arraySize = 0;
 
        SendCommandMessageFunction(countScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        //sleep(1);

        //count no.
        arraySize = strlen(CountNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, countNoPositionColorString, 10);
        commandPtr = commandPtr + 10;
        memcpy(commandPtr, CountNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);
        SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
 
        //good count
        if(arraySize > 0)
        {
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + 7));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + 7));
            countPtr = (unsigned char *)malloc(sizeof(unsigned char)*7);
            memset(countPtr, 0, sizeof(unsigned char)*7);

            sprintf(countPtr, "%ld", (GoodCount%1000000));

            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, goodNoPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, countPtr, 7);
            commandPtr = commandPtr + 7;
            memcpy(commandPtr, endString, 3);
            SendCommandMessageFunction(commandArrayPtr, 18 + 7);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
            if(countPtr != NULL)
            {
                free(countPtr);
                countPtr = NULL;
            }

            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + 7));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + 7));
            countPtr = (unsigned char *)malloc(sizeof(unsigned char)*7);
            memset(countPtr, 0, sizeof(unsigned char)*7);
         
            sprintf(countPtr, "%ld", (TotalBadCount%1000000));

            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, totalBadNoPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, countPtr, 7);
            commandPtr = commandPtr + 7;
            memcpy(commandPtr, endString, 3);
            SendCommandMessageFunction(commandArrayPtr, 18 + 7);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
            if(countPtr != NULL)
            {
                free(countPtr);
                countPtr = NULL;
            }

        }
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 99)
    {
        unsigned char BarcodeError1[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '1', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError2[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '2', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError3[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '3', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError4[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '4', 0x0a, 0x00, 0x0d};

        unsigned char BarcodeError5[24] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'E', 'r', 'r', 'o', 'r', '5', 0x0a, 0x00, 0x0d};

        if(BarcodeIndex == 0)
            SendCommandMessageFunction(BarcodeError1, 28);
        else if(BarcodeIndex == 1)
            SendCommandMessageFunction(BarcodeError2, 28);
        else if(BarcodeIndex == 2)
            SendCommandMessageFunction(BarcodeError4, 28);
        else if(BarcodeIndex == 3)
            SendCommandMessageFunction(BarcodeError3, 28);
        else
            SendCommandMessageFunction(BarcodeError5, 28);

    }else if(screenIndex == 2)
    {
        //power p3
        SendCommandMessageFunction(powerOffScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 3)
    {
        unsigned char bar1[34] = {0x31,0x04,0x31,0x03,0x00,0x00,0x48,0x00,0x37,0xFF,0xFF,0xFF,0x00,0x00,0x00,
                                           '>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','<', 0x0a, 0x00, 0x0d};
        
        //menu1 p4
        SendCommandMessageFunction(menuScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);

        SendCommandMessageFunction(bar1, 34);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if (screenIndex == 4)
    {
        unsigned char bar2[34] = {0x31,0x04,0x31,0x03,0x00,0x00,0x48,0x00,0x69,0xFF,0xFF,0xFF,0x00,0x00,0x00,
                                           '>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','<', 0x0a, 0x00, 0x0d};
        //menu 2 p4
        SendCommandMessageFunction(menuScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);


        SendCommandMessageFunction(bar2, 34);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 5)
    {
        unsigned char bar3[34] = {0x31,0x04,0x31,0x03,0x00,0x00,0x48,0x00,0x9D,0xFF,0xFF,0xFF,0x00,0x00,0x00,
                                           '>',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','<', 0x0a, 0x00, 0x0d};
        //menu 3 p4
        SendCommandMessageFunction(menuScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);

        SendCommandMessageFunction(bar3, 34);
        SendCommandMessageFunction(popUpScreen, 18);
    }else if(screenIndex == 6)
    {
        unsigned char machineNoPositionColorString[10] = {0x00, 0x8c, 0x00, 0x8c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char ipAddressPositionColorString[10] = {0x00, 0x64, 0x00, 0xc8, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};   

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr, * countPtr;
        int arraySize = 0;
 
        SendCommandMessageFunction(configScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        //sleep(1);

        int fd2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct ifreq ethreq;
        memset(&ethreq, 0, sizeof(ethreq));
        strncpy(ethreq.ifr_name, ZHNETWORKTYPE, IFNAMSIZ);
        ioctl(fd2, SIOCGIFFLAGS, &ethreq);

        if(ethreq.ifr_flags & IFF_RUNNING)
        {
            char ipAddrArray[17] ;
            int fd = socket(AF_INET, SOCK_DGRAM, 0);
            struct ifreq ifr;
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, ZHNETWORKTYPE, IFNAMSIZ-1);
            ioctl(fd, SIOCGIFADDR, &ifr);
            close(fd);

            memset(ipAddrArray, 0, sizeof(char)*17);
            strncpy(ipAddrArray, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), sizeof(char)*17);
            arraySize = 17;
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, ipAddressPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, ipAddrArray, arraySize);
            commandPtr = commandPtr + arraySize;
            memcpy(commandPtr, endString, 3);
            SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }else
        {
            ;//
        }
        close(fd2);

        //machine no.
        arraySize = strlen(MachineNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, machineNoPositionColorString, 10);
        commandPtr = commandPtr + 10;
        memcpy(commandPtr, MachineNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);
        SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
 
        SendCommandMessageFunction(popUpScreen, 18);
   
    }else if(screenIndex == 7)
    {
        unsigned char repairModePosition[28] = {0x31,0x04,0x31,0x03,0x00,0x00,0x0A,0x00,0x32,0xFF,0xFF,0xFF,0x00,0x00,0x00
                                                    ,'R','e','p','a','i','r','i','n','g',0x0a, 0x00, 0x0d};
        unsigned char repairerPositionColorString[14] = {0x00, 0x0A, 0x00, 0x86, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,'I','D',':', ' '};   

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr, * countPtr;
        int arraySize = 0;
 
        SendCommandMessageFunction(powerOffScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);

        SendCommandMessageFunction(repairModePosition, 27);

        //machine no.
        arraySize = strlen(RepairNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(22 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(22 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString2, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, repairerPositionColorString, 14);
        commandPtr = commandPtr + 14;
        memcpy(commandPtr, RepairNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);
        SendCommandMessageFunction(commandArrayPtr, 22 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
 
        SendCommandMessageFunction(popUpScreen, 18);
    }else
    {
        //info p1
        /*unsigned char lotPostionColorString[10] = {0x00, 0x78, 0x00, 0x3c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString1[10] = {0x00, 0x78, 0x00, 0x5f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString2[10] = {0x00, 0x64, 0x00, 0x82, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char countNoPositionColorString[10] = {0x00, 0x96, 0x00, 0xa5, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char userNoPositionColorString[10] = {0x00, 0x96, 0x00, 0xcf, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        */
        unsigned char lotPostionColorString[10] = {0x00, 0x5f, 0x00, 0x3c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString1[10] = {0x00, 0x69, 0x00, 0x5f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString2[10] = {0x00, 0x69, 0x00, 0x82, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char countNoPositionColorString[10] = {0x00, 0x82, 0x00, 0xa5, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char userNoPositionColorString[10] = {0x00, 0x82, 0x00, 0xcf, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};

        unsigned char * commandArrayPtr;
        unsigned char * commandPtr,  * managerPtr;
        int arraySize = 0;

        SendCommandMessageFunction(infoScreen, 13);
        nanosleep((const struct timespec[]){{0, 450000000L}}, NULL);
        //sleep(1);
        
        //lot no
        if(strlen(ISNo) > 0)
        { 
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*32); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*32);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, lotPostionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, ISNo, 14);
            commandPtr = commandPtr +14;
            memcpy(commandPtr, endString, 3);

            //31 4 31 3 10 0 78 0 3c ff ff ff 0 0 0 31 32 33 34 35 36 37 38 39 30 31 32 33 34 a 0 d
            SendCommandMessageFunction(commandArrayPtr, 32);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }
        if(strlen(ManagerCard) > 0)
        {
            //part no. 1
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*28);
            memset(commandArrayPtr, 0, sizeof(unsigned char)*28);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, partPositionColorString1, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, ManagerCard, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, endString, 3);

            SendCommandMessageFunction(commandArrayPtr, 28);
            if(commandArrayPtr != NULL)
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        
            //part no. 2
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char )*32);
            memset(commandArrayPtr, 0, sizeof(unsigned char)*32);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, partPositionColorString2, 10);
            commandPtr = commandPtr + 10;
            managerPtr = ManagerCard;
            managerPtr = managerPtr + 10;
            memcpy(commandPtr, managerPtr, 14);
            commandPtr = commandPtr + 14;
            memcpy(commandPtr, endString, 3);

            SendCommandMessageFunction(commandArrayPtr, 32);
            if(commandArrayPtr != NULL)
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
            if(managerPtr != NULL)
            {
                managerPtr = NULL;
            }
        }

        //count no.
        arraySize = strlen(CountNo);
#ifdef DEBUG
        printf("array size:%d\n", arraySize);
#endif
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, 5);
        commandPtr = commandPtr + 5;
        memcpy(commandPtr, countNoPositionColorString, 10);
        commandPtr = commandPtr + 10;
        memcpy(commandPtr, CountNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, 3);

        //31 4 31 3 10 0 78 0 3c ff ff ff 0 0 0 31 32 33 34 35 36 37 38 39 30 31 32 33 34 a 0 d
        SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
                
        //user no
        if(isInPairMode == 1)
        {
            if(CanChangeRepairModeFlag == 3)
            {
                arraySize = strlen("Repair Done");
                
                commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
                memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
                commandPtr = commandArrayPtr;
                memcpy(commandPtr, startString, 5);
                commandPtr = commandPtr + 5;
                memcpy(commandPtr, userNoPositionColorString, 10);
                commandPtr = commandPtr + 10;
                memcpy(commandPtr, "Repair Done", arraySize);
                commandPtr = commandPtr + arraySize;
                memcpy(commandPtr, endString, 3);

                SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
                if(commandArrayPtr != NULL) 
                {
                    free(commandArrayPtr);
                    commandPtr = NULL;
                }
            }else
            {
                arraySize = strlen("Repairing");
                
                commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
                memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
                commandPtr = commandArrayPtr;
                memcpy(commandPtr, startString, 5);
                commandPtr = commandPtr + 5;
                memcpy(commandPtr, userNoPositionColorString, 10);
                commandPtr = commandPtr + 10;
                memcpy(commandPtr, "Repairing", arraySize);
                commandPtr = commandPtr + arraySize;
                memcpy(commandPtr, endString, 3);

                SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
                if(commandArrayPtr != NULL) 
                {
                    free(commandArrayPtr);
                    commandPtr = NULL;
                }
            }
        }else
        {
            arraySize = strlen(UserNo);
            char *userNoPtr = UserNo;
            if(arraySize > 25)
            {
                arraySize = arraySize - 24;
                userNoPtr = UserNo + 24;
            } 
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(18 + arraySize)); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(18 + arraySize));
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, 5);
            commandPtr = commandPtr + 5;
            memcpy(commandPtr, userNoPositionColorString, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, userNoPtr, arraySize);
            commandPtr = commandPtr + arraySize;
            memcpy(commandPtr, endString, 3);

            SendCommandMessageFunction(commandArrayPtr, 18 + arraySize);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }
        SendCommandMessageFunction(popUpScreen, 18);
    }
    return 0;
}


void * ChangeScreenEventListenFunction(void *argument)
{
    int screenIndex = 3;
    char flagForZHPIN32, flagForZHPIN22, flagForZHPIN36, flagForZHPIN38;

    flagForZHPIN32 = flagForZHPIN22 = flagForZHPIN36 = flagForZHPIN38 = 0;
    //ScreenIndex = screenIndex;

    while(1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if(DisableUpDown == 0 && flagForZHPIN32 == 0 && digitalRead(ZHPIN32) == 0)
        {
            printf("get ZHPIN_32 event\n");
            //screenIndex = (screenIndex + 1) % 2;
            screenIndex = (screenIndex + 1) % 3 + 3;
            ScreenIndex = screenIndex;
            flagForZHPIN32 = 1; 
            UpdateScreenFunction(screenIndex);
        }else if(flagForZHPIN32 == 1 && digitalRead(ZHPIN32) == 1)
        {
            flagForZHPIN32 = 0;
        }else if(DisableUpDown == 0 && flagForZHPIN22 == 0 && digitalRead(ZHPIN22) == 0)
        {
            printf("get ZHPIN_22 event\n");
            //screenIndex = (screenIndex + 1) % 2;
            screenIndex = (screenIndex - 1) % 3 + 3;
            ScreenIndex = screenIndex;
            flagForZHPIN36 = 1; 
            UpdateScreenFunction(screenIndex);
        }else if(flagForZHPIN22 == 1 && digitalRead(ZHPIN22) == 1)
        {
            flagForZHPIN22 = 0;
        }else if(flagForZHPIN36 == 0 && digitalRead(ZHPIN36) == 0)
        {
            printf("get ZHPIN_36 event\n");
            ScreenIndex = screenIndex;
            flagForZHPIN36 = 1;
            UpdateScreenFunction(screenIndex);
        }else if(flagForZHPIN36 == 1 && digitalRead(ZHPIN36) == 1)
        {
            flagForZHPIN36 = 0;
            DisableUpDown = 0;
        }else if(DisableUpDown == 0 && flagForZHPIN38 == 0 && digitalRead(ZHPIN38) == 0)
        {
            printf("get ZHPIN_38 event\n");
            if(screenIndex == 3)
            {
                ScreenIndex = 0;
                UpdateScreenFunction(0);
            }
            else if(screenIndex == 4)
            {
                ScreenIndex = 1;
                UpdateScreenFunction(1);
            }
            else
            {
                //if(isInPairMode)
                //{
                //    ScreenIndex =7; 
                //    UpdateScreenFunction(7);
                //}else
                //{
                    ScreenIndex =6; 
                    UpdateScreenFunction(6);
                //}
            }
            flagForZHPIN38 = 1;
            DisableUpDown = 1;
        }else if(flagForZHPIN38 == 1 && digitalRead(ZHPIN38) == 1)
        {
            flagForZHPIN38 = 0;
        }
        else;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
        sleep(0);
    }
}

int SendCommandMessageFunction (unsigned char *message, int arrayLength)
{
    unsigned char *copyMessageArray;
    int forCount = 0;

#ifdef ZHCHECKSCREENBUSY
    unsigned char x[2] = {0x00, 0x00};
    int maxRetry = 100;

    while(x[1] != 0x01)
    {
        x[0] = 0x01;
        x[1] = 0x00;
        wiringPiSPIDataRW(CSCHANNEL, x, 2);
        printf("we wait %x\n", x[1]);
        if(x[1] == 0x01) break;
        nanosleep((const struct timespec[]){{0, 10000000L}}, NULL);
        maxRetry --;
        if(maxRetry <= 0)
        {
            digitalWrite (ZHPIN33, LOW);
            //nanosleep((const struct timespec[]){{0, 200000000L}}, NULL);
            sleep(2);    

            digitalWrite (ZHPIN33, HIGH);
            //nanosleep((const struct timespec[]){{0, 1500000000L}}, NULL);
            sleep(2);    
            maxRetry = 100;
            printf("refresh\n");
        }
    }
    printf("lcd idle\n");
#endif

    copyMessageArray = (unsigned char *)malloc(sizeof(unsigned char)*arrayLength);

    memset(copyMessageArray, 0, sizeof(unsigned char)*arrayLength);
    memcpy(copyMessageArray, message, arrayLength);

#ifdef DEBUG 
    printf("[%s|%d] ", __func__, __LINE__); 
    for(forCount = 0; forCount < arrayLength; forCount++)
    {
        printf("%x ", copyMessageArray[forCount]);
    }
    printf("\n");
#endif

    wiringPiSPIDataRW(CSCHANNEL, copyMessageArray, arrayLength);
    ///nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    //sleep(1);
    if(copyMessageArray != NULL)
    {
        //printf("free alloc memory\n");
        free(copyMessageArray);
    }
    return 0;
}

