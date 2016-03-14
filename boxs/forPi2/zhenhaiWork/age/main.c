//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

//#define LOCALTEST

#include "zhenhai.h"
#include "standerInput.h"
#include "ftp.h"
#include "eeprom.h"

#include "ats_1/ats_1.h"
#define ATS_1

//#include "gt_1318p/gt_1318p.h"
//#define ZHGT1318P

#define GOODRATE 1.03
#define ZHMACHLOG "/home/pi/zhlog/machStageLog"

#if defined(ZHGT1318P) 
#define PINDEFINESHIFT 1
#else
#define PINDEFINESHIFT 2
#endif

void sig_fork(int signo)
{
    pid_t pid;
    int stat;
    pid=waitpid(0,&stat,WNOHANG);
    printf("%s: child process finish upload\n", __func__);
    
    return;
}

int WriteFile(int mode)
{
    FILE *filePtr, *logPtr;
    int forCount = 0;
    struct timeval now;
    //static struct  timeval changeIntoRepairmodeTimeStemp;
    struct ifreq ifr;
    int fd;
    time_t nowLog;
    struct tm ts;
    char buf[80];

    memset(buf, 0, sizeof(char)*80);

    InputNode *tempNode = ZHList;   
 
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ZHNETWORKTYPE, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    
    //printf("%s open file %s\n", __func__, ZHList->UploadFilePath);
    logPtr = fopen(ZHMACHLOG, "w");
    time(&nowLog);
    ts = *localtime(&nowLog);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

    switch(mode)
    {
        case MachRUNNING:
            fprintf(logPtr, "%s MachRunning", buf);
        break;
        case MachJobDone:
            fprintf(logPtr, "%s MachJobDone", buf);
        break;
        case MachLOCK:
            fprintf(logPtr, "%s MachLOCK", buf);
        break;
        case MachUNLOCK:
            fprintf(logPtr, "%s MachUNLOCK", buf);
        break;
        case MachSTOPForce1:
            fprintf(logPtr, "%s MachSTOPForce1", buf);
        break;
        case MachSTOPForce2:
        break;
        case MachSTART:
            fprintf(logPtr, "%s MachSTART", buf);
        break;
        case MachSTANDBY:
            fprintf(logPtr, "%s MachSTANDBY", buf);
        break;
        case MachRESUMEFROMPOWEROFF:
            fprintf(logPtr, "%s MachSUMEFROMPOWEROFF", buf);
        break;
        case MachPOWEROFF:
            fprintf(logPtr, "%s MachPOWEROFF", buf);
        break;
        default:
        ;

    }
    fclose(logPtr);

    if(ZHList != NULL)
    {
        filePtr = fopen(ZHList->UploadFilePath, "a");
        if(filePtr != NULL)
        {
            switch(mode)
            {
                case MachRUNNING:
                    gettimeofday(&now, NULL);            
                    for(forCount = 0; forCount < EVENTSIZE; forCount++)
                    {
                        if(Count[forCount] != ExCount[forCount])
                        {
                            if(forCount == GOODCOUNT)
                            {
                                if(abs(Count[forCount] - ExCount[forCount]) > ZHMAXOUTPUT)
                                {
                                    fprintf(filePtr, "%s %s %s -1 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachRUNNING);
                                }else
                                {
                                    fprintf(filePtr, "%s %s %s %ld %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, 
                                                                Count[forCount] - ExCount[forCount], (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachRUNNING);
                                }
                            }else
                            {
                                fprintf(filePtr, "%s %s %s 0 %ld %ld %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                Count[forCount] - ExCount[forCount],
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                forCount+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachRUNNING);
                            }
                            }else;
                    }
                break;
                /*
                case MachREPAIRING:
                    gettimeofday(&changeIntoRepairmodeTimeStemp, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, RepairNo, MachREPAIRING);
            
                break;
                case MachREPAIRING2:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                atoi(FixItemNo), MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRING);
                break;
                case MachREPAIRDone:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRDone);
                    break;
                */
                case MachJobDone:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachJobDone);
                break;
                case MachLOCK:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachLOCK);          
                break;
                case MachUNLOCK:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachUNLOCK); 
                break;
                case MachSTOPForce1:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachSTOPForce1);        
                break;
                case MachSTOPForce2:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachSTOPForce2);        
                break;
                case MachSTART:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachSTART);
                break;
                case MachSTANDBY:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s 0 0 0 0 %02d\n", (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, MachSTANDBY);
                break;
                case MachRESUMEFROMPOWEROFF:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s 0 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, MachRESUMEFROMPOWEROFF);
                break;
                case MachPOWEROFF:
                    gettimeofday(&now, NULL);
                    fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, ZHList->UserNo, MachPOWEROFF);
                    sleep(1);     
                    while(tempNode->link != NULL)
                    {
                        tempNode = tempNode->link;
                        gettimeofday(&now, NULL);
                        fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                tempNode->ISNo, tempNode->ManagerCard, tempNode->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, tempNode->UserNo, MachPOWEROFF);
                        sleep(1);
                    }
                break;
                default:
                ;
            }
            //write data into file
            fclose(filePtr);
        }else
        {
            printf("open file fail\n");
        }
    }
    return 0;
}

void * PowerOffEventListenFunction(void *argument)
{
    printf("%s start\n", __func__);
    while(1)
    {
        if(digitalRead(ZHPIN29) == 1)
        {
            ZHResetFlag = 1;
            LoopLeaveEventIndex = ZHPowerOffEvent;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            printf("even trigger (PIN_29)1\n");
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
           // sleep(1);
        }
        nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITLONG}}, NULL);
    }
    printf("[%s|%d]exit\n",__func__, __LINE__);
}


void * EventListenFunction(void *argument)
{
    while(1)
    {
        while((!ButtonEnableFlag) && LoopLeaveEventIndex != ZHPowerOffEvent)
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

            }else if(digitalRead(ZHPIN7) == 1)
            {
                //finish job
                printf("event trigger (PIN_7)!\n");
                ZHResetFlag = 1;
                LoopLeaveEventIndex = ZHForceExitEvent;
                usleep(USLEEPTIMEUNITSHORT);
            }/*else if(digitalRead(ZHPIN29) == 1)
            {
#ifndef LOCALTEST
                printf("event trigger (PIN_29)\n");
                ZHResetFlag = 1;
                LoopLeaveEventIndex = ZHPowerOffEvent;
                usleep(USLEEPTIMEUNITSHORT);
#endif
            }*/
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
        }else if(LoopLeaveEventIndex == ZHPowerOffEvent)
        {
            printf("poweroff leave\n");
            break;
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
    pthread_t inputThread, eventListenThread, watchdogThread , changeScreenThread, poweroffThread;

#if defined ATS_1
    pthread_t interruptThread1; 
#endif

#if defined ZHGT1318P
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
//#ifdef LOCALTEST
    if(0)
//#else
//    if(!GetRemoteDataFunction())
//#endif
    {
        Count[GOODCOUNT] = ExCount[GOODCOUNT] = atol(ZHList->GoodNo);
        WriteFile(MachRESUMEFROMPOWEROFF);
        pthread_mutex_lock(&MutexScreen);
        DisableUpDown = 1;
        ScreenIndex = 0;
        UpdateScreenFunction(0, 0);
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
        //pthread_mutex_lock(&MutexScreen);
        //DisableUpDown = 1;
        //ScreenIndex = 0;
        //UpdateScreenFunction(0, 0);
        //pthread_mutex_unlock(&MutexScreen);
        InputMode = 1;
        printf("get data from local\n");
    }else
    {
        printf("load data fail\n");
        EarseEEPROMData();
        if(ZHList != NULL)
        {
            free(ZHList);
            ZHList = NULL;
        }
        //sleep(1);
        pthread_mutex_lock(&MutexScreen);
        ScreenIndex = 3;
        UpdateScreenFunction(3, 0);
        pthread_mutex_unlock(&MutexScreen);
    }

    rc = pthread_create(&eventListenThread, NULL, EventListenFunction, NULL);
    assert(rc == 0);

    InputThreadFlag = 1;
    rc = pthread_create(&inputThread, NULL, InputFunction, NULL);
    assert(rc == 0);

    rc = pthread_create(&changeScreenThread, NULL, ChangeScreenEventListenFunction, NULL);
    assert(rc == 0);

    rc = pthread_create(&poweroffThread, NULL, PowerOffEventListenFunction, NULL);
    assert(rc == 0);

    InputDone = 0;
    while(1)
    {
        unsigned char cancelThreadDone = 0;
        while((ZHList == NULL || InputDone == 0) && LoopLeaveEventIndex != ZHPowerOffEvent)
        {
            //printf("ZHList is NULL\n");
            usleep(USLEEPTIMEUNITLONG);
        }

        if(ZHList != NULL)
        {
            if(isFirstinLoop)
            {
                //first time, we need to write MachSTAR to FILE
                WriteFile(MachSTART);
                isFirstinLoop = 0;
                digitalWrite (ZHPIN31, HIGH);
            }
            printf("%s %s %s %s\n", ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, MachineNo); 
        }
 
#if defined ATS_1
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
#endif

#if defined ZHGT1318P
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
                    
#if defined ZHGT1318P
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

#if defined ZHGT1318P
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
                    if(ExCount[GOODCOUNT] >= (long)(atol(ZHList->CountNo)/GOODRATE))
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

#if defined ZHGT1318P
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
            printf("ready to upload\n");
            memset(uploadFilePath, 0 ,sizeof(char)*INPUTLENGTH);           
            pthread_mutex_lock(&MutexFile);
            if(ZHList != NULL)
            {
                printf("ZHList is not null\n");
                strcpy(uploadFilePath, ZHList->UploadFilePath);
                gettimeofday(&now, NULL);
                sprintf(ZHList->UploadFilePath, "%ld%s.txt", (long)now.tv_sec, MachineNo);
                printf("local: %s\nglobal: %s\n", uploadFilePath, ZHList->UploadFilePath);
            }else
            {
                printf("ZHList is null, create a temp fail\n");
                gettimeofday(&now, NULL);
                sprintf(uploadFilePath, "%ld%s.txt", (long)now.tv_sec, MachineNo);
                printf("local: %s\n", uploadFilePath);
            }
            pthread_mutex_unlock(&MutexFile); 

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
                    }   
                }else 
                {
                    printf("unlink\n");
                    unlink(uploadFilePath);
                }
            }
            if(LoopLeaveEventIndex == ZHPowerOffEvent)
            {
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(poweroffThread);
                pthread_join(poweroffThread, NULL);

                usleep(USLEEPTIMEUNITMID);
                pthread_cancel(eventListenThread);
                pthread_join(eventListenThread, NULL);

                usleep(USLEEPTIMEUNITMID);
                pthread_cancel(changeScreenThread);
                pthread_join(changeScreenThread, NULL);

                if(ZHList != NULL)
                {
                    sprintf(ZHList->GoodNo, "%ld", ExCount[GOODCOUNT]);
                }
                if(WriteEEPROMData())
                {
                    printf("write EEPROM fail\n");
                    EarseEEPROMData();
                }

                sleep(10);
                digitalWrite (ZHPIN31, LOW);
                pthread_mutex_lock(&MutexScreen);
                ScreenIndex = 2;
                UpdateScreenFunction(2, 0);
                pthread_mutex_unlock(&MutexScreen);
                printf("bye\n");

                return 0;
            }else if(LoopLeaveEventIndex == ZHNormalExitEvent || LoopLeaveEventIndex == ZHForceExitEvent)
            {
                pthread_mutex_lock(&MutexEEPROM);
                EarseEEPROMData();
                pthread_mutex_unlock(&MutexEEPROM);

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
                    UpdateScreenFunction(0, 0);
                    pthread_mutex_unlock(&MutexScreen);
                }else if(ScreenIndex == 1)
                {
                    pthread_mutex_lock(&MutexScreen);
                    UpdateScreenFunction(1, 0);
                    pthread_mutex_unlock(&MutexScreen);
                }else;
            }else;

            if(cancelThreadDone == 1)
            {
                break;
            }
            printf("[%s|%d]ready to wait again\n",__func__, __LINE__);
        }
    }
    return 0;
}
