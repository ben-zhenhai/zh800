//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

//#define LOCALTEST

#include "zhenhai.h"
#include "standerInput.h"
#include "ftp.h"
#include "eeprom.h"
//#include "lcd.h"

//#include "ats_1/ats_1.h"
//#define ATS_1

#include "gt_1318p/gt_1318p.h"
#define GT1318P

void sig_fork(int signo)
{
    pid_t pid;
    int stat;
    pid=waitpid(0,&stat,WNOHANG);
    printf("%s: child process finish upload\n", __func__);
    
    return;
}

void * EventListenFunction(void *argument)
{
    while(1)
    {
        while(!ButtonEnableFlag)
        {
            usleep(USLEEPTIMEUNITLONG);
            //printf("we wait\n");
        }

        ZHResetFlag = 0;
        LoopLeaveEventIndex = ZHTimeoutExitEvent;
        pthread_mutex_lock(&MutexEEPROM);
        sleep(1);
        //because write eeprom need more time, a check for go through too fast
        pthread_mutex_unlock(&MutexEEPROM);
 
        while(!ZHResetFlag)
        {
            usleep(USLEEPTIMEUNITSHORT);
            GoodCount = ExCount[GOODCOUNT];
            if(ExCount[GOODCOUNT] >= (long)atol(ZHList->CountNo))
            //if(ExCount[GOODCOUNT] >= 0)
            {
                //finish job                 
                printf("Good Count Number arrival!\n");
                ZHResetFlag = 1; 
                LoopLeaveEventIndex = ZHNormalExitEvent;
                usleep(USLEEPTIMEUNITSHORT);
            }else if(digitalRead(ZHPIN40) == 0)
            {
                printf("event trigger (PIN_40)\n");
                ZHResetFlag = 1;
                LoopLeaveEventIndex = ZHChangeUserExitEvent;
                usleep(USLEEPTIMEUNITSHORT);
                //printf("event trigger (PIN_29)\n");
                //ZHResetFlag = 1;
                //LoopLeaveEventIndex = ZHPowerOffEvent;
                //usleep(USLEEPTIMEUNITSHORT);

            }else if(digitalRead(ZHPIN7) == 1)
            {
                //finish job
                printf("event trigger (PIN_7)!\n");
                ZHResetFlag = 1;
                LoopLeaveEventIndex = ZHForceExitEvent;
                usleep(USLEEPTIMEUNITSHORT);
            }else if(digitalRead(ZHPIN29) == 1)
            {
#ifndef LOCALTEST
                printf("event trigger (PIN_29)\n");
                ZHResetFlag = 1;
                LoopLeaveEventIndex = ZHPowerOffEvent;
                usleep(USLEEPTIMEUNITSHORT);
#endif
            }
            else;
        }       
        pthread_mutex_lock(&MutexMain);
        ButtonEnableFlag = 0;
        pthread_cond_signal(&CondMain);
        pthread_mutex_unlock(&MutexMain);
            
        if(LoopLeaveEventIndex == ZHNormalExitEvent || LoopLeaveEventIndex == ZHForceExitEvent)
        {
            //normal leave
            printf("normal leave\n");
        }else
        {
            //timeout leave
            printf("isnormal leave\n");
            while(1)
            {
                StopUpdateNetworkStatusFlag = 1;
                InputDone = 0;
                while(InputDone == 0)
                {
                    usleep(USLEEPTIMEUNITMID);
                }
                StopUpdateNetworkStatusFlag = 0;
                if(InputDone == 1)
                {
                    break;
                }
            }
        }
    }
    printf("ready to exit %s\n", __func__);
}

int main()
{
    pthread_t inputThread, eventListenThread, watchdogThread , changeScreenThread;

#if defined ATS_1
    pthread_t interruptThread1; 
#endif

#if defined GT1318P
    pthread_t serialThread;
#endif

    int rc = 0;
    int filesize = 0;
    struct timeval now;
    struct timespec outtime;
    FILE *filePtr;
    char *buffer;
    int isFirstinLoop = 1; //確保能夠跑進START 只有一次
    struct stat fileInfo; 
    char uploadFilePath[INPUTLENGTH];
    InputMode = 0; 
    //InputNode *node;

    pthread_mutex_init(&MutexMain, NULL);
    pthread_mutex_init(&MutexEEPROM, NULL);
    pthread_mutex_init(&MutexInput, NULL);
    pthread_mutex_init(&MutexFile, NULL);
    pthread_mutex_init(&MutexWatchdog, NULL);
    pthread_mutex_init(&MutexScreen, NULL);

    pthread_cond_init(&CondMain, NULL);
    pthread_cond_init(&CondWatchdog, NULL);
       
    memset(Count, 0, sizeof(unsigned long)*EVENTSIZE);
    memset(ExCount, 0, sizeof(unsigned long)*EVENTSIZE);
    GoodCount = 0;
    TotalBadCount = 0;

#if defined ATS_1
    memset(I2CEXValue, 0, sizeof(int)*6);
    memset(CutRoll, 0, sizeof(short)*2);
#endif

    wiringPiSetup();
    wiringPiSPISetup(CSCHANNEL, SPICLOCK);
    if(SetI2cConfig())
    {
        printf("set i2c config fail\n");
        return 1;
    }
    signal (SIGCHLD, sig_fork);
    
    pinMode(ZHPIN7, INPUT);
    pinMode(ZHPIN22, INPUT);
    pinMode(ZHPIN29, INPUT);
    pinMode(ZHPIN32, INPUT);
    pinMode(ZHPIN36, INPUT);
    pinMode(ZHPIN38, INPUT);
    pinMode(ZHPIN40, INPUT);

    pullUpDnControl(ZHPIN22, PUD_UP);
    pullUpDnControl(ZHPIN29, PUD_UP);
    pullUpDnControl(ZHPIN32, PUD_UP);
    pullUpDnControl(ZHPIN36, PUD_UP);
    pullUpDnControl(ZHPIN38, PUD_UP);
    pullUpDnControl(ZHPIN40, PUD_UP);
    pullUpDnControl(ZHPIN7, PUD_DOWN);

    pinMode(ZHPIN15, OUTPUT);
    pinMode(ZHPIN16, OUTPUT);
    pinMode(ZHPIN18, OUTPUT);
    pinMode(ZHPIN31, OUTPUT);

    filePtr = fopen(BarcodeFilePath , "r");
    if(filePtr != NULL)
    {
        int forCount = 0;

        fseek(filePtr, 0, SEEK_END);
        filesize = ftell(filePtr);
        rewind(filePtr);
        buffer = (char *)malloc(sizeof(char)*filesize);
        fread(buffer, 1, filesize, filePtr);
        fclose(filePtr);    

        memset(MachineNo, 0, sizeof(char)*INPUTLENGTH);

        for(forCount = 0; forCount < filesize -1; ++forCount)
        {
            MachineNo[forCount] = buffer[forCount];
        }
    
        if(buffer != NULL)
        {
            free(buffer);
        }
    }else
    {
        printf("read machine's number error");
        return 0;
    }

    //make sure we can create a node for get lastest information
    while(1)
    {
        ZHNode = (InputNode *)malloc(sizeof(InputNode));
        if(ZHNode == NULL)
        {
            printf("alloc fail\n");
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
        break;  
    }

    memset(ZHNode->UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
    gettimeofday(&now, NULL);
    sprintf(ZHNode->UploadFilePath,"%ld%s.txt",(long)now.tv_sec, MachineNo);
    printf("[%s|%d]new file name:%s\n", __func__, __LINE__, ZHNode->UploadFilePath);
    ZHList = ZHNode;

    WriteFile(MachSTANDBY);
    if(stat(ZHList->UploadFilePath, &fileInfo) == 0)
    {
        int fileSize = fileInfo.st_size;
        if(fileSize > 0)
        {
            memset(uploadFilePath, 0, sizeof(char)*INPUTLENGTH);
            pthread_mutex_lock(&MutexFile);
            strcpy(uploadFilePath, ZHList->UploadFilePath);
            sleep(1);
            gettimeofday(&now, NULL);
            sprintf(ZHList->UploadFilePath, "%ld%s.txt", (long)now.tv_sec,MachineNo);
            pthread_mutex_unlock(&MutexFile);
 
            pid_t proc = fork();
            if(proc < 0)
            {
               printf("fork child fail\n");
               return 0;
            }else if(proc == 0)
            {
               //execl("../../.nvm/v0.10.25/bin/node", "node", "../../mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                execl("/home/pi/.nvm/v0.10.25/bin/node", "node", "/home/pi/mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                exit(0);
            }else
            {
                //int result = -1;
                // wait(&result);
                //printf("upload success\n");
                waitpid(-1, NULL, WNOHANG);
            }   
        }else
        {
            printf("unlink file\n");
            unlink(uploadFilePath);
        }
    }
#ifdef LOCALTEST
    if(0)
#else
    if(!GetRemoteDataFunction())
#endif
    {
        Count[GOODCOUNT] = ExCount[GOODCOUNT] = atol(ZHList->GoodNo);
        WriteFile(MachRESUMEFROMPOWEROFF);
        pthread_mutex_lock(&MutexScreen);
        DisableUpDown = 1;
        ScreenIndex = 0;
        UpdateScreenFunction(0);
        pthread_mutex_unlock(&MutexScreen);
        InputMode = 1;
        printf("get data form remote\n");
    }
#ifdef LOCALTEST
    else if(0)
#else
    else if(!ReadEEPROMData())
#endif
    {
        //read data from eeprom
        Count[GOODCOUNT] = ExCount[GOODCOUNT] = atol(ZHList->GoodNo);
        pthread_mutex_lock(&MutexScreen);
        DisableUpDown = 1;
        ScreenIndex = 0;
        UpdateScreenFunction(0);
        pthread_mutex_unlock(&MutexScreen);
        InputMode = 1;
        printf("get data from local\n");
    }else
    {
        printf("load data fail\n");
        EarseEEPROMData();
        if(ZHList != NULL)
        {
            ZHList = NULL;
            free(ZHList);
        }
        //sleep(1);
        pthread_mutex_lock(&MutexScreen);
        ScreenIndex = 3;
        UpdateScreenFunction(3);
        pthread_mutex_unlock(&MutexScreen);
    }

    rc = pthread_create(&eventListenThread, NULL, EventListenFunction, NULL);
    assert(rc == 0);

    InputThreadFlag = 1;
    rc = pthread_create(&inputThread, NULL, InputFunction, NULL);
    assert(rc == 0);

    rc = pthread_create(&changeScreenThread, NULL, ChangeScreenEventListenFunction, NULL);
    assert(rc == 0);

    InputDone = 0;
    while(1)
    {
        unsigned char cancelThreadDone = 0;
        while(ZHList == NULL || InputDone == 0)
        {
            //printf("ZHList is NULL\n");
            usleep(USLEEPTIMEUNITLONG);
        }

        if(isFirstinLoop)
        {
            //first time, we need to write MachSTAR to FILE
            WriteFile(MachSTART);
            isFirstinLoop = 0;
            digitalWrite (ZHPIN31, HIGH);
        }
        printf("%s %s %s %s\n", ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, MachineNo); 
 
#if defined ATS_1
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
#endif

#if defined GT1318P
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#endif

        WatchdogThreadFlag = 1;
        rc = pthread_create(&watchdogThread, NULL, WatchdogFunction, NULL);
        assert(rc == 0);

        pthread_mutex_lock(&MutexMain);
        ButtonEnableFlag = 1;
        pthread_mutex_unlock(&MutexMain);
        while(1)
        {
            if(!ButtonEnableFlag);
            else
            {
                pthread_mutex_lock(&MutexMain);
                gettimeofday(&now, NULL);
                printf("[%s|%d] %ld ready to sleep\n", __func__, __LINE__, (long)now.tv_sec);
                outtime.tv_sec = now.tv_sec + 300;
                outtime.tv_nsec = now.tv_usec * 1000;
                pthread_cond_timedwait(&CondMain, &MutexMain, &outtime);
                pthread_mutex_unlock(&MutexMain);
                gettimeofday(&now, NULL);
                printf("[%s|%d] %ld wake up\n",__func__, __LINE__ ,(long)now.tv_sec);
            }
            if(!ButtonEnableFlag)
            {
                if(LoopLeaveEventIndex == ZHNormalExitEvent)
                {
#if defined ATS_1
                    usleep(USLEEPTIMEUNITMID);
                    pthread_cancel(interruptThread1);
                    pthread_join(interruptThread1, NULL); 
#endif
                    
#if defined GT1318P
                    usleep(USLEEPTIMEUNITMID);
                    SerialFunctionFlag = 0;
                    pthread_join(serialThread, NULL); 
#endif
                    usleep(USLEEPTIMEUNITMID);
                    pthread_mutex_lock(&MutexWatchdog);
                    WatchdogThreadFlag = 0;
                    pthread_cond_signal(&CondWatchdog);
                    pthread_mutex_unlock(&MutexWatchdog);
 
                    pthread_join(watchdogThread, NULL);

                    printf("cancel done (normal exit)\n");
                    //normal leave 
                    pthread_mutex_lock(&MutexFile);
                    WriteFile(MachJobDone);
                    WriteFile(MachSTANDBY);    
                    pthread_mutex_unlock(&MutexFile);                    

                    cancelThreadDone = 1;
                }
                else if(LoopLeaveEventIndex == ZHForceExitEvent)
                {
#if defined ATS_1
                    usleep(USLEEPTIMEUNITMID);
                    pthread_cancel(interruptThread1);
                    pthread_join(interruptThread1, NULL);
#endif

#if defined GT1318P
                    usleep(USLEEPTIMEUNITMID);
                    SerialFunctionFlag = 0;
                    pthread_join(serialThread, NULL); 
#endif
                    usleep(USLEEPTIMEUNITMID);
                    pthread_mutex_lock(&MutexWatchdog);
                    WatchdogThreadFlag = 0;
                    pthread_cond_signal(&CondWatchdog);
                    pthread_mutex_unlock(&MutexWatchdog);
 
                    printf("cancel done (force exit)\n");
                    //force leave
                    pthread_mutex_lock(&MutexFile);
                    if(ExCount[GOODCOUNT] >= (long)(atol(ZHList->CountNo)/1.04))
                    {
                        WriteFile(MachSTOPForce1);
                        WriteFile(MachSTANDBY);    
                    }
                    else
                    {
                        WriteFile(MachSTOPForce2);                    
                        WriteFile(MachSTANDBY);    
                    }
                    pthread_mutex_unlock(&MutexFile);
                    cancelThreadDone = 1;
                }else if(LoopLeaveEventIndex == ZHPowerOffEvent)
                {
#if defined ATS_1
                    usleep(USLEEPTIMEUNITMID);
                    pthread_cancel(interruptThread1);
                    pthread_join(interruptThread1, NULL); 
#endif

#if defined GT1318P
                    usleep(USLEEPTIMEUNITMID);
                    SerialFunctionFlag = 0;
                    pthread_join(serialThread, NULL); 
#endif
                    usleep(USLEEPTIMEUNITMID);
                    pthread_mutex_lock(&MutexWatchdog);
                    WatchdogThreadFlag = 0;
                    pthread_cond_signal(&CondWatchdog);
                    pthread_mutex_unlock(&MutexWatchdog);
 
                    pthread_join(watchdogThread, NULL);

                    //ready to power off, so we don't need it
                    usleep(USLEEPTIMEUNITMID);
                    //InputThreadFlag = 0;
                    pthread_cancel(inputThread);
                    pthread_join(inputThread, NULL);

                    pthread_mutex_lock(&MutexFile);
                    WriteFile(MachPOWEROFF);
                    pthread_mutex_unlock(&MutexFile);
                    cancelThreadDone = 1;
                }else
                {
                    //change user we need restart it 
                    usleep(USLEEPTIMEUNITMID);
                    //InputThreadFlag = 0;
                    pthread_cancel(inputThread);
                    pthread_join(inputThread, NULL);

                    if(ZHNode != NULL)
                    {
                        free(ZHNode); 
                    }
                    pthread_mutex_lock(&MutexMain);
                    ButtonEnableFlag = 1;
                    pthread_mutex_unlock(&MutexMain);

                    InputThreadFlag = 1;
                    InputMode = 1;
                    rc = pthread_create(&inputThread, NULL, InputFunction, NULL);
                    assert(rc == 0);
                }
            }
            //upload
            memset(uploadFilePath, 0 ,sizeof(char)*INPUTLENGTH);           
            pthread_mutex_lock(&MutexFile);
            strcpy(uploadFilePath, ZHList->UploadFilePath);
            gettimeofday(&now, NULL);
            sprintf(ZHList->UploadFilePath, "%ld%s.txt", (long)now.tv_sec, MachineNo);
            pthread_mutex_unlock(&MutexFile); 

            printf("local: %s\nglobal: %s\n", uploadFilePath, ZHList->UploadFilePath);

            if(stat(uploadFilePath, &fileInfo) == 0)
            {
                int fileSize = fileInfo.st_size;
                if(fileSize > 0)
                {
                    pid_t proc = fork();
                    if(proc < 0)
                    {
                        printf("fork child fail\n");
                        unlink(uploadFilePath);
                        return 0;
                    }else if(proc == 0)
                    {
                        //execl("../../.nvm/v0.10.25/bin/node", "node", "../../mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                        execl("/home/pi/.nvm/v0.10.25/bin/node", "node", "/home/pi/mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                        exit(0);
                    }else
                    {
                        //int result = -1;
                        //wait(&result);
                        //printf("upload success\n");
                        waitpid(-1, NULL, WNOHANG);
                        pthread_mutex_lock(&MutexEEPROM);
                        if(LoopLeaveEventIndex == ZHNormalExitEvent || LoopLeaveEventIndex == ZHForceExitEvent)
                        {
                            EarseEEPROMData();
                            EarseEEPROMData();
                        }else
                        {
                            //sprintf(ZHList->GoodNo, "%ld", ExCount[GOODCOUNT]);
                            //WriteEEPROMData();
                        }
                        pthread_mutex_unlock(&MutexEEPROM);
                    }   
                }else 
                {
                    printf("unlink\n");
                    unlink(uploadFilePath);
                }
            }
            if(LoopLeaveEventIndex == ZHPowerOffEvent)
            {
                usleep(USLEEPTIMEUNITMID);
                pthread_cancel(eventListenThread);
                pthread_join(eventListenThread, NULL);

                usleep(USLEEPTIMEUNITMID);
                pthread_cancel(changeScreenThread);
                pthread_join(changeScreenThread, NULL);

                sprintf(ZHList->GoodNo, "%ld", ExCount[GOODCOUNT]);
                if(WriteEEPROMData())
                {
                    EarseEEPROMData();
                    EarseEEPROMData();
                }

                sleep(10);
                digitalWrite (ZHPIN31, LOW);
                pthread_mutex_lock(&MutexScreen);
                ScreenIndex = 2;
                UpdateScreenFunction(2);
                pthread_mutex_unlock(&MutexScreen);

                return 0;
            }else if(LoopLeaveEventIndex == ZHNormalExitEvent || LoopLeaveEventIndex == ZHForceExitEvent)
            {
                memset(Count, 0, sizeof(unsigned long)*EVENTSIZE);
                memset(ExCount, 0, sizeof(unsigned long)*EVENTSIZE);
                GoodCount = 0;                
                TotalBadCount = 0;

                pthread_mutex_lock(&MutexLinklist);
                InputNode *p = ZHList;
                if(p->link != NULL)
                {
                    ZHList = ZHList->link;

                    memset(ZHList->UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
                    gettimeofday(&now, NULL);
                    sprintf(ZHList->UploadFilePath,"%ld%s.txt",(long)now.tv_sec, MachineNo);
                    printf("[%s|%d]new file name:%s\n", __func__, __LINE__,ZHList->UploadFilePath);

                    free(p);
                }else if(p->link == NULL)
                {
                    //no more jobs
                    free(p);
                    ZHList = NULL;
                    digitalWrite (ZHPIN31, LOW);
                }else;
                if(OrderInBox <= 1)
                {
                    OrderInBox = 0;
                }else
                {
                    OrderInBox = OrderInBox -1;
                }
                pthread_mutex_unlock(&MutexLinklist);

                isFirstinLoop = 1;                
                printf("reset done\n");
 
                if(ScreenIndex == 0)
                {
                    pthread_mutex_lock(&MutexScreen);
                    UpdateScreenFunction(0);
                    pthread_mutex_unlock(&MutexScreen);
                }else if(ScreenIndex == 1)
                {
                    pthread_mutex_lock(&MutexScreen);
                    UpdateScreenFunction(1);
                    pthread_mutex_unlock(&MutexScreen);
                }else;
            }else;

            if(cancelThreadDone == 1)
            {
                break;
            }
        }
    }
    return 0;
}
