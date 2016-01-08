//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"

//#define LOCALTEST

#include "lock.h"
#include "standerInput.h"
#include "eeprom.h"
#include "ftp.h"

#include "tsw100/tsw100.h"
#define ZHTSW100

//#include "tsw303/tsw303.h"
//#define ZHTSW303

//#include "m800/m800.h"
//#define ZHM800

//#include "m3050/m3050.h"
//#define ZHM3050

//#include "taicon/taicon.h"
//#define ZHTAICON

//#include "ncr236/ncr236.h"
//#define ZHNCR236

//#include "m3100/m3100.h"
//#define ZHM3100

//#include "m2200/m2200.h"
//#define ZHM2200

//#include "m2200a/m2200a.h"
//#define ZHM2200A

//#include "m2600/m2600.h"
//#define ZHM2600

//#include "m168t/m168t.h"
//#define ZHM168T

//#include "sph3000/sph3000.h"
//#define ZHSPH3000

static unsigned char WaitMainFunctionScanBarcodeDone;

//nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
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
        CanChangeRepairModeFlag = 0;     
        InputDone = 0;
        if(BarcodeIndex == 0 || BarcodeIndex == SCANDONE)
        {
            BarcodeIndex = ISNO;
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
            }
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
        }
        //2
        InputDone = 0;
        if(BarcodeIndex == ISNO)
        {
            BarcodeIndex = MANAGERCARD;
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
            }
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
        }
        //3
        InputDone = 0;
        if(BarcodeIndex == MANAGERCARD)
        {
            BarcodeIndex = COUNTNO;
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
            }
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
        }
        //4
        InputDone = 0;
        if(BarcodeIndex == COUNTNO)
        {
            BarcodeIndex = USERNO;
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
            }
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
        }
        BarcodeIndex = SCANDONE;
        CanChangeRepairModeFlag = 1;
        
        while(!UploadFileFlag)
        {
            nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITLONG}}, NULL);
            //printf("we wait\n");
        }

        while(1)
        {
            ZHResetFlag = 0;
            LoopLeaveEventIndex = ZHTimeoutExitEvent;
            pthread_mutex_lock(&MutexEEPROM);
            //because write eeprom need more time, a check for go through too fast
            pthread_mutex_unlock(&MutexEEPROM);
 
            while(!ZHResetFlag)
            {
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                GoodCount = ExCount[GOODCOUNT];
#ifdef M3100
                if(ExCount[GOODCOUNT]-ExCount[GOODCOUNT-1] >= atol(CountNo))
                //if(ExCount[GOODCOUNT] >= 0)
                {
                    //finish job                 
                    printf("Good Count Number arrival!\n");
                    ZHResetFlag = 1; 
                    LoopLeaveEventIndex = ZHNormalExitEvent;
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                }
#else
                if(ExCount[GOODCOUNT] >= atol(CountNo))
                //if(ExCount[GOODCOUNT] >= 0)
                {
                    //finish job                 
                    printf("Good Count Number arrival!\n");
                    ZHResetFlag = 1; 
                    LoopLeaveEventIndex = ZHNormalExitEvent;
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                }
#endif
                else if(digitalRead(ZHPIN40) == 0)
                {
                    printf("event trigger (PIN_40)!\n");
                    ZHResetFlag = 1;
                    LoopLeaveEventIndex = ZHChangeUserExitEvent;                    
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                }else if(digitalRead(ZHPIN7) == 1)
                {
                    //finish job
                    printf("event trigger (PIN_7)\n");
                    ZHResetFlag = 1;
                    LoopLeaveEventIndex = ZHForceExitEvent;
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                }else if(digitalRead(ZHPIN29) == 1)
                {
#ifndef LOCALTEST
                    printf("even trigger (PIN_29)");
                    ZHResetFlag = 1;
                    LoopLeaveEventIndex = ZHPowerOffEvent;
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
#endif
                }
                else;
            }       

            pthread_mutex_lock(&MutexMain);
            UploadFileFlag = 0;
            pthread_cond_signal(&CondMain);
            pthread_mutex_unlock(&MutexMain);
            
            if(LoopLeaveEventIndex == ZHNormalExitEvent || LoopLeaveEventIndex == ZHForceExitEvent)
            {
                //normal leave
                printf("not timeout leave\n");
                break;
            }else
            {
                //timeout leave
                printf("timeout leave\n");
                WaitMainFunctionScanBarcodeDone = 1;
                while(WaitMainFunctionScanBarcodeDone)
                {
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITLONG}}, NULL);
                }
            }
        }
    }
}

int main()
{
    pthread_t inputThread, eventListenThread, watchDogThread, changeScreenThread;

#if defined(ZHTSW100) || defined(ZHM2200A)
    pthread_t interruptThread1, interruptThread2, interruptThread3;
#endif

//#ifdef ZHM2200A
//    pthread_t interruptThread1, interruptThread2, interruptThread3;
//#endif

#if defined (ZHM800) || defined (ZHM3100) || defined (ZHM2200)
    pthread_t interruptThread1, interruptThread2;
#endif

//#ifdef ZHM3100
//    pthread_t interruptThread1, interruptThread2;
//#endif

#if defined (ZHTAICON) || defined (ZHNCR236)
    pthread_t interruptThread1;
#endif

//#ifdef ZHNCR236
//    pthread_t interruptThread1;
//#endif

#if defined (ZHTSW303) || defined (ZHM3050) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
    pthread_t serialThread;
#endif

//#ifdef ZHM3050
//    pthread_t serialThread;
//#endif

//#ifdef ZHM2200
//    pthread_t interruptThread1;
//#endif

//#ifdef ZHM2600
//    pthread_t serialThread;
//#endif

//#ifdef ZHM168T
//    pthread_t serialThread;
//#endif

//#ifdef ZHSPH3000
//    pthread_t serialThread;
//#endif

    int rc = 0;
    int filesize = 0;
    struct timeval now;
    struct timespec outtime;
    FILE *filePtr;
    char *buffer;
    int isFirstinLoop = 1; //確保能夠跑進START 只有一次
    struct stat fileInfo; 

    pthread_mutex_init(&MutexMain, NULL);
    pthread_mutex_init(&MutexEEPROM, NULL);
    pthread_mutex_init(&MutexInput, NULL);
    pthread_mutex_init(&MutexFile, NULL);
    pthread_mutex_init(&MutexWatchdog, NULL);

#if defined (ZHTSW303) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
    pthread_mutex_init(&MutexSerial, NULL);
#endif

//#ifdef ZHM2600
//    pthread_mutex_init(&MutexSerial, NULL);
//#endif

//#ifdef ZHM168T
//pthread_mutex_init(&MutexSerial, NULL);
//#endif

//#ifdef ZHSPH3000
//pthread_mutex_init(&MutexSerial, NULL);
//#endif

#ifdef ZHM3050
    pthread_mutex_init(&MutexSerial, NULL);
    pthread_cond_init(&CondSerial,NULL);
#endif

    pthread_cond_init(&CondMain, NULL);
    pthread_cond_init(&CondWatchdog, NULL);
       
    memset(Count, 0, sizeof(unsigned long)*EVENTSIZE);
    memset(ExCount, 0, sizeof(unsigned long)*EVENTSIZE);
    GoodCount = 0;
    TotalBadCount = 0;

#if defined (ZHTSW100) || defined (ZHM800) || defined (ZHM3100)
    memset(I2CEXValue, 0, sizeof(int)*6);
    memset(CutRoll, 0, sizeof(short)*2);
#endif

#ifdef ZHM2200A
    memset(I2CEXValue, 0, sizeof(int)*6);
    memset(CutRoll, 0, sizeof(short)*4);
#endif

//#ifdef ZHM800
//    memset(I2CEXValue, 0, sizeof(int)*6);
//    memset(CutRoll, 0, sizeof(short)*2);
//#endif

//#ifdef ZHM3100
//    memset(I2CEXValue, 0, sizeof(int)*6);
//    memset(CutRoll, 0, sizeof(short)*2);
//#endif

#if defined (ZHTAICON) || defined (ZHNCR236) || defined (ZHM2200)
    memset(I2CEXValue, 0, sizeof(int)*6);
#endif

//#ifdef ZHNCR236
//    memset(I2CEXValue, 0, sizeof(int)*6);
//#endif

//#ifdef ZHM2200
//    memset(I2CEXValue, 0, sizeof(int)*6);
//#endif

#ifdef ZHM2600
    memset(MessageArray, 0, sizeof(int)*6);
    memset(ExMessageArray, 0, sizeof(int)*6);
#endif

#ifdef ZHM168T
    memset(MessageArray, 0, sizeof(int)*4);
    memset(ExMessageArray, 0, sizeof(int)*4);
#endif

#ifdef ZHSPH3000
    MessageArray = ExMessageArray = 0;
#endif

    DisableUpDown = 0;
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
    //pinMode(ZHPIN24, OUTPUT);
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
    //unmask for test batter 
    //digitalWrite (ZHPIN31, HIGH);

    memset(UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
    gettimeofday(&now, NULL);
    sprintf(UploadFilePath,"%ld%s.txt",(long)now.tv_sec, MachineNo);
    printf("new file name:%s\n", UploadFilePath);

    WriteFile(MachSTANDBY);
    if(stat(UploadFilePath, &fileInfo) == 0)
    {
        int fileSize = fileInfo.st_size;
        if(fileSize > 0)
        {
            pid_t proc = fork();
            if(proc < 0)
            {
                printf("fork child fail\n");
                return 0;
            }else if(proc == 0)
            {
                //execl("../.nvm/v0.10.25/bin/node", "node", "../mongodb/SendDataClient.js", UploadFilePath, (char *)0);
                execl("/home/pi/.nvm/v0.10.25/bin/node", "node", "/home/pi/mongodb/SendDataClient.js", UploadFilePath, (char *)0);
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
            unlink(UploadFilePath);
        }
    }
    printf("old file name:%s\n", UploadFilePath);
    memset(UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
    gettimeofday(&now, NULL);
    sprintf(UploadFilePath,"%ld%s.txt",(long)now.tv_sec+1, MachineNo);
    printf("new file name:%s\n", UploadFilePath);

#ifdef LOCALTEST
    if(0)
#else
    if(!GetRemoteDataFunction())
#endif
    {
        WriteFile(MachRESUMEFROMPOWEROFF);
        Count[GOODCOUNT] = ExCount[GOODCOUNT] = atol(GoodNo);
        GoodCount = ExCount[GOODCOUNT];
        BarcodeIndex = COUNTNO;
        sleep(1);
        DisableUpDown = 1;
        ScreenIndex = 0;
        UpdateScreenFunction(0);
        printf("get data from remote\n");  
    }
#ifdef LOCALTEST
    else if(0)
#else
    else if(!ReadEEPROMData())
#endif
    {
        //read data from eeprom
        Count[GOODCOUNT] = ExCount[GOODCOUNT] = atol(GoodNo);
        GoodCount = ExCount[GOODCOUNT];
        BarcodeIndex = COUNTNO;
        sleep(1);
        DisableUpDown = 1;
        ScreenIndex = 0;
        UpdateScreenFunction(0);
        printf("get data from local\n");  
    }else
    {
        printf("load data fail\n");
        ZHEarseEEPROMData();
        ScreenIndex = 3;
        UpdateScreenFunction(3);
    }
   
    rc = pthread_create(&inputThread, NULL, InputFunction, NULL);
    assert(rc == 0);

    rc = pthread_create(&eventListenThread, NULL, EventListenFunction, NULL);
    assert(rc == 0);
    
    rc = pthread_create(&changeScreenThread, NULL, ChangeScreenEventListenFunction, NULL);
    assert(rc == 0);

    while(1)
    {
        unsigned char cancelThreadDone = 0;
        
        while(BarcodeIndex != SCANDONE)
        {
            nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITLONG}}, NULL);
        }
        if(isFirstinLoop)
        {
            //first time, we need to write MachSTAR to FILE
            WriteFile(MachSTART);
            isFirstinLoop = 0;
            digitalWrite (ZHPIN31, HIGH);
        }
#ifdef DEUBG 
        printf("[%s|%d]", __func__, __LINE__);
#endif       
        printf("%s %s %s %s %s %s\n", ISNo, ManagerCard, CountNo, UserNo, MachineNo, UploadFilePath);

#if defined (ZHTSW100) || defined (ZHM2200A)
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
        
        rc = pthread_create(&interruptThread2, NULL, ZHI2cReaderFunction2, NULL);
        assert(rc == 0);
    
        rc = pthread_create(&interruptThread3, NULL, ZHI2cReaderFunction3, NULL);
        assert(rc == 0);
#endif

/*
#ifdef ZHM2200A
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
        
        rc = pthread_create(&interruptThread2, NULL, ZHI2cReaderFunction2, NULL);
        assert(rc == 0);
    
        rc = pthread_create(&interruptThread3, NULL, ZHI2cReaderFunction3, NULL);
        assert(rc == 0);
#endif
*/

#ifdef ZHM2200
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
#endif

#if defined (ZHM800) || defined (ZHM3100) 
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
        
        rc = pthread_create(&interruptThread2, NULL, ZHI2cReaderFunction2, NULL);
        assert(rc == 0); 
#endif

/*
#ifdef ZHM3100
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
        
        rc = pthread_create(&interruptThread2, NULL, ZHI2cReaderFunction2, NULL);
        assert(rc == 0); 
#endif
*/

#if defined (ZHTAICON) || defined (ZHNCR236)
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
#endif
/*
#ifdef ZHNCR236
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
#endif
*/

#if defined (ZHTSW303) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#endif

/*
#ifdef ZHM2600
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#endif

#ifdef ZHM168T
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#endif

#ifdef ZHSPH3000
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#endif
*/

#ifdef ZHM3050
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#else        
        WatchdogThreadFlag = 1; 
        rc = pthread_create(&watchDogThread, NULL, WatchdogFunction, NULL);
        assert(rc == 0);
#endif
        pthread_mutex_lock(&MutexMain);
        UploadFileFlag = 1;
        pthread_mutex_unlock(&MutexMain);
        while(1)
        {
            char uploadFilePath[INPUTLENGTH];           
            if(!UploadFileFlag);
            else
            {
                pthread_mutex_lock(&MutexMain);
                gettimeofday(&now, NULL);
                printf("[%s|%d] %ld and ready to sleep\n", __func__, __LINE__,(long)now.tv_sec);
                outtime.tv_sec = now.tv_sec + 300;
                outtime.tv_nsec = now.tv_usec * 1000;
                pthread_cond_timedwait(&CondMain, &MutexMain, &outtime);
                pthread_mutex_unlock(&MutexMain);
                gettimeofday(&now, NULL);
                printf("[%s|%d] %ld wake up\n", __func__, __LINE__,(long)now.tv_sec);
            }
            if(!UploadFileFlag)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();

#if defined (ZHTSW100) || defined (ZHM2200A)
                printf("ready to cancel thread\n");
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
        
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread2);
                pthread_join(interruptThread2, NULL);
        
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread3);
                pthread_join(interruptThread3, NULL);
#endif
/*
#ifdef ZHM2200A
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
        
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread2);
                pthread_join(interruptThread2, NULL);
        
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread3);
                pthread_join(interruptThread3, NULL);
#endif
*/
#if defined (ZHTAICON) || defined (ZHNCR236) || defined (ZHM2200)
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
#endif
/*
#ifdef ZHNCR236
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
#endif

#ifdef ZHM2200
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
#endif
*/
#if defined (ZHM800) || defined (ZHM3100)
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
        
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread2);
                pthread_join(interruptThread2, NULL);
#endif
/*
#ifdef ZHM3100
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
        
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread2);
                pthread_join(interruptThread2, NULL);
#endif
*/
#if defined (ZHTSW303) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                SerialFunctionFlag = 0;
                pthread_join(serialThread, NULL);
#endif                

/*
#ifdef ZHM2600
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                SerialFunctionFlag = 0;
                pthread_join(serialThread, NULL);
#endif  

#ifdef ZHM168T
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                SerialFunctionFlag = 0;
                pthread_join(serialThread, NULL);
#endif  


#ifdef ZHSPH3000
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                SerialFunctionFlag = 0;
                pthread_join(serialThread, NULL);
#endif 
*/

#ifdef ZHM3050
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                SerialFunctionFlag = 0;
                pthread_join(serialThread, NULL);
                printf("cancel done %d\n", LoopLeaveEventIndex);
#else
                printf("ready to cancle watchdogThread %d\n", LoopLeaveEventIndex);
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_mutex_lock(&MutexWatchdog);
                WatchdogThreadFlag = 0;
                pthread_cond_signal(&CondWatchdog);
                pthread_mutex_unlock(&MutexWatchdog);
 
                pthread_join(watchDogThread, NULL);
                printf("cancel done %d\n", LoopLeaveEventIndex);
#endif
                cancelThreadDone = 1;

                if(LoopLeaveEventIndex == ZHNormalExitEvent)
                {
                    //normal leave 
                    pthread_mutex_lock(&MutexFile);
                    WriteFile(MachJobDone);
                    WriteFile(MachSTANDBY);    
                    pthread_mutex_unlock(&MutexFile);                    
                }else if(LoopLeaveEventIndex == ZHForceExitEvent)
                {
                    //force leave
                    pthread_mutex_lock(&MutexFile);
#ifdef ZHM3100
                    if(ExCount[GOODCOUNT] - ExCount[GOODCOUNT-1] >= (atol(CountNo)/ 1.04))
                    {
                        WriteFile(MachSTOPForce1);
                        WriteFile(MachSTANDBY);    
                    }
#else
                    if(ExCount[GOODCOUNT] >= (atol(CountNo)/1.04))
                    {
                        WriteFile(MachSTOPForce1);
                        WriteFile(MachSTANDBY);    
                    }
#endif
                    else
                    {
                        WriteFile(MachSTOPForce2);                    
                        WriteFile(MachSTANDBY);    
                    }
                    pthread_mutex_unlock(&MutexFile);
                }else if(LoopLeaveEventIndex == ZHPowerOffEvent)
                {
                    pthread_mutex_lock(&MutexFile);
                    WriteFile(MachPOWEROFF);
                    pthread_mutex_unlock(&MutexFile);
                }else
                { 
                    char tempUserNo[INPUTLENGTH];

                    memset(tempUserNo, 0, sizeof(char)*INPUTLENGTH);  
                    //change user
                    pthread_mutex_lock(&MutexFile);
                    WriteFile(MachLOCK);
                    pthread_mutex_unlock(&MutexFile);
                    
                    //need upload right now 
                    memset(uploadFilePath, 0 ,sizeof(char)*INPUTLENGTH);           
                    pthread_mutex_lock(&MutexFile);
                    strcpy(uploadFilePath, UploadFilePath);
                    gettimeofday(&now, NULL);
                    sprintf(UploadFilePath, "%ld%s.txt", (long)now.tv_sec, MachineNo);
                    pthread_mutex_unlock(&MutexFile); 

                    printf("Uplaod for timeout or changeUser ");
                    printf("old file:%s, new file %s\n", uploadFilePath, UploadFilePath);

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
                                //execl("../.nvm/v0.10.25/bin/node", "node", "../mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                                execl("/home/pi/.nvm/v0.10.25/bin/node", "node", "/home/pi/mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                                exit(0);
                            }else
                            {
                                //int result = -1;
                                //wait(&result);
                                //printf("upload success\n");
                                waitpid(-1, NULL, WNOHANG);
                                sprintf(GoodNo, "%ld", ExCount[GOODCOUNT]);
                            }   
                        }else
                        {
                            unlink(uploadFilePath);
                        }
                    }
                    //unlink(uploadFilePath);

                    //reset for hinet user need to scan
                    strcpy(tempUserNo, UserNo); 
                    memset(UserNo, 0, sizeof(char)*INPUTLENGTH);
                    DisableUpDown = 1;
                    ScreenIndex = 0;
                    UpdateScreenFunction(0); 

                    InputDone = 0;
                    BarcodeIndex = USERNO;
                    while(InputDone == 0)
                    {
                        nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                    }
                   
                    if(CanChangeRepairModeFlag == 2)
                    {
                        //into repair mode
                        pthread_mutex_lock(&MutexFile);
                        WriteFile(MachREPAIRING);
                        pthread_mutex_unlock(&MutexFile);
                        isInPairMode = 1;
                        //DisableUpDown = 1;
                        //ScreenIndex = 7;
                        //UpdateScreenFunction(7);    
                        DisableUpDown = 1;
                        ScreenIndex = 0;
                        UpdateScreenFunction(0);    

                        //need upload right now 
                        memset(uploadFilePath, 0 ,sizeof(char)*INPUTLENGTH);           
                        pthread_mutex_lock(&MutexFile);
                        strcpy(uploadFilePath, UploadFilePath);
                        gettimeofday(&now, NULL);
                        sprintf(UploadFilePath, "%ld%s.txt", (long)now.tv_sec, MachineNo);
                        pthread_mutex_unlock(&MutexFile); 

                        printf("[%s|%d]old file:%s, new file %s\n", __func__, __LINE__, uploadFilePath, UploadFilePath);

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
                                    //execl("../.nvm/v0.10.25/bin/node", "node", "../mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                                    execl("/home/pi/.nvm/v0.10.25/bin/node", "node", "/home/pi/mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                                    exit(0);
                                }else
                                {
                                    //int result = -1;
                                    //wait(&result);
                                    waitpid(-1, NULL, WNOHANG);
                                }   
                            }
                            else 
                                printf("unlink\n");
                                unlink(uploadFilePath);
                        }
 
                        while(1)
                        {
                            InputDone = 0;
                            BarcodeIndex = FIXITEM;
                            while(InputDone == 0)
                            {
                                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                            }                            
                            if(CanChangeRepairModeFlag == 4)
                            {
                                pthread_mutex_lock(&MutexFile);
                                WriteFile(MachREPAIRDone);
                                pthread_mutex_unlock(&MutexFile);
                                BarcodeIndex = SCANDONE;                                
                                break;
                            }else
                            {
                                pthread_mutex_lock(&MutexFile);
                                WriteFile(MachREPAIRING2);
                                pthread_mutex_unlock(&MutexFile);
                                DisableUpDown = 1;
                                ScreenIndex = 0;
                                UpdateScreenFunction(0);    
                            }
                        }
                        strcpy(UserNo, tempUserNo); 
                        isInPairMode = 0;

                    }else
                    {
                        printf("[%s|%d]%s %s %s %s %s %s\n", __func__, __LINE__,ISNo, ManagerCard, CountNo, UserNo, MachineNo, UploadFilePath);
                        BarcodeIndex = SCANDONE;
                    }
                    pthread_mutex_lock(&MutexFile);
                    WriteFile(MachUNLOCK);
                    pthread_mutex_unlock(&MutexFile);
                    WaitMainFunctionScanBarcodeDone = 0;
                    DisableUpDown = 1;
                    ScreenIndex = 0;
                    UpdateScreenFunction(0);    
                }
            }
            //upload
            
            memset(uploadFilePath, 0 ,sizeof(char)*INPUTLENGTH);           
            pthread_mutex_lock(&MutexFile);
            strcpy(uploadFilePath, UploadFilePath);
            gettimeofday(&now, NULL);
            sprintf(UploadFilePath, "%ld%s.txt", (long)now.tv_sec, MachineNo);
            pthread_mutex_unlock(&MutexFile); 

            printf("[%s|%d]old:  %s |new: %s\n", __func__, __LINE__, uploadFilePath, UploadFilePath);

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
                        //execl("../.nvm/v0.10.25/bin/node", "node", "../mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                        execl("/home/pi/.nvm/v0.10.25/bin/node", "node", "/home/pi/mongodb/SendDataClient.js", uploadFilePath, (char *)0);
                        exit(0);
                    }else
                    {
                        //int result = -1;
                        //wait(&result);
                        //printf("upload success\n");
                        waitpid(-1, NULL, WNOHANG);
                        if(LoopLeaveEventIndex == ZHNormalExitEvent || LoopLeaveEventIndex == ZHForceExitEvent)
                        {
                        }else
                        {
                            ;
                        }
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
                pthread_cancel(inputThread);
                pthread_join(inputThread, NULL);
                
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(eventListenThread);
                pthread_join(eventListenThread, NULL);

                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(changeScreenThread);
                pthread_join(changeScreenThread, NULL);

                sprintf(GoodNo, "%ld", ExCount[GOODCOUNT]);
                
                if(WriteEEPROMData())
                {
                    printf("write data to EEPROM fail!! ready to earse eeprom\n");
                    ZHEarseEEPROMData();
                    ZHEarseEEPROMData();
                    printf("done\n");
                }

                sleep(10);
                digitalWrite (ZHPIN31, LOW);
                //user config for black screen;
                ScreenIndex = 2;
                UpdateScreenFunction(2);

                return 0;
            }else if(LoopLeaveEventIndex == ZHNormalExitEvent || LoopLeaveEventIndex == ZHForceExitEvent)
            {
                pthread_mutex_lock(&MutexEEPROM);
                ZHEarseEEPROMData();
                ZHEarseEEPROMData();
                printf("Erase eeprom done!\n");
                pthread_mutex_unlock(&MutexEEPROM);

                DisableUpDown = 0;
                memset(ISNo, 0, sizeof(char)*INPUTLENGTH);
                memset(ManagerCard, 0, sizeof(char)*INPUTLENGTH);
                memset(CountNo, 0, sizeof(char)*INPUTLENGTH);
                memset(UserNo, 0, sizeof(char)*INPUTLENGTH);

                memset(Count, 0, sizeof(unsigned long)*EVENTSIZE);
                memset(ExCount, 0, sizeof(unsigned long)*EVENTSIZE);
                GoodCount = 0;
                TotalBadCount = 0;

#if defined (ZHTSW100) || defined (ZHM800) || defined (ZHM3100)
                memset(I2CEXValue, 0, sizeof(int)*6);
                memset(CutRoll, 0, sizeof(short)*2);
#endif

#ifdef ZHM2200A
                memset(I2CEXValue, 0, sizeof(int)*6);
                memset(CutRoll, 0, sizeof(short)*4);
#endif
/*   
#ifdef ZHM800
                memset(I2CEXValue, 0, sizeof(int)*6);
                memset(CutRoll, 0, sizeof(short)*2);
#endif

#ifdef ZHM3100
                memset(I2CEXValue, 0, sizeof(int)*6);
                memset(CutRoll, 0, sizeof(short)*2);
#endif
*/

#if defined (ZHTAICON) || defined (ZHNCR236) || defined (ZHM2200) 
                memset(I2CEXValue, 0, sizeof(int)*6);
#endif

/*
#ifdef ZHNCR236
                memset(I2CEXValue, 0, sizeof(int)*6);
#endif
#ifdef ZHM2200
                memset(I2CEXValue, 0, sizeof(int)*6);
#endif
*/

#ifdef ZHM2600
                memset(MessageArray, 0, sizeof(int)*6);
                memset(ExMessageArray, 0, sizeof(int)*6);
#endif

#ifdef ZHM168T
                memset(MessageArray, 0, sizeof(int)*4);
                memset(ExMessageArray, 0, sizeof(int)*4);
#endif

#ifdef ZHSPH3000
                MessageArray = ExMessageArray = 0;
#endif
                //LED
                //digitalWrite (ZHPIN15, HIGH);
                //digitalWrite (ZHPIN16, HIGH);
                //digitalWrite (ZHPIN18, HIGH);   
                isFirstinLoop = 1;                
                digitalWrite (ZHPIN31, LOW);
                if(ScreenIndex == 0)
                {
                    UpdateScreenFunction(0);
                }else if(ScreenIndex == 1)
                {
                    UpdateScreenFunction(1);
                }else;
            }
            else;
            if(cancelThreadDone == 1)
            {
                printf("loop back cancelThreadDone 1\n");
                break;
            }else
            {
                printf("loop back cancelThreadDone 2\n");
            }
        }
    }
    return 0;
}

