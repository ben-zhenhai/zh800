//#ifndef ZHDef
//#define ZHDef "zhenhai.h"
//#include ZHDef
//#endif

#include "zhenhai.h"

//define localTest 的情況, 執行時不會跟server 撈data 方便local 做測試, 不然還要等timeout
//#define LOCALTEST

#include "lock.h"
#include "standerInput.h"
#include "eeprom.h"
#include "ftp.h"

#define GOODRATE 1.03


//針對不同機台需要inlcude 不同type 的機型
//#include "tsw100/tsw100.h"
//#define ZHTSW100

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

//#include "ncr236_2/ncr236.h"
//#define ZHNCR236_2

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

#if defined(ZHSPH3000) || defined(ZHM3050) || defined(ZHM2600) || defined(ZHM168T)
#define PINDEFINESHIFT 1
#else
#define PINDEFINESHIFT 2
#endif

// 紀錄機台的最後狀況, 方便zhenhai ssh 進去box 做確認
#define ZHMACHLOG "/home/pi/zhlog/machStageLog"
#define GOODRATE 1.03

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


// write data to file 都使用這個method, 傳入值為 machine status, 細節請參閱 zhenhai.h 或是Brian 提供之文件
int WriteFile(int mode)
{
    FILE *filePtr, *logPtr;
    int forCount = 0;
    struct timeval now;
    static struct  timeval changeIntoRepairmodeTimeStemp;
    struct ifreq ifr;
    int fd;
    time_t nowLog;
    struct tm ts;
    char buf[80];   

    memset(buf, 0 ,sizeof(char)*80);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ZHNETWORKTYPE, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);

    //printf("shift %d\n", PINDEFINESHIFT);
    
    filePtr = fopen(UploadFilePath, "a");
    logPtr = fopen(ZHMACHLOG, "w");
    time(&nowLog);
    ts = *localtime(&nowLog);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

    switch(mode)
    {
        case MachRUNNING:
            fprintf(logPtr, "%s MachRunning", buf);
            gettimeofday(&now, NULL);
            for(forCount = 0; forCount < EVENTSIZE; forCount++)
            {
                if(Count[forCount] != ExCount[forCount])
                {
                    if(forCount == GOODCOUNT)
                    {
#ifdef ZHM3100
                        if(abs((Count[forCount] - Count[forCount+1]) - (ExCount[forCount] - ExCount[forCount+1])) > ZHMAXOUTPUT )
                        {
                            fprintf(filePtr, "%s %s %s -1 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachRUNNING);
                        }
#else
                        if(abs(Count[forCount] - ExCount[forCount]) > ZHMAXOUTPUT)
                        {
                            fprintf(filePtr, "%s %s %s -1 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachRUNNING);
                        }
#endif
                        else
                        {
                            fprintf(filePtr, "%s %s %s %ld %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ISNo, ManagerCard, CountNo, 
                                                                Count[forCount] - ExCount[forCount], (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachRUNNING);
                        }
#ifdef ZHM3050
                        if(NewDataIncome == 0) NewDataIncome = 1;
#endif
                    }else
                    {
                        fprintf(filePtr, "%s %s %s 0 %ld %ld %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                Count[forCount] - ExCount[forCount],
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                forCount+PINDEFINESHIFT, MachineNo, UserNo, MachRUNNING);
                    }
                }else;
            }
        break;
        case MachREPAIRING:
            fprintf(logPtr, "%s MachREPAIRING", buf);
            gettimeofday(&changeIntoRepairmodeTimeStemp, NULL);
            if(strlen(ISNo)> 0)
            {
                fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ISNo, ManagerCard, CountNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, RepairNo, MachREPAIRING);
            }else
            {
                fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, RepairNo, MachREPAIRING);
            }
        break;
        case MachREPAIRING2:
            fprintf(logPtr, "%s MachREPAIRING2", buf);
            gettimeofday(&now, NULL);
            if(strlen(ISNo)>0)
            {
                fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                atoi(FixItemNo), MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRING);
            }else
            {
                fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                atoi(FixItemNo), MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRING);
            }           
        break;
        case MachREPAIRDone:
            fprintf(logPtr, "%s MachREPAIRDone", buf);
            gettimeofday(&now, NULL);
            if(strlen(ISNo)>0)
            {
                fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRDone);
            }else
            {
                fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRDone);
            }
        break;
        case MachJobDone:
            fprintf(logPtr, "%s MachJobDone", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachJobDone);
        break;
        case MachLOCK:
            fprintf(logPtr, "%s MachLOCK", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachLOCK);          
        break;
        case MachUNLOCK:
            fprintf(logPtr, "%s MachUNLOCK", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachUNLOCK); 
        break;
        case MachSTOPForce1:
            fprintf(logPtr, "%s MachSTOPForce1", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachSTOPForce1);        
        break;
        case MachSTOPForce2:
            fprintf(logPtr, "%s MachSTOPForce2", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachSTOPForce2);        
        break;
        case MachSTART:
            fprintf(logPtr, "%s MachSTART", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachSTART);
        break;
        case MachSTANDBY:
            fprintf(logPtr, "%s MachSTANDBY", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s 0 0 0 0 %02d\n", (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, MachSTANDBY);
        break;
        case MachRESUMEFROMPOWEROFF:
            fprintf(logPtr, "%s MachRESUMEPOWEROFF", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s 0 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, MachRESUMEFROMPOWEROFF);
        break;
        case MachPOWEROFF:
            fprintf(logPtr, "%s MachPOWEROFF", buf);
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+PINDEFINESHIFT, MachineNo, UserNo, MachPOWEROFF);
        break;
        default:
        ;
    }
    //write data into file
    fclose(filePtr);
    fclose(logPtr);
    return 0;
}

//這個method 已經不使用, lcd refresh 以交由 lcd.c 中的 sendCommandMessageFunction 做處理
void * LcdRefreshFunction(void *argument)
{
    struct timeval now;
    struct timespec outtime;

    while(LcdRefreshFlag)
    {
        pthread_mutex_lock(&MutexLcdRefresh);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + 300;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&CondLcdRefresh, &MutexLcdRefresh, &outtime);
        pthread_mutex_unlock(&MutexLcdRefresh);

        digitalWrite (ZHPIN33, LOW);
        //nanosleep((const struct timespec[]){{0, 200000000L}}, NULL);
        sleep(2);    

        digitalWrite (ZHPIN33, HIGH);
        //nanosleep((const struct timespec[]){{0, 1500000000L}}, NULL);
        sleep(2);    

        pthread_mutex_lock(&MutexScreen);
        UpdateScreenFunction(ScreenIndex);    
        pthread_mutex_unlock(&MutexScreen);
        printf("refresh2\n");
    }
    printf("[%s] exit\n",__func__);
}

//進入關電模式的listener method, 只要 gpio 拉high 就會被trigger

void * PowerOffEventListenFunction(void *argument)
{
    printf("%s start\n", __func__);
    while(1)
    {
        if((digitalRead(ZHPIN29) == 1)) //&& (LoopLeaveEventIndex != ZHPowerOffEvent))
        {
            ZHResetFlag = 1;
            LoopLeaveEventIndex = ZHPowerOffEvent;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
            printf("even trigger (PIN_29)\n");
            pthread_mutex_lock(&MutexMain);
            UploadFileFlag = 0;
            pthread_cond_signal(&CondMain);
            pthread_mutex_unlock(&MutexMain);
            //printf("even trigger (PIN_29)2\n");
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
           // sleep(1);
        }else nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITLONG}}, NULL);
        nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITLONG}}, NULL);
    }
    printf("[%s|%d]exit\n",__func__, __LINE__);
}

//cancel 管理卡資訊 method, 只要工單輸入完成, 這個pthread 就會被中止 然後那個gpio 就會被拿去做換班

void * CancelOrderFunction(void *argument)
{
    while(1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        if(strlen(ISNo) > 0 && BarcodeIndex != SCANDONE)
        {
            if(digitalRead(ZHPIN40) == 0)
            {
                printf("event trigger (PIN_40) clear data now\n");
                pthread_mutex_lock(&MutexInput);
                BarcodeIndex = 0;
                
                memset(ISNo, 0, sizeof(char)*INPUTLENGTH);
                memset(ManagerCard, 0, sizeof(char)*INPUTLENGTH);
                memset(CountNo, 0, sizeof(char)*INPUTLENGTH);
                memset(UserNo, 0, sizeof(char)*INPUTLENGTH);

                pthread_mutex_lock(&MutexScreen);
                DisableUpDown = 1;
                ScreenIndex = 0;
                UpdateScreenFunction(0);    
                pthread_mutex_unlock(&MutexScreen);
                pthread_mutex_unlock(&MutexInput);
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
            }
        }
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
    }
}

// 除了 poweroff event 跟 lcd 操作的 gpio 外, 所有event都在這邊做listen 
// 良品數量到達目標, 按下結單或是換班, 這三件事情
// BarcodeIndex 的變換也是由這邊跟main 做操作, standInput底下只做讀取
void * EventListenFunction(void *argument)
{   
    while(1)
    {    
        CanChangeRepairModeFlag = 0;     
        InputDone = 0;
        if(BarcodeIndex == 0 || BarcodeIndex == SCANDONE)
        {
            pthread_mutex_lock(&MutexInput);
            BarcodeIndex = ISNO;
            pthread_mutex_unlock(&MutexInput);
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
                if(BarcodeIndex != ISNO) break;
            }
            pthread_mutex_lock(&MutexScreen);
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
            pthread_mutex_unlock(&MutexScreen);
        }
        //2
        InputDone = 0;
        if(BarcodeIndex == ISNO)
        {
            pthread_mutex_lock(&MutexInput);
            BarcodeIndex = MANAGERCARD;
            pthread_mutex_unlock(&MutexInput);
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
                if(BarcodeIndex != MANAGERCARD) break;
            }
            pthread_mutex_lock(&MutexScreen);
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
            pthread_mutex_unlock(&MutexScreen);
        }
        //3
        InputDone = 0;
        if(BarcodeIndex == MANAGERCARD)
        {
            pthread_mutex_lock(&MutexInput);
            BarcodeIndex = COUNTNO;
            pthread_mutex_unlock(&MutexInput);
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
                if(BarcodeIndex != COUNTNO) break;
            }
            pthread_mutex_lock(&MutexScreen);
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
            pthread_mutex_unlock(&MutexScreen);
        }
        //4
        InputDone = 0;
        if(BarcodeIndex == COUNTNO)
        {
            pthread_mutex_lock(&MutexInput);
            BarcodeIndex = USERNO;
            pthread_mutex_unlock(&MutexInput);
            while(InputDone == 0)
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();
                if(BarcodeIndex != USERNO) break;
            }
            pthread_mutex_lock(&MutexScreen);
            DisableUpDown = 1;
            ScreenIndex = 0;
            UpdateScreenFunction(0);    
            pthread_mutex_unlock(&MutexScreen);
            
            if(BarcodeIndex == USERNO)
            { 
                pthread_mutex_lock(&MutexInput);
                BarcodeIndex = SCANDONE;
                pthread_mutex_unlock(&MutexInput);
                CanChangeRepairModeFlag = 1;
            }
        }
        
        if(BarcodeIndex == SCANDONE)
        {
            while(!UploadFileFlag) //UploadFileFlag == 0
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
 
                while(!ZHResetFlag) // ZHResetFlag == 0
                {
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                    GoodCount = ExCount[GOODCOUNT];
#ifdef M3100
                    if(ExCount[GOODCOUNT]-ExCount[GOODCOUNT-1] >= atol(CountNo))
                    {
                        //finish job                 
                        printf("Good Count Number arrival!\n");
                        ZHResetFlag = 1; 
                        LoopLeaveEventIndex = ZHNormalExitEvent;
                        nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                    }
#else
                    if(ExCount[GOODCOUNT] >= atol(CountNo))
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
                    }/*else if(digitalRead(ZHPIN29) == 1)
                    {
#ifndef LOCALTEST
                        printf("even trigger (PIN_29)");
                        ZHResetFlag = 1;
                        LoopLeaveEventIndex = ZHPowerOffEvent;
                        nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
#endif
                    }*/
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
}


// 執行 mongoDB的script 放入此,每5min 只要file size > 0就會執行一次
int main()
{
    pthread_t inputThread, eventListenThread, watchDogThread, changeScreenThread, poweroffThread, cancelOrderThread, lcdRefreshThread;

#if defined(ZHTSW100) || defined(ZHM2200A)
    pthread_t interruptThread1, interruptThread2, interruptThread3;
#endif

#if defined (ZHM800) || defined (ZHM3100) || defined (ZHM2200)
    pthread_t interruptThread1, interruptThread2;
#endif

#if defined (ZHTAICON) || defined (ZHNCR236) || defined (ZHNCR236_2)
    pthread_t interruptThread1;
#endif

#if defined (ZHTSW303) || defined (ZHM3050) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
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

    pthread_mutex_init(&MutexMain, NULL);
    pthread_mutex_init(&MutexEEPROM, NULL);
    pthread_mutex_init(&MutexInput, NULL);
    pthread_mutex_init(&MutexFile, NULL);
    pthread_mutex_init(&MutexWatchdog, NULL);
    pthread_mutex_init(&MutexLcdRefresh, NULL);
    pthread_mutex_init(&MutexScreen, NULL);

#if defined (ZHTSW303) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
    pthread_mutex_init(&MutexSerial, NULL);
#endif

#ifdef ZHM3050
    pthread_mutex_init(&MutexSerial, NULL);
    pthread_cond_init(&CondSerial,NULL);
#endif

    pthread_cond_init(&CondMain, NULL);
    pthread_cond_init(&CondLcdRefresh, NULL);
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

#if defined (ZHTAICON) || defined (ZHNCR236) || defined (ZHM2200) || defined(ZHNCR236_2)
    memset(I2CEXValue, 0, sizeof(int)*6);
#endif

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
    pinMode(ZHPIN37, INPUT);

    pullUpDnControl(ZHPIN22, PUD_UP);
    pullUpDnControl(ZHPIN29, PUD_UP);
    pullUpDnControl(ZHPIN32, PUD_UP);
    //pullUpDnControl(ZHPIN33, PUD_UP);
    pullUpDnControl(ZHPIN36, PUD_UP);
    pullUpDnControl(ZHPIN37, PUD_UP);
    pullUpDnControl(ZHPIN38, PUD_UP);
    pullUpDnControl(ZHPIN40, PUD_UP);
    pullUpDnControl(ZHPIN7, PUD_DOWN);

    pinMode(ZHPIN15, OUTPUT);
    pinMode(ZHPIN16, OUTPUT);
    pinMode(ZHPIN18, OUTPUT);
    //pinMode(ZHPIN24, OUTPUT);
    pinMode(ZHPIN31, OUTPUT);
    pinMode(ZHPIN33, OUTPUT);
    digitalWrite (ZHPIN33, HIGH);

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
        pthread_mutex_lock(&MutexScreen);
        DisableUpDown = 1;
        ScreenIndex = 0;
        UpdateScreenFunction(0);
        pthread_mutex_unlock(&MutexScreen);
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
        pthread_mutex_lock(&MutexScreen);
        DisableUpDown = 1;
        ScreenIndex = 0;
        UpdateScreenFunction(0);
        pthread_mutex_unlock(&MutexScreen);
        printf("get data from local\n");  
    }else
    {
        printf("load data fail\n");
        ZHEarseEEPROMData();
        pthread_mutex_lock(&MutexScreen);
        ScreenIndex = 3;
        UpdateScreenFunction(3);
        pthread_mutex_unlock(&MutexScreen);
    }

#ifdef ZHREFRESHSCREEN
    printf("[%s] Refresh screen function enable\n", __func__);
#endif

#ifdef ZHCHECKSCREENBUSY
    printf("[%s] Check Screen Busy function enable\n", __func__);
#endif

#ifdef ZHREFRESHSCREEN
    LcdRefreshFlag = 1;
    rc = pthread_create(&lcdRefreshThread, NULL, LcdRefreshFunction, NULL);   
    assert(rc == 0);
#endif

    rc = pthread_create(&inputThread, NULL, InputFunction, NULL);
    assert(rc == 0);

    rc = pthread_create(&eventListenThread, NULL, EventListenFunction, NULL);
    assert(rc == 0);
    
    rc = pthread_create(&changeScreenThread, NULL, ChangeScreenEventListenFunction, NULL);
    assert(rc == 0);
#ifndef LOCALTEST
    rc = pthread_create(&poweroffThread, NULL, PowerOffEventListenFunction, NULL);
    assert(rc == 0);
#endif 
    while(1)
    {
        unsigned char cancelThreadDone = 0;
        if(isFirstinLoop == 1)
        {
            rc = pthread_create(&cancelOrderThread, NULL, CancelOrderFunction, NULL);
            assert(rc == 0);
        }
        
        while(BarcodeIndex != SCANDONE && LoopLeaveEventIndex != ZHPowerOffEvent)
        {
            nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITLONG}}, NULL);
        }
        if(isFirstinLoop == 1)
        {
            pthread_cancel(cancelOrderThread);
            pthread_join(cancelOrderThread, NULL);
        }

        pthread_mutex_lock(&MutexMain);
        UploadFileFlag = 1;
        pthread_mutex_unlock(&MutexMain);

        if(isFirstinLoop && strlen(ISNo) > 0 && strlen(ManagerCard) > 0 && strlen(UserNo) > 0 && strlen(CountNo) > 0)
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

#if defined (ZHTAICON) || defined (ZHNCR236) || defined(ZHNCR236_2)
        rc = pthread_create(&interruptThread1, NULL, ZHI2cReaderFunction1, NULL);
        assert(rc == 0);
#endif

#if defined (ZHTSW303) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#endif

#ifdef ZHM3050
        SerialFunctionFlag = 1;
        rc = pthread_create(&serialThread, NULL, ZHSerialFunction, NULL);
        assert(rc == 0);
#else        
        WatchdogThreadFlag = 1; 
        rc = pthread_create(&watchDogThread, NULL, WatchdogFunction, NULL);
        assert(rc == 0);
#endif
        while(1)
        {
            char uploadFilePath[INPUTLENGTH];           
            if(!UploadFileFlag); //UploadFileFlag == 0;
            else if(LoopLeaveEventIndex == ZHPowerOffEvent)
            {
                printf("[%s]power off, so we don't sleep\n", __func__);
                gettimeofday(&now, NULL);
            }
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
            if(!UploadFileFlag || LoopLeaveEventIndex == ZHPowerOffEvent) //UploadFileFlag == 0;
            {
                LockMachineFunction();
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                UnlockMachineFunction();

                printf("ready to cancel thread\n");
#if defined (ZHTSW100) || defined (ZHM2200A)
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
#if defined (ZHTAICON) || defined (ZHNCR236) || defined (ZHM2200) || defined (ZHNCR236_2)
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
#endif
#if defined (ZHM800) || defined (ZHM3100)
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread1);
                pthread_join(interruptThread1, NULL);
        
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(interruptThread2);
                pthread_join(interruptThread2, NULL);
#endif
#if defined (ZHTSW303) || defined (ZHM2600) || defined (ZHM168T) || defined (ZHSPH3000)
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                SerialFunctionFlag = 0;
                pthread_join(serialThread, NULL);
#endif                
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
                    if(ExCount[GOODCOUNT] - ExCount[GOODCOUNT-1] >= (atol(CountNo)/ GOODRATE))
                    {
                        WriteFile(MachSTOPForce1);
                        WriteFile(MachSTANDBY);    
                    }
#else
                    if(ExCount[GOODCOUNT] >= (atol(CountNo)/ GOODRATE))
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
                }else if(LoopLeaveEventIndex == ZHTimeoutExitEvent || LoopLeaveEventIndex == ZHChangeUserExitEvent)
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
                    nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITSHORT}}, NULL);
                    pthread_mutex_lock(&MutexScreen);
                    DisableUpDown = 1;
                    ScreenIndex = 0;
                    UpdateScreenFunction(0); 
                    pthread_mutex_unlock(&MutexScreen);

                    InputDone = 0;
                    BarcodeIndex = USERNO;
                    while(InputDone == 0 && LoopLeaveEventIndex != ZHPowerOffEvent)
                    {
                        LockMachineFunction();
                        nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                        UnlockMachineFunction();
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
                        pthread_mutex_lock(&MutexScreen);
                        DisableUpDown = 1;
                        ScreenIndex = 0;
                        UpdateScreenFunction(0);    
                        pthread_mutex_unlock(&MutexScreen);

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
                                CanChangeRepairModeFlag = 1; 
                                break;
                            }else
                            {
                                pthread_mutex_lock(&MutexFile);
                                WriteFile(MachREPAIRING2);
                                pthread_mutex_unlock(&MutexFile);
                                pthread_mutex_lock(&MutexScreen);
                                DisableUpDown = 1;
                                ScreenIndex = 0;
                                UpdateScreenFunction(0);    
                                pthread_mutex_unlock(&MutexScreen);
                            }
                        }
                        strcpy(UserNo, tempUserNo); 
                        isInPairMode = 0;

                    }else
                    {
                        printf("[%s|%d]%s %s %s %s %s %s\n", __func__, __LINE__,ISNo, ManagerCard, CountNo, UserNo, MachineNo, UploadFilePath);
                        BarcodeIndex = SCANDONE;
                    }
                    if(LoopLeaveEventIndex != ZHPowerOffEvent)
                    {
                        pthread_mutex_lock(&MutexFile);
                        WriteFile(MachUNLOCK);
                        pthread_mutex_unlock(&MutexFile);
                        WaitMainFunctionScanBarcodeDone = 0;
                        pthread_mutex_lock(&MutexScreen);
                        DisableUpDown = 1;
                        ScreenIndex = 0;
                        UpdateScreenFunction(0);
                        pthread_mutex_unlock(&MutexScreen);
                    }
                }else;
            }
            if(LoopLeaveEventIndex == ZHPowerOffEvent)
            {
                WriteFile(MachPOWEROFF);
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
                    }   
                }else 
                {
                    printf("unlink\n");
                    unlink(uploadFilePath);
                }
            }
            if(LoopLeaveEventIndex == ZHPowerOffEvent)
            {
#ifndef LOCALTEST
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_cancel(poweroffThread);
                pthread_join(poweroffThread, NULL);
#endif

#ifdef REFRESHSCREEN
                nanosleep((const struct timespec[]){{0, NANOSLEEPTIMEUNITMID}}, NULL);
                pthread_mutex_lock(&MutexLcdRefresh);
                LcdRefreshFlag = 0;
                pthread_cond_signal(&CondLcdRefresh);
                pthread_mutex_unlock(&MutexLcdRefresh);
                pthread_join(lcdRefreshThread, NULL);
#endif

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
                pthread_mutex_lock(&MutexScreen);
                ScreenIndex = 2;
                UpdateScreenFunction(2);
                pthread_mutex_unlock(&MutexScreen);

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

#if defined (ZHTAICON) || defined (ZHNCR236) || defined (ZHM2200) || defined (ZHNCR236_2) 
                memset(I2CEXValue, 0, sizeof(int)*6);
#endif

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
                isFirstinLoop = 1;                
                digitalWrite (ZHPIN31, LOW);
                pthread_mutex_lock(&MutexScreen);
                if(ScreenIndex == 0)
                {
                    UpdateScreenFunction(0);
                }else if(ScreenIndex == 1)
                {
                    UpdateScreenFunction(1);
                }else;
                pthread_mutex_unlock(&MutexScreen);
            }
            else;
            if(cancelThreadDone == 1)
            {
                printf("loop back cancelThreadDone\n");
                break;
            }else;
        }
    }
    return 0;
}

