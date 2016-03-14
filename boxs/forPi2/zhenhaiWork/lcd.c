#include "lcd.h"

#define LCDCOMMANDPOSITION 10
#define LCDCOMMANDPREFIX 5
#define LCDCOMMANDPOSTFIX 3

#define ZHBUTTONEVENTLOG "/home/pi/zhlog/zhEventLog"

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
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
        commandPtr = commandPtr + LCDCOMMANDPREFIX;
        memcpy(commandPtr, countNoPositionColorString, LCDCOMMANDPOSITION);
        commandPtr = commandPtr + LCDCOMMANDPOSITION;
        memcpy(commandPtr, CountNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);
        SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
        //good count
        if(arraySize > 0)
        {
            //now we only show last 7 number
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 7 + LCDCOMMANDPOSTFIX));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 7 + LCDCOMMANDPOSTFIX));
            countPtr = (unsigned char *)malloc(sizeof(unsigned char)*7);
            memset(countPtr, 0, sizeof(unsigned char)*7);

            sprintf(countPtr, "%ld", (GoodCount%10000000));

            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, goodNoPositionColorString, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            memcpy(commandPtr, countPtr, 7);
            commandPtr = commandPtr + 7;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);
            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 7 + LCDCOMMANDPOSTFIX);
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

            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 7 + LCDCOMMANDPOSTFIX));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 7 + LCDCOMMANDPOSTFIX));
            countPtr = (unsigned char *)malloc(sizeof(unsigned char)*7);
            memset(countPtr, 0, sizeof(unsigned char)*7);
         
            sprintf(countPtr, "%ld", (TotalBadCount%10000000));

            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, totalBadNoPositionColorString, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            memcpy(commandPtr, countPtr, 7);
            commandPtr = commandPtr + 7;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);
            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 7 + LCDCOMMANDPOSTFIX);
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
    }/*else if(screenIndex == 99)
    {
        unsigned char loadingProcess[28] = {0x31, 0x04, 0x31, 0x13, 0x10, 
                   0x00, 0x5a, 0x00, 0x78, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 'L', 'o', 'a', 'd', 'i', 'n', 'g', '.', '.', '.', 0x0a, 0x00, 0x0d};

        
        SendCommandMessageFunction(loadingProcess, 28);

    }*/else if(screenIndex == 2)
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
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, ipAddressPositionColorString, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            memcpy(commandPtr, ipAddrArray, arraySize);
            commandPtr = commandPtr + arraySize;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);
            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX);
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
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
        commandPtr = commandPtr + LCDCOMMANDPREFIX;
        memcpy(commandPtr, machineNoPositionColorString, LCDCOMMANDPOSITION);
        commandPtr = commandPtr + LCDCOMMANDPOSITION;
        memcpy(commandPtr, MachineNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);
        SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
 
        SendCommandMessageFunction(popUpScreen, 18);
   
    }/*else if(screenIndex == 7)
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
    }*/else
    {
        //info p1
        /*unsigned char lotPostionColorString[10] = {0x00, 0x78, 0x00, 0x3c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString1[10] = {0x00, 0x78, 0x00, 0x5f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString2[10] = {0x00, 0x64, 0x00, 0x82, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char countNoPositionColorString[10] = {0x00, 0x96, 0x00, 0xa5, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char userNoPositionColorString[10] = {0x00, 0x96, 0x00, 0xcf, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        */
        unsigned char lotPostionColorString[10] = {0x00, 0x5f, 0x00, 0x3c, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString1[10] = {0x00, 0x5f, 0x00, 0x5f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
        unsigned char partPositionColorString2[10] = {0x00, 0x5f, 0x00, 0x82, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00};
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
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 14 + LCDCOMMANDPOSTFIX)); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 14 + LCDCOMMANDPOSTFIX);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, lotPostionColorString, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            memcpy(commandPtr, ISNo, 14);
            commandPtr = commandPtr +14;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

            //31 4 31 3 10 0 78 0 3c ff ff ff 0 0 0 31 32 33 34 35 36 37 38 39 30 31 32 33 34 a 0 d
            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 14 + LCDCOMMANDPOSTFIX);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }
        if(strlen(ManagerCard) > 0)
        {
            //part no. 1
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 10 + LCDCOMMANDPOSTFIX));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*28);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, partPositionColorString1, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            memcpy(commandPtr, ManagerCard, 10);
            commandPtr = commandPtr + 10;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 10 + LCDCOMMANDPOSTFIX);
            if(commandArrayPtr != NULL)
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        
            //part no. 2
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char )*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 14 + LCDCOMMANDPOSTFIX));
            memset(commandArrayPtr, 0, sizeof(unsigned char)*32);
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, partPositionColorString2, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            managerPtr = ManagerCard;
            managerPtr = managerPtr + 10;
            memcpy(commandPtr, managerPtr, 14);
            commandPtr = commandPtr + 14;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + 14 + LCDCOMMANDPOSTFIX);
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
        commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX)); 
        memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION  + arraySize + LCDCOMMANDPOSTFIX));
        commandPtr = commandArrayPtr;
        memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
        commandPtr = commandPtr + LCDCOMMANDPREFIX;
        memcpy(commandPtr, countNoPositionColorString, LCDCOMMANDPOSITION);
        commandPtr = commandPtr + LCDCOMMANDPOSITION;
        memcpy(commandPtr, CountNo, arraySize);
        commandPtr = commandPtr + arraySize;
        memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

        //31 4 31 3 10 0 78 0 3c ff ff ff 0 0 0 31 32 33 34 35 36 37 38 39 30 31 32 33 34 a 0 d
        SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION  + arraySize + LCDCOMMANDPOSTFIX);
        if(commandArrayPtr != NULL) 
        {
            free(commandArrayPtr);
            commandPtr = NULL;
        }
                
        //user no
        if(IsInRepairMode == 1)
        {
            if(CanChangeRepairModeFlag == 3)
            {
                arraySize = strlen("Repair Done");
                
                commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
                memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION  + arraySize + LCDCOMMANDPOSTFIX));
                commandPtr = commandArrayPtr;
                memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
                commandPtr = commandPtr + LCDCOMMANDPREFIX;
                memcpy(commandPtr, userNoPositionColorString, LCDCOMMANDPOSITION);
                commandPtr = commandPtr + LCDCOMMANDPOSITION;
                memcpy(commandPtr, "Repair Done", arraySize);
                commandPtr = commandPtr + arraySize;
                memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

                SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX);
                if(commandArrayPtr != NULL) 
                {
                    free(commandArrayPtr);
                    commandPtr = NULL;
                }
            }else
            {
                arraySize = strlen("Repairing");
                
                commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
                memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
                commandPtr = commandArrayPtr;
                memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
                commandPtr = commandPtr + LCDCOMMANDPREFIX;
                memcpy(commandPtr, userNoPositionColorString, LCDCOMMANDPOSITION);
                commandPtr = commandPtr + LCDCOMMANDPOSITION;
                memcpy(commandPtr, "Repairing", arraySize);
                commandPtr = commandPtr + arraySize;
                memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

                SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX);
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
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX)); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, userNoPositionColorString, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            memcpy(commandPtr, userNoPtr, arraySize);
            commandPtr = commandPtr + arraySize;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX);
            if(commandArrayPtr != NULL) 
            {
                free(commandArrayPtr);
                commandPtr = NULL;
            }
        }
        if(BarcodeIndex == NEEDPRIVILEGE)
        {
            unsigned char startString3[5] = {0x31, 0x04, 0x31, 0x10, 0x10};
            unsigned char hintPositionColorString[10] = {0x00, 0x32, 0x00, 0x64, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff};
            arraySize = strlen("A Repeat ISNo");    
                
            commandArrayPtr = (unsigned char *)malloc(sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX)); 
            memset(commandArrayPtr, 0, sizeof(unsigned char)*(LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX));
            commandPtr = commandArrayPtr;
            memcpy(commandPtr, startString3, LCDCOMMANDPREFIX);
            commandPtr = commandPtr + LCDCOMMANDPREFIX;
            memcpy(commandPtr, hintPositionColorString, LCDCOMMANDPOSITION);
            commandPtr = commandPtr + LCDCOMMANDPOSITION;
            memcpy(commandPtr, "A Repeat ISNo", arraySize);
            commandPtr = commandPtr + arraySize;
            memcpy(commandPtr, endString, LCDCOMMANDPOSTFIX);

            SendCommandMessageFunction(commandArrayPtr, LCDCOMMANDPREFIX + LCDCOMMANDPOSITION + arraySize + LCDCOMMANDPOSTFIX);
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
    FILE *logPtr;

    flagForZHPIN32 = flagForZHPIN22 = flagForZHPIN36 = flagForZHPIN38 = 0;
    //ScreenIndex = screenIndex;

    while(1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if(DisableUpDown == 0 && flagForZHPIN32 == 0 && digitalRead(ZHPIN32) == 0)
        {
#ifdef ZHBUTTONEVENTLOG
            logPtr = fopen(ZHBUTTONEVENTLOG, "w");
            if(logPtr != NULL)
            {
                fprintf(logPtr, "get ZHPIN_32 event\n");
                fclose(logPtr);
            }
#endif
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
#ifdef ZHBUTTONEVENTLOG
            logPtr = fopen(ZHBUTTONEVENTLOG, "w");
            if(logPtr != NULL)
            {
                fprintf(logPtr, "get ZHPIN_22 event\n");
                fclose(logPtr);
            }
#endif
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
#ifdef ZHBUTTONEVENTLOG
            logPtr = fopen(ZHBUTTONEVENTLOG, "w");
            if(logPtr != NULL)
            {
                fprintf(logPtr, "get ZHPIN_36 event\n");
                fclose(logPtr);
            }
#endif
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
#ifdef ZHBUTTONEVENTLOG
            logPtr = fopen(ZHBUTTONEVENTLOG, "w");
            if(logPtr != NULL)
            {
                fprintf(logPtr, "get ZHPIN_36 event\n");
                fclose(logPtr);
            }
#endif
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
    }
}

int SendCommandMessageFunction (unsigned char *message, int arrayLength)
{
    unsigned char *copyMessageArray;
#ifdef DEBUG
    int forCount = 0;
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
    nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    //sleep(1);
    if(copyMessageArray != NULL)
    {
        //printf("free alloc memory\n");
        free(copyMessageArray);
    }
    return 0;
}

