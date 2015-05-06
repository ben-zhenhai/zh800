#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <assert.h>

#include <wiringPi.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <errno.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <net/if.h>

#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr

#define SHMSZ 2000
#define ZHNetworkType "eth0"

#define I2C_IO_Extend_1 0x20
#define I2C_IO_Extend_2 0x21
#define I2C_IO_Extend_3 0x22

#define WiringPiPIN_11 0
#define WiringPiPIN_12 1
#define WiringPiPIN_13 2
#define WiringPiPIN_15 3
#define WiringPiPIN_16 4
#define WiringPiPIN_18 5
#define WiringPiPIN_21 13
#define WiringPiPIN_22 6

#define IN_P0 0x00
#define IN_P1 0x01
#define OUT_P0 0x02
#define OUT_P1 0x03
#define INV_P0 0x04
#define INV_P1 0x05
#define CONFIG_P0 0x06
#define CONFIG_P1 0x07

#define WatchDogCountValue 600
#define InputLength 256
#define UPLoadFileLength 256
#define CountPeriod 4
#define FTPCountValue 300
#define FTPWakeUpValue 60
#define BLANKTIMER 500000
#define zhMAXOUTPUT 10

#define goodrate 1
//#define LogMode
#define PrintInfo
#define PrintMode

//#define Log(str, part, file, func, line, opt) strcat(str, part);strcat(str, file);strcat(str, func);strcat(str, opt);printf("%d %s\n",line,str);
//#define Log(func, line, opt) printf("%s, %s, %d", func, opt, line);
//#define Log(s,func, line, opt) strcat(s,func);strcat(s,opt);s=s+strlen(func)+strlen(opt);if(s>=tail)s=shm+1
#define Log(s,func, line, opt) StringCat(func);StringCat(opt)

enum{
    MachRUNNING = 1,
    MachREPAIRING,
    MachREPAIRDone,
    MachJobDone,
    MachLOCK,
    MachUNLOCK,
    MachSTOPForce1,
    MachSTOPForce2
};

int WatchDogThreadFlag;
int zhResetFlag = 0;
int zhInterruptEnable1 = 0;
int zhInterruptEnable2 = 0;
int zhInterruptEnable3 = 0;
int PrintLeftLog = 0;
short zhTelnetFlag = 0;
short WatchDogFlag = 0;
short LightControlFlag = 0;
short FTPFlag = 0;
short ButtonFlag = 0;
short MasterFlag = 0;
short WaitBarcodeInput = 0;
short ReadytoRead = 0;

char *shm, *s, *tail;
char *shm_pop;

pthread_cond_t cond, cond1, condFTP;
pthread_mutex_t mutex, mutex_2, mutex_3, mutex_log, mutexFTP, mutexFile, mutexInput;

long PINCount[6][8];
long PINEXCount[5][8];
int I2CEXValue[6];
short CutRoll[2];

char ISNo[InputLength], ManagerCard[InputLength], MachineCode[InputLength], UserNo[InputLength], CountNo[InputLength];
char UPLoadFile[UPLoadFileLength];
char tempString[InputLength];

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
void * zhLogFunction(void *argument);
void * FTPFunction(void *arguemnt);
void * ButtonListenFunction(void *argument);
void * InputFunction(void *argument);
void StringCat(const char *str);

void * zhINTERRUPT1(void *argument);
void * zhINTERRUPT2(void *argument);
void * zhINTERRUPT3(void *argument);

void * LightControl(void *argument);
void * RemoteControl(void *argument);

void StringCat(const char *str)
{
    int x = strlen(str);
    int count;
    pthread_mutex_lock(&mutex_log);
    for(count = 0; count < x; count++)
    {
        *s = *str;
        //printf("%c",*s);
        s++;
        str++;
        if(s >= tail) {
            s = shm +1;
        }
    }
    pthread_mutex_unlock(&mutex_log);
}

void * RemoteControl(void *argument)
{
    int socket_desc , new_socket , c;
    struct sockaddr_in server , client;
    int iSetOption = 1;
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
    //Bind
    //int x =  bind(socket_desc,(struct sockaddr *)&server , sizeof(server));
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)))
    //if( x < 0)
    {
        printf("%d ", errno);
        puts("bind failed");
        pthread_exit(NULL);
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while( zhTelnetFlag == 1 && (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted\n");
        zhTelnetFlag = 0;
        break;
    }
    printf("Telnet close\n"); 
    /*if(close(socket_desc) < 0)
    {
        printf("erron %d",errno);
    }*/
    close(socket_desc);
    //free(socket_desc);
    sleep(1);
}

void * zhLogFunction(void *argument)
{
    int shmid;
    long size;
    key_t key;
    char *s_pop;
    struct shmid_ds shmid_ds;
    struct timeval now;
    struct tm ts;
    struct stat st;
    char LogFileLocation[80];
    char LogString[300];
    int WriteFileCount = 0;
    char buff[40];

    gettimeofday(&now, NULL);
    ts = *localtime(&now.tv_sec);
    strftime(buff, sizeof(buff), "%Y/%m/%d_%H:%M:%S", &ts);
    
    key = 5678;
    if((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0)
    {
        perror("shmget");
        pthread_exit(NULL);
    }
    if((shm_pop = shmat(shmid, NULL, 0)) == (char *)-1)
    {
        perror("shmat");
        pthread_exit(NULL);
    }
    s_pop = shm_pop + 1;
    int count;

    for(count = 0 ; count < SHMSZ -1 ; count ++)
    {
        *s_pop = NULL;
        s_pop++;
    }
    s_pop = shm_pop + 1;
    //char *tail1 = shm_pop + (SHMSZ -1);
    tail = shm_pop + (SHMSZ -1);
    memset(LogString, 0, sizeof(char)*300);
    sprintf(LogFileLocation,"/media/usb0/Log_%ld.txt",(long)now.tv_sec);
    FILE *pfile;
    pfile = fopen(LogFileLocation, "a");
    fprintf(pfile,"NCR236\t%s\t0\t", buff);
    fclose(pfile);
    //printf("%s\n",LogFileLocation);
    while(*shm_pop != '*')
    {
        if(*s_pop == NULL)
        {
            //usleep(5000000);
            usleep(500000);
            continue;
        }
        
        if(WriteFileCount == 299 || PrintLeftLog == 1)
        {
            pfile = fopen(LogFileLocation,"a");
            
            int ForCount;
            for(ForCount = 0; ForCount <= WriteFileCount; ForCount++)
            {
                fprintf(pfile, "%c", LogString[ForCount]);
                if(LogString[ForCount] == '\n')
                {
                    gettimeofday(&now, NULL);
                    ts = *localtime(&now.tv_sec);
                    strftime(buff, sizeof(buff), "%Y/%m/%d_%H:%M:%S", &ts);
                    fprintf(pfile,"NCR236\t%s\t%ld\t",buff,PINCount[0][3]);
                }
            }      
    
            fclose(pfile);
            
            memset(LogString, 0, sizeof(char)*300);
            WriteFileCount = 0;
        }
        //printf("%c",*s_pop);
        //pfile = fopen(LogFileLocation,"a");
        
        //fprintf(pfile,"%c",*s_pop);
        strncat(LogString, s_pop, 1);
        WriteFileCount++;
        //fclose(pfile);
        *s_pop = NULL;
        if(s_pop >= tail)
        {
            s_pop = shm_pop + 1;

        }
        else s_pop++;

        stat(LogFileLocation, &st);
        size = st.st_size;
        if(size > 100000)
        {
            gettimeofday(&now, NULL);
            sprintf(LogFileLocation,"/media/usb0/Log_%ld.txt",(long)now.tv_sec);
        }
    }
    shmdt(shm_pop);
    shmctl(shmid, IPC_RMID, &shmid_ds);
}
void * LightControl(void *argument)
{
    struct timeval now;
    struct timespec outtime;

    char blankFlag = 0x00;

    while(LightControlFlag)
    {
        if(blankFlag != 0x00)
        {
            digitalWrite (WiringPiPIN_21, HIGH);
            blankFlag = 0x00;
        }else
        {
            digitalWrite (WiringPiPIN_21, LOW);
            blankFlag = 0x01;
        }
        pthread_mutex_lock(&mutex_2);

        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec;
        outtime.tv_nsec = (now.tv_usec + BLANKTIMER) * 1000; 

        if( outtime.tv_nsec > 1000000000)
        {
            outtime.tv_sec = outtime.tv_sec + 1;
            outtime.tv_nsec = (outtime.tv_nsec % 1000000000);
        }

        pthread_cond_timedwait(&cond1, &mutex_2, &outtime);
        pthread_mutex_unlock(&mutex_2);
        
    }
    digitalWrite (WiringPiPIN_21, HIGH);
}

void * WatchDogForGood(void *argument)
{
#ifdef LogMode
    Log(s, __func__, __LINE__, " entry\n");
#endif
    struct timeval now;
    struct timespec outtime;
    struct ifreq ifr;
    int WatchDogCoolDown = WatchDogCountValue;

    int fd;
    long size; 
    int ForCount, ForCount2;
    FILE * pfile;

    while(WatchDogThreadFlag)
    {
        pthread_mutex_lock(&mutex);

        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + CountPeriod;
        outtime.tv_nsec = now.tv_usec * 1000; 

        pthread_cond_timedwait(&cond, &mutex, &outtime);
        pthread_mutex_unlock(&mutex);

#ifdef LogMode
    Log(s, __func__, __LINE__, " WatchDog wake\n");
#endif
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name, ZHNetworkType, IFNAMSIZ-1);
        ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);
        gettimeofday(&now, NULL);

        if (WatchDogFlag == 1) 
        {
            WatchDogCoolDown = WatchDogCountValue;
	        //pthread_mutex_lock(&mutex_3);
            WatchDogFlag = 0;
	        //pthread_mutex_unlock(&mutex_3);
        }
        else 
        {
            WatchDogCoolDown = WatchDogCoolDown - CountPeriod;
            printf("%d \n",WatchDogCoolDown);
        }
        if (WatchDogCoolDown <= 0)
        {
            zhResetFlag = 1;
        }
        pthread_mutex_lock(&mutexFile);        
        
        pfile = fopen(UPLoadFile, "a");

        for(ForCount = 0; ForCount < 2; ++ForCount)
        {
            for(ForCount2 = 0; ForCount2 < 8; ++ForCount2)
            {
                if((PINCount[ForCount][ForCount2] - PINEXCount[ForCount][ForCount2]) > zhMAXOUTPUT)
                {
                     fprintf(pfile, "%s %s %s -1 %ld 0 %s %d %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, 
                                                                               (long)now.tv_sec,
                                                                               inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                               ForCount * 8 + ForCount2 + 2, MachineCode, UserNo, MachRUNNING);
                }
                else if(PINEXCount[ForCount][ForCount2] != PINCount[ForCount][ForCount2] && ForCount2 == 3 && ForCount == 0)
                {
#ifdef PrintMode
                    fprintf(pfile, "%s %s %s %ld %ld 0 %s %d %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, 
                                                                                  PINCount[0][3] - PINEXCount[0][3], 
                                                                                  (long)now.tv_sec,
                                                                                  inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                  ForCount * 8 + ForCount2 + 2, MachineCode, UserNo, MachRUNNING);
#else
                    fprintf(pfile, "%s %s %s %ld %ld 0 %s %d %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                                 inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                 ForCount * 8 + ForCount2 + 2, MachineCode, UserNo, MachRUNNING);
#endif
                }
                else if(PINEXCount[ForCount][ForCount2] != PINCount[ForCount][ForCount2])
                {    
#ifdef PrintMode
                    fprintf(pfile, "%s %s %s 0 %ld %ld %s %d %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo,
                                                                                  (long)now.tv_sec,  
                                                                                  PINCount[ForCount][ForCount2] - PINEXCount[ForCount][ForCount2],
                                                                                  inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                  ForCount * 8 + ForCount2 + 2, MachineCode, UserNo, 
                                                                                  MachRUNNING);
#else
                    fprintf(pfile, "%s %s %s %ld %ld %ld %s %d %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], 
                                                                                    (long)now.tv_sec,  PINCount[ForCount][ForCount2],
                                                                                    inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                    ForCount * 8 + ForCount2 + 2, MachineCode, UserNo, 
                                                                                    MachRUNNING);
#endif
                }else;
            }
        }
        size = ftell(pfile);        
        fclose(pfile);
        pthread_mutex_unlock(&mutexFile);        

        printf("%s %s %s %s %s %ld|| %ld %ld %ld %ld %ld %ld %ld %ld || %ld %ld %ld %ld %ld %ld %ld %ld \n",
                ISNo, ManagerCard, MachineCode, UserNo, CountNo, size, 
                PINCount[0][0], PINCount[0][1], PINCount[0][2], PINCount[0][3], PINCount[0][4], PINCount[0][5], PINCount[0][6], PINCount[0][7],
                PINCount[1][0], PINCount[1][1], PINCount[1][2], PINCount[1][3], PINCount[1][4], PINCount[1][5], PINCount[1][6], PINCount[1][7]);
        
        memcpy(PINEXCount, PINCount, sizeof(long)*40);

        //check network status
        int fd2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct ifreq ethreq;
        memset(&ethreq, 0, sizeof(ethreq));
        strncpy(ethreq.ifr_name, ZHNetworkType, IFNAMSIZ);
        ioctl(fd2, SIOCGIFFLAGS, &ethreq);
        if(ethreq.ifr_flags & IFF_RUNNING)
        {
            digitalWrite (WiringPiPIN_15, HIGH);
            digitalWrite (WiringPiPIN_16, HIGH);
            digitalWrite (WiringPiPIN_18, LOW);
        }else
        {
            digitalWrite (WiringPiPIN_15, HIGH);
            digitalWrite (WiringPiPIN_16, LOW);
            digitalWrite (WiringPiPIN_18, LOW);
        }
        close(fd2);
       
        //[vers| a local log for match with server]
        /*pfile = fopen("DataLog", "w");
        for(ForCount = 0; ForCount < 5; ForCount++)
        {
            for(ForCount2 = 0; ForCount2 < 8; ForCount2++)
            {
                fprintf(pfile, "%d %ld\n",(ForCount*8+ForCount2+2), PINEXCount[ForCount][ForCount2]);
            }
        }
        fclose(pfile);*/
        //[vers| end]
#ifdef LogMode
        Log(s, __func__, __LINE__, " WatchDog sleep\n");
#endif
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, " exit\n");
#endif
}
void * zhINTERRUPT1(void * argument)
{
    while(zhInterruptEnable1)
    {
        //pthread_mutex_lock(&mutex_3);
        //Log(s, __func__, __LINE__, " entry\n");
        char *dev = "/dev/i2c-1";
        int fd , r;
        int x, y;
        int ForCount ,first, second;
    
        fd = open(dev, O_RDWR);
        if(fd < 0)
        {
            perror("Open fail");
            exit(0);
        }

        r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_1);
        if (r < 0)
        {
            perror("Selecting i2c device\n");
            exit(0);
        }
    
        x = i2c_smbus_read_byte_data(fd, IN_P0);
        y = i2c_smbus_read_byte_data(fd, IN_P1);
        close(fd);
        //pthread_mutex_unlock(&mutex_3);
        if(I2CEXValue[0] != x || I2CEXValue[1] != y)
        {
            first = I2CEXValue[0] ^ 0xff;
            first = first & x;
            second = I2CEXValue[1] ^ 0xff;
            second = second & y;

            I2CEXValue[0] = x;
            I2CEXValue[1] = y;

            for(ForCount = 0; ForCount < 8; ++ForCount)
            {
                //PINCount[0][ForCount] = PINCount[0][ForCount] + (first & 1);
                if(ForCount == 4 && (first &1) == 1)
                {
                    if((x & 32) == 0)
                    {
                        PINCount[0][5] = PINCount[0][5] + 1;   
                    }
                    if((x & 8) == 8)
                    {
                        PINCount[0][3] = PINCount[0][3] + 1;
                    }
                }
                first = first >> 1;
                //PINCount[1][ForCount] = PINCount[1][ForCount] + (second & 1);
                second = second >> 1; 
            }
#ifdef PrintInfo 
            printf("reader 1: %3d, %3d | %ld %ld %ld %ld %ld %ld %ld %ld || %ld %ld %ld %ld %ld %ld %ld %ld \n",
                                x , y, PINCount[0][0], PINCount[0][1], PINCount[0][2], 
                                PINCount[0][3], PINCount[0][4], PINCount[0][5], PINCount[0][6], PINCount[0][7],
                                PINCount[1][0], PINCount[1][1], PINCount[1][2], PINCount[1][3], PINCount[1][4], 
                                PINCount[1][5], PINCount[1][6], PINCount[1][7]);
#endif
	        //pthread_mutex_lock(&mutex_3);
            WatchDogFlag = 1;
	        //pthread_mutex_unlock(&mutex_3);
#ifdef LogMode        
            Log(s, __func__, __LINE__, " exit\n");
#endif
        }
    }
}

void * zhINTERRUPT2(void * argument)
{
    while(zhInterruptEnable2)
    {   
        //pthread_mutex_lock(&mutex_3);
        //Log(s, __func__, __LINE__, " entry\n");
        char *dev = "/dev/i2c-1";
        int fd, r, x, y;
        int ForCount ,first, second;
        
        fd = open(dev, O_RDWR);

        if(fd < 0)
        {
            perror("Open fail");
            printf("fd fail\n");
        }

        r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_2);
        if(r < 0)
        {
            perror("Selecting i2c device\n");
            printf("ioctl fail\n");
        }

        x = i2c_smbus_read_byte_data(fd, IN_P0);
        y = i2c_smbus_read_byte_data(fd, IN_P1);
	    close(fd);
    	//pthread_mutex_unlock(&mutex_3);
       
        if(I2CEXValue[2] != x || I2CEXValue[3] != y)
        { 
            first = I2CEXValue[2] ^ 0xff;
            first = first & x;
            second = I2CEXValue[3] ^ 0xff;
            second = second & y;

            I2CEXValue[2] = x;
            I2CEXValue[3] = y;

            for(ForCount = 0; ForCount < 8; ++ForCount)
            {
                PINCount[2][ForCount] = PINCount[2][ForCount] + (first & 1);
                first = first >> 1;
                PINCount[3][ForCount] = PINCount[3][ForCount] + (second & 1);
                second = second >> 1; 
            }
#ifdef PrintInfo
            printf("reader 2: %3d, %3d | %ld %ld %ld %ld %ld %ld %ld %ld || %ld %ld %ld %ld %ld %ld %ld %ld \n",
                                x , y, PINCount[2][0], PINCount[2][1], PINCount[2][2], 
                                PINCount[2][3], PINCount[2][4], PINCount[2][5], PINCount[2][6], PINCount[2][7],
                                PINCount[3][0], PINCount[3][1], PINCount[3][2], PINCount[3][3], PINCount[3][4], PINCount[3][5], PINCount[3][6], PINCount[3][7]);
#endif
            WatchDogFlag = 1;
#ifdef LogMode
            Log(s, __func__, __LINE__, " exit\n");
#endif
        }
    }
}

void * zhINTERRUPT3(void * argument)
{
    while(zhInterruptEnable3)
    {
        //pthread_mutex_lock(&mutex_3);
        int fd, r, x;
        char *dev = "/dev/i2c-1";
        int ForCount, first;
        fd = open(dev, O_RDWR);
      
        if(fd < 0)
        {
            perror("Open fail");
        }
        r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_3);
        if(r < 0)
        {
            perror("Selecting i2c device");
        }
        x = i2c_smbus_read_byte_data(fd, IN_P0);
        close(fd);
        //pthread_mutex_unlock(&mutex_3);
        if(I2CEXValue[4] != x)
        {
            first = I2CEXValue[4] ^ 0xff;
            first = first & x;
        
            I2CEXValue[4] = x;
       
            for(ForCount = 0; ForCount < 8; ForCount++)
            {
                PINCount[4][ForCount] = PINCount[4][ForCount] + (first & 1);
                first = first >> 1;
            }
#ifdef PrintInfo
            printf("reader 3: %3d,     | %ld %ld %ld %ld %ld %ld %ld %ld \n",x ,PINCount[4][0], PINCount[4][1], PINCount[4][2], 
                                    PINCount[4][3], PINCount[4][4], PINCount[4][5], PINCount[4][6], PINCount[4][7]);
#endif 
            WatchDogFlag = 1;
        }
    }
}

void * InputFunction(void * argument)
{
    char getStringinBuffer[InputLength];
    while(1)
    {
        memset(getStringinBuffer, 0, sizeof(char));
        gets(getStringinBuffer);
        if(WaitBarcodeInput && ReadytoRead == 0)
        {
            pthread_mutex_lock(&mutexInput);
            memset(tempString, 0, sizeof(char));
            strcpy(tempString, getStringinBuffer);
            ReadytoRead = 1;
            pthread_mutex_unlock(&mutexInput); 
        }
        usleep(100000);
    }
}

int main(int argc ,char *argv[])
{
    char *dev = "/dev/i2c-1";
    int rc;    
    pthread_t WatchDogThread, LogThread, LightControlThread, TelnetControlThread, InterruptThread1, InterruptThread2, InterruptThread3;
    pthread_t FTPThread, InputThread;
    int shmid;
    key_t key;
    int fd, r;
    long goodCount;
    char *tempPtr;
    struct timeval now;
    FILE *pfile;
    struct ifreq ifr;

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_2, NULL);
    pthread_mutex_init(&mutex_3, NULL);
    pthread_mutex_init(&mutex_log, NULL);
    pthread_mutex_init(&mutexFTP, NULL);
    pthread_mutex_init(&mutexFile, NULL);
    pthread_mutex_init(&mutexInput, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond1, NULL);
    pthread_cond_init(&condFTP, NULL);
 
    wiringPiSetup(); 
  
    //pinMode(WiringPiPIN_11, INPUT);
    //pinMode(WiringPiPIN_12, INPUT);
    //pinMode(WiringPiPIN_13, INPUT);
    pinMode(WiringPiPIN_22, INPUT);
 
    //pullUpDnControl (WiringPiPIN_11, PUD_UP); 
    //pullUpDnControl (WiringPiPIN_12, PUD_UP); 
    //pullUpDnControl (WiringPiPIN_13, PUD_UP); 
    pullUpDnControl (WiringPiPIN_22, PUD_UP); 
  
    pinMode(WiringPiPIN_15, OUTPUT); 
    pinMode(WiringPiPIN_16, OUTPUT); 
    pinMode(WiringPiPIN_18, OUTPUT);
    pinMode(WiringPiPIN_21, OUTPUT);
   
    /*scanner check
     * 1. ISNO
     * 2. manager card
     * 3. Count
     * 4. machine No
     * 5. user No
     */
 
#ifdef LogMode

    rc = pthread_create(&LogThread, NULL, zhLogFunction, NULL);
    assert(rc == 0);
    key = 5678;
    sleep(1);

    if ((shmid = shmget(key, SHMSZ, 0666)) < 0)
    {
        perror("shmget main");
        return 1;
    }

    if ((shm = shmat(shmid, NULL, 0)) == (char *)-1)
    {
        perror("shmat");
        return 1;
    }
    s = shm + 1;
#endif

    //tail = shm + (SHMSZ - 1);

#ifdef LogMode
    Log(s, __func__, __LINE__, " ready to init\n");
#endif    

    rc = pthread_create(&InputThread, NULL, InputFunction, NULL);
    assert(rc == 0);

    //the mechine always standby
    while(1)
    {
        unsigned char isNormalStop = 0;
        MasterFlag = 1;
#ifdef LogMode
        Log(s, __func__, __LINE__, " scan barcode ready\n");
#endif

        //lock
        fd = open(dev, O_RDWR);
        if(fd < 0)
        {
            perror("Open Fail");
            return 1;
        }
        r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_3);
        if(r < 0)
        {
            perror("Selection i2c device fail");
            return 1;
        }
        
        i2c_smbus_write_byte_data(fd, OUT_P1, 0x07);
        i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
        close(fd);

        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, HIGH);
        //3rd i3c board will control 3*8 control

#ifdef  PrintInfo 
        printf("Ready to work...\n");
#endif
        WaitBarcodeInput = 1;
        while(1)
        {
            sleep(1);
            if(ReadytoRead)
            {
                pthread_mutex_lock(&mutexInput);
                ReadytoRead = 0;
                if(strlen(tempString) == 14)
                {
                    memset(ISNo, 0, sizeof(char)*InputLength);
                    //tempPtr = tempString + 3;
                    //memcpy(ISNo, tempPtr, sizeof(tempString)-2);
                    tempPtr = tempString;
                    memcpy(ISNo, tempPtr, sizeof(tempString));
                    digitalWrite (WiringPiPIN_15, LOW);
                    digitalWrite (WiringPiPIN_16, HIGH);
                    digitalWrite (WiringPiPIN_18, HIGH);
                    pthread_mutex_unlock(&mutexInput);
                    break;
                }
                pthread_mutex_unlock(&mutexInput);
                printf("scan ISNo error code\n");
            }
        }
        while(1)
        {
            sleep(1);
            if(ReadytoRead)
            {
                pthread_mutex_lock(&mutexInput);
                ReadytoRead = 0;
                if(strlen(tempString) == 24)
                {
                    memset(ManagerCard, 0, sizeof(char)*InputLength);
                    //tempPtr = tempString + 3;
                    //memcpy(ManagerCard, tempPtr, sizeof(tempString)-2);
                    tempPtr = tempString;
                    memcpy(ManagerCard, tempPtr, sizeof(tempString));
                    digitalWrite (WiringPiPIN_15, HIGH);
                    digitalWrite (WiringPiPIN_16, LOW);
                    digitalWrite (WiringPiPIN_18, HIGH);
                    pthread_mutex_unlock(&mutexInput);
                    break;
                }
                pthread_mutex_unlock(&mutexInput);
                printf("ManagerCard scan error code\n");
            }
        }
        while(1)
        {
            sleep(1);
            if(ReadytoRead)
            {
                int stringLength = strlen(tempString);
                int arrayCount = 0;
                short flagFailPass = 0;
                pthread_mutex_lock(&mutexInput);
                ReadytoRead = 0;
                while(arrayCount < stringLength)
                {
                    if(tempString[arrayCount] == '0') ;
                    else if(tempString[arrayCount] == '1');
                    else if(tempString[arrayCount] == '2');
                    else if(tempString[arrayCount] == '3');
                    else if(tempString[arrayCount] == '4');
                    else if(tempString[arrayCount] == '5');
                    else if(tempString[arrayCount] == '6');
                    else if(tempString[arrayCount] == '7');
                    else if(tempString[arrayCount] == '8');
                    else if(tempString[arrayCount] == '9');
                    else 
                    {
                        flagFailPass = 1;
                        break;
                    }
                    ++arrayCount;
                }
                if(flagFailPass == 0 && stringLength > 0)
                {
                    memset(CountNo, 0, sizeof(char)*InputLength);
                    memcpy(CountNo, tempPtr, sizeof(tempString));
                    goodCount = (atoi(CountNo)*goodrate);
                    if(goodCount > 0)
                    {
                        printf("need finish: %ld\n", goodCount);
                        digitalWrite (WiringPiPIN_15, LOW);
                        digitalWrite (WiringPiPIN_16, LOW);
                        digitalWrite (WiringPiPIN_18, HIGH);
                        pthread_mutex_unlock(&mutexInput);
                        break;
                    }
                }
                /*if(strncmp(tempString, "WWW", 3) == 0)
                {
                    memset(CountNo, 0, sizeof(char)*InputLength);
                    tempPtr = tempString + 3;
                    memcpy(CountNo, tempPtr, sizeof(tempString)-2);
                    goodCount = (atoi(CountNo)*goodrate);
                    printf("need finish: %ld\n", goodCount);
                    digitalWrite (WiringPiPIN_15, LOW);
                    digitalWrite (WiringPiPIN_16, LOW);
                    digitalWrite (WiringPiPIN_18, HIGH);
                
                    break;
                }*/
                pthread_mutex_unlock(&mutexInput);
                printf("CountNo scan error code\n");
            }
        } 
     
        while(1)
        {
            sleep(1);
            if(ReadytoRead)
            {
                pthread_mutex_lock(&mutexInput);
                ReadytoRead = 0;
                if(strncmp(tempString, "XXXP", 4) == 0)
                {
                    memset(UserNo, 0, sizeof(char)*InputLength);
                    tempPtr = tempString + 4;
                    memcpy(UserNo, tempPtr, sizeof(tempString)-3);
                
                    digitalWrite (WiringPiPIN_15, HIGH);
                    digitalWrite (WiringPiPIN_16, HIGH);
                    digitalWrite (WiringPiPIN_18, LOW);
                    pthread_mutex_unlock(&mutexInput);
                    break;
                }
                pthread_mutex_unlock(&mutexInput);
                printf("UserNo scan error code\n");
            }
        }
        WaitBarcodeInput = 0;

        char FakeInput[5][InputLength];
        memset(FakeInput, 0, sizeof(char)*(5*InputLength));
        int filesize, FakeInputNumber = 0;
        int FakeInputNumber_2 = 0;
        char * buffer, * charPosition;
        short FlagNo = 0;        

        pfile = fopen("/home/pi/works/NCR236/barcode","r");
        fseek(pfile, 0, SEEK_END);
        filesize = ftell(pfile);
        rewind(pfile);
        buffer = (char *) malloc (sizeof(char)*filesize);
        charPosition = buffer;
        fread(buffer, 1, filesize, pfile);
        fclose(pfile);
       
        while(filesize > 1)
        {
            if(*charPosition == ' ')
            {
                FlagNo = 1;
            }
            else if(*charPosition != ' ' && FlagNo == 1)
            {
                FakeInputNumber++;
                FakeInputNumber_2 = 0;

                FakeInput[FakeInputNumber][FakeInputNumber_2] = *charPosition;
                FakeInputNumber_2++;
                FlagNo = 0;
            }
            else
            {
                FakeInput[FakeInputNumber][FakeInputNumber_2] = *charPosition;
                FakeInputNumber_2++;
            }
            filesize--;
            charPosition++;
        }
        free(buffer);
        
        sleep(1);
        memset(MachineCode, 0 , sizeof(char)*InputLength);
        strcpy(MachineCode, FakeInput[2]);

        memset(UPLoadFile, 0, sizeof(char)*UPLoadFileLength);

        gettimeofday(&now, NULL);
        sprintf(UPLoadFile,"%ld%s.txt",(long)now.tv_sec, MachineCode); 
        
        printf("%s %s %s %s %s %s\n", ISNo, ManagerCard, MachineCode, UserNo, CountNo, UPLoadFile);
 
        //reset count value and other;
        memset(PINCount, 0, sizeof(long)*48);
        memset(PINEXCount, 0, sizeof(long)*40);
        memset(I2CEXValue, 0, sizeof(int)*6);
        memset(CutRoll, 0, sizeof(short)*2);

        if(zhTelnetFlag == 0)
        {
            /*zhTelnetFlag = 1;
            rc = pthread_create(&TelnetControlThread, NULL, RemoteControl, NULL);
            assert(rc == 0);*/
        }

        while(MasterFlag)
        {
            //i2c init start
            sleep(1);
            fd = open(dev, O_RDWR);
            if(fd < 0)
            {
                perror("Open Fail");
                return 1;
            }
            r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_1);
            if(r < 0)
            {
                perror("Selecting i2c device");
                return 1;
            }
            i2c_smbus_write_byte_data(fd, OUT_P0, 0x00);
            i2c_smbus_write_byte_data(fd, INV_P0, 0x38);
            i2c_smbus_write_byte_data(fd, CONFIG_P0, 0x38);

            i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
            i2c_smbus_write_byte_data(fd, INV_P1, 0x00);
            i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00); 

            i2c_smbus_read_byte_data(fd, IN_P0);
            i2c_smbus_read_byte_data(fd, IN_P1);
            close(fd);
         
            fd = open(dev, O_RDWR);
        
            if(fd < 0)
            {
                perror("Open Fail");
                return 1;
            }
            r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_2);
            if(r < 0)
            {
                perror("Selection i2c device fail");
                return 1;
            }
            i2c_smbus_write_byte_data(fd, OUT_P0, 0x00);
            i2c_smbus_write_byte_data(fd, INV_P0, 0x00);
            i2c_smbus_write_byte_data(fd, CONFIG_P0, 0x00);

            i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
            i2c_smbus_write_byte_data(fd, INV_P1, 0x00);
            i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
            i2c_smbus_read_byte_data(fd, IN_P0);
            i2c_smbus_read_byte_data(fd, IN_P1);
            close(fd);

            fd = open(dev, O_RDWR);
            if(fd < 0)
            {
                perror("Open Fail");
                return 1;
            }
            r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_3);
            if(r < 0)
            {
                perror("Selection i2c device fail");
                return 1;
            }
            i2c_smbus_write_byte_data(fd, OUT_P0, 0x00);
            i2c_smbus_write_byte_data(fd, INV_P0, 0x00);
            i2c_smbus_write_byte_data(fd, CONFIG_P0, 0x00);
        
            i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
            i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
            i2c_smbus_read_byte_data(fd, IN_P0);
            i2c_smbus_read_byte_data(fd, IN_P1);
            close(fd);
            //i2c init finish

            PrintLeftLog = 0;  
            zhInterruptEnable1 = 1;
            rc = pthread_create(&InterruptThread1, NULL, zhINTERRUPT1, NULL);
            assert(rc == 0);
            /*
            zhInterruptEnable2 = 1;
            rc = pthread_create(&InterruptThread2, NULL, zhINTERRUPT2, NULL);
	        assert(rc == 0);
            zhInterruptEnable3 = 1;
            rc = pthread_create(&InterruptThread3, NULL, zhINTERRUPT3, NULL);
            assert(rc == 0);
            */

            zhResetFlag = 0; //reset flag clean

            WatchDogThreadFlag = 1;
            rc = pthread_create(&WatchDogThread, NULL, WatchDogForGood, NULL);
            assert(rc == 0);

            FTPFlag = 1;
            rc = pthread_create(&FTPThread, NULL, FTPFunction, NULL);
            assert(rc == 0);

            //LightControlFlag = 1;
            //rc = pthread_create(&LightControlThread, NULL, LightControl, NULL);
            //assert(rc == 0);

            while(zhResetFlag == 0)
            {
                usleep(100000);
                if(PINCount[0][3] >= goodCount)
                //if(PINCount[0][3] >= 0)
                {
                    //finish job
                    printf("Houston we are ready to back!\n");
#ifdef LogMode
                    Log(s, __func__, __LINE__, " fininsh job\n");
                    Log(s, __func__, __LINE__, " fininsh job\n");
                    Log(s, __func__, __LINE__, " fininsh job\n");
                    Log(s, __func__, __LINE__, " fininsh job\n");
                    Log(s, __func__, __LINE__, " fininsh job\n");
                    Log(s, __func__, __LINE__, " fininsh job\n");
#endif
                    zhResetFlag = 1;
                    MasterFlag = 0;
                    isNormalStop = 1;
                    sleep(1);
                }
                else if(digitalRead(WiringPiPIN_22) == 0)
                {
                    //finish job
                    printf("Houston PIN_22 call me back to base!\n");
#ifdef LogMode
                    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
                    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
                    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
                    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
                    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
                    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
#endif
                    zhResetFlag = 1;
                    MasterFlag = 0;
                    sleep(1);
                }
                /*else if(zhTelnetFlag == 0)
                {
                    printf("Houston remote user call me back to base!\n");
#ifdef LogMode
                    Log(s, __func__, __LINE__, " remote user call me back\n");
                    Log(s, __func__, __LINE__, " remote user call me back\n");
                    Log(s, __func__, __LINE__, " remote user me back\n");
#endif
                    pthread_join(TelnetControlThread, NULL);
                    zhResetFlag = 1;
                    MasterFlag = 0;
                }*/
                else;
            }

            zhInterruptEnable1 = 0;
            pthread_join(InterruptThread1, NULL);
            sleep(1);
            /*
            zhInterruptEnable2 = 0;
            pthread_join(InterruptThread2, NULL);
            sleep(1);
            zhInterruptEnable3 = 0;
            pthread_join(InterruptThread3, NULL);
            sleep(1);
            */

            WatchDogThreadFlag = 0;
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            pthread_join(WatchDogThread, NULL);
            sleep(1);

            //get ip address
            fd = socket(AF_INET, SOCK_DGRAM, 0);
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, ZHNetworkType, IFNAMSIZ-1);
            ioctl(fd, SIOCGIFADDR, &ifr);
            close(fd);
            gettimeofday(&now, NULL);

            if(MasterFlag == 0 && isNormalStop == 1)
            {
                pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
                fprintf(pfile, "%s %s %s 0 %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachJobDone);
#else
                fprintf(pfile, "%s %s %s %ld %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachJobDone);
#endif
                fclose(pfile);
            }else if(MasterFlag == 0)
            {
                if(PINCount[0][3] >= (goodCount / 1.04))
                {
                    pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
                    fprintf(pfile, "%s %s %s 0 %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachSTOPForce1);
#else
                    fprintf(pfile, "%s %s %s %ld %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachSTOPForce1);
#endif
                    fclose(pfile);
                }
                else
                {
                    pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
                    fprintf(pfile, "%s %s %s 0 %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachSTOPForce2);
#else
                    fprintf(pfile, "%s %s %s %ld %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachSTOPForce2);
#endif
                    fclose(pfile);
                }
            }else
            {
               pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
               fprintf(pfile, "%s %s %s 0 %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                            inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                            MachineCode, UserNo, MachLOCK);
#else
               fprintf(pfile, "%s %s %s %ld %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                            inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                            MachineCode, UserNo, MachLOCK);

#endif
               fclose(pfile);
            }

            FTPFlag = 0;
            pthread_mutex_lock(&mutexFTP);
            pthread_cond_signal(&condFTP);
            pthread_mutex_unlock(&mutexFTP);
            pthread_join(FTPThread, NULL);
            sleep(1);

            //LightControlFlag = 0;
            //pthread_mutex_lock(&mutex_2);
            //pthread_cond_signal(&cond1);
            //pthread_mutex_unlock(&mutex_2);
            //pthread_join(LightControlThread, NULL);

            //[vers | 2014.05.25| print left log]
            PrintLeftLog = 1;
            //[vers | end]          
 
            //machine idle x sec will let MasterFlag is true and zhResetFlag false
            
            if(MasterFlag)
            {
                //lock
                fd = open(dev, O_RDWR);
                if(fd < 0)
                {
                    perror("Open Fail");
                    return 1;
                }
                r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_3);
                if(r < 0)
                {
                    perror("Selection i2c device fail");
                    return 1;
                }
        
                i2c_smbus_write_byte_data(fd, OUT_P1, 0x07);
                i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
                close(fd);
                
                //hint for scan user
                digitalWrite (WiringPiPIN_15, LOW);
                digitalWrite (WiringPiPIN_16, LOW);
                digitalWrite (WiringPiPIN_18, HIGH);

                WaitBarcodeInput = 1;
                while(1)
                {
                    sleep(1);
                    if(ReadytoRead == 1)
                    {
                        pthread_mutex_lock(&mutexInput);
                        ReadytoRead = 0;
                        pthread_mutex_unlock(&mutexInput);
                        if(strncmp(tempString, "XXXP", 4) == 0)
                        {
                            pthread_mutex_lock(&mutexInput);
                            memset(UserNo, 0, sizeof(char)*InputLength);
                            tempPtr = tempString + 4;
                            memcpy(UserNo, tempPtr, sizeof(tempString)-3);
                            pthread_mutex_unlock(&mutexInput);
                        
                            //unlock
                            fd = open(dev, O_RDWR);
                            if(fd < 0)
                            {
                                perror("Open Fail");
                                return 1;
                            }
                            r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_3);
                            if(r < 0)
                            {
                                perror("Selection i2c device fail");
                                return 1;
                            }
        
                            i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
                            i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
                            close(fd);

                            break;
                        }else if(strncmp(tempString, "XXXM", 4) == 0)
                        {
                            pthread_mutex_lock(&mutexInput);
                            char FixerNo[InputLength];
                            struct timeval changeIntoRepairmodeTimeStemp;
                            unsigned char flagScanEventDone = 0;
                            pthread_t buttonThread;
                            memset(FixerNo, 0, sizeof(char)*InputLength);
                            tempPtr = tempString + 4;
                            memcpy(FixerNo, tempPtr, sizeof(tempString)-3);
                            pthread_mutex_unlock(&mutexInput); 

                            //unlock
                            fd = open(dev, O_RDWR);
                            if(fd < 0)
                            {
                                perror("Open Fail");
                                return 1;
                            }
                            r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_3);
                            if(r < 0)
                            {
                                perror("Selection i2c device fail");
                                return 1;
                            }
        
                            i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
                            i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
                            close(fd);
                
                            digitalWrite (WiringPiPIN_15, LOW);
                            digitalWrite (WiringPiPIN_16, LOW);
                            digitalWrite (WiringPiPIN_18, LOW);
                       
                            //get ip address
                            fd = socket(AF_INET, SOCK_DGRAM, 0);
                            ifr.ifr_addr.sa_family = AF_INET;
                            strncpy(ifr.ifr_name, ZHNetworkType, IFNAMSIZ-1);
                            ioctl(fd, SIOCGIFADDR, &ifr);
                            close(fd);
                            gettimeofday(&now, NULL);
                            gettimeofday(&changeIntoRepairmodeTimeStemp, NULL);

                            pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
                            fprintf(pfile, "%s %s %s 0 %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                                       inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                       MachineCode, FixerNo, MachREPAIRING);
#else
                            fprintf(pfile, "%s %s %s %ld %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                                         inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                         MachineCode, FixerNo, MachREPAIRING);
#endif
                            fclose(pfile);
            
                            FTPFlag = 1;
                            rc = pthread_create(&FTPThread, NULL, FTPFunction, NULL);
                            assert(rc == 0);
                            sleep(1);
                            FTPFlag = 0;
                            pthread_mutex_lock(&mutexFTP);
                            pthread_cond_signal(&condFTP);
                            pthread_mutex_unlock(&mutexFTP);
                            pthread_join(FTPThread, NULL);

                            ButtonFlag = 1;
                            rc = pthread_create(&buttonThread, NULL, ButtonListenFunction, NULL);
                            assert(rc == 0);

                            while(1)
                            {
                                sleep(1);
                                if(ReadytoRead == 1)
                                {
                                    pthread_mutex_lock(&mutexInput);
                                    ReadytoRead = 0;
                                    pthread_mutex_unlock(&mutexInput);
                                    if(strncmp(tempString, "XXXM", 4) == 0 && flagScanEventDone)
                                    {
                                        pthread_mutex_lock(&mutexInput);
                                        memset(FixerNo, 0, sizeof(char)*InputLength);
                                        tempPtr = tempString + 4;
                                        memcpy(FixerNo, tempPtr, sizeof(tempString)-3);
                                        pthread_mutex_unlock(&mutexInput);

                                        fd = socket(AF_INET, SOCK_DGRAM, 0);
                                        ifr.ifr_addr.sa_family = AF_INET;
                                        strncpy(ifr.ifr_name, ZHNetworkType, IFNAMSIZ-1);
                                        ioctl(fd, SIOCGIFADDR, &ifr);
                                        close(fd);
                                        gettimeofday(&now, NULL);

                                        pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
                                        fprintf(pfile, "%s %s %s 0 %ld 0 %s 5 %s %s %ld 0 0 %02d\n", 
                                                                                 ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                                 inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                 MachineCode, FixerNo, (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRDone);
#else
                                        fprintf(pfile, "%s %s %s %ld %ld 0 %s 5 %s %s %ld 0 0 %02d\n", 
                                                                                 ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                                 inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                                 MachineCode, FixerNo, (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRDone);
#endif
                                        fclose(pfile);
            
                                        FTPFlag = 1;
                                        rc = pthread_create(&FTPThread, NULL, FTPFunction, NULL);
                                        assert(rc == 0);
                                        sleep(1);
                                        FTPFlag = 0;
                                        pthread_mutex_lock(&mutexFTP);
                                        pthread_cond_signal(&condFTP);
                                        pthread_mutex_unlock(&mutexFTP);
                                        pthread_join(FTPThread, NULL);
                                        break;
                                    }else if(strncmp(tempString, "UUU", 3) == 0 && (!flagScanEventDone))
                                    {
                                        pthread_mutex_lock(&mutexInput);
                                        char fixItem[InputLength];
                                        memset(fixItem, 0, sizeof(char)*InputLength);
                                        tempPtr = tempString + 3;
                                        memcpy(fixItem, tempPtr, sizeof(tempString)-2);
                                        pthread_mutex_unlock(&mutexInput);

                                        //get ip address & time
                                        fd = socket(AF_INET, SOCK_DGRAM, 0);
                                        ifr.ifr_addr.sa_family = AF_INET;
                                        strncpy(ifr.ifr_name, ZHNetworkType, IFNAMSIZ-1);
                                        ioctl(fd, SIOCGIFADDR, &ifr);
                                        close(fd);
                                        gettimeofday(&now, NULL);

                                        pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
                                        fprintf(pfile, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                             ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), atoi(fixItem),
                                                                             MachineCode, FixerNo, (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRING);
#else
                                        fprintf(pfile, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n", 
                                                                             ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), atoi(fixItem),
                                                                             MachineCode, FixerNo, (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRING);
#endif
                                        fclose(pfile);
                                        flagScanEventDone = 1;
                                    }else
                                    {
                                        printf("FixerNo scan error code\n");
                                    }
                                }
                            }
                            ButtonFlag = 0;
                            pthread_join(buttonThread, NULL);
                            break; 
                        }
                        printf("UserNo scan error code\n");
                    }
                }
                WaitBarcodeInput = 0;

                //get ip address
                fd = socket(AF_INET, SOCK_DGRAM, 0);
                ifr.ifr_addr.sa_family = AF_INET;
                strncpy(ifr.ifr_name, ZHNetworkType, IFNAMSIZ-1);
                ioctl(fd, SIOCGIFADDR, &ifr);
                close(fd);
                gettimeofday(&now, NULL);

                pfile = fopen(UPLoadFile, "a");
#ifdef PrintMode
                fprintf(pfile, "%s %s %s 0 %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachUNLOCK);
#else
                fprintf(pfile, "%s %s %s %ld %ld 0 %s 5 %s %s 0 0 0 %02d\n", ISNo, ManagerCard, CountNo, PINCount[0][3], (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             MachineCode, UserNo, MachUNLOCK);
#endif
                fclose(pfile);
                digitalWrite (WiringPiPIN_15, HIGH);
                digitalWrite (WiringPiPIN_16, HIGH);
                digitalWrite (WiringPiPIN_18, LOW);
            }
        }
    }
    //*shm = '*';
    return 0;
}

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    curl_off_t nread;
    /* in real-world cases, this would probably get this data differently
       as this fread() stuff is exactly what the library already would do
       by default internally */
    size_t retcode = fread(ptr, size, nmemb, stream);

    nread = (curl_off_t)retcode;

    fprintf(stderr, "*** We read %" CURL_FORMAT_CURL_OFF_T
            " bytes from file\n", nread);
    return retcode;
}

void * FTPFunction(void *argument)
{
#ifdef LogMode
    Log(s, __func__, __LINE__, " FTP entry\n");
#endif
    //CURL *curl;
    //CURLcode res;
    //curl_off_t fsize;
    //FILE *hd_src;
    struct stat file_info, file_info_2;
    char UPLoadFile_3[UPLoadFileLength];
    struct timeval now;
    struct timespec outtime;
    int FTPCount = 0;

    while(FTPFlag)
    {
        //char Remote_url[UPLoadFileLength] = "ftp://192.168.10.254:21/home/";
        //char Remote_url[UPLoadFileLength] = "ftp://192.168.2.223:8888/";
        //struct curl_slist *headerlist=NULL;
        long size = 0;
        
        pthread_mutex_lock(&mutexFTP);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + FTPWakeUpValue;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&condFTP, &mutexFTP, &outtime);
        pthread_mutex_unlock(&mutexFTP);
        FTPCount = (FTPCount + FTPWakeUpValue) % FTPCountValue;
        pthread_mutex_lock(&mutexFile);
        
        if(stat(UPLoadFile, &file_info_2) == 0)
        {
            size = file_info_2.st_size;
            //printf("size:%ld\n", size);
        }
        pthread_mutex_unlock(&mutexFile);

        if(FTPCount == 0 || FTPFlag == 0 || size > 100000)
        {
            pthread_mutex_lock(&mutexFile);
            memset(UPLoadFile_3, 0, sizeof(char)*UPLoadFileLength);
            strcpy(UPLoadFile_3, UPLoadFile);
            gettimeofday(&now, NULL);
            sprintf(UPLoadFile,"%ld%s.txt",(long)now.tv_sec, MachineCode);
            pthread_mutex_unlock(&mutexFile);

            printf("%s\n", UPLoadFile_3);
            if(stat(UPLoadFile_3, &file_info) < 0) {
                printf("Couldnt open %s: %s\n", UPLoadFile_3, strerror(errno));
#ifdef LogMode
                Log(s, __func__, __LINE__, " FTP fail_1\n");
#endif
            }
            else if(file_info.st_size > 0)
            {
                pid_t proc = fork();
                if(proc < 0)
                {
                    printf("fork child fail\n");
                    return 0;
                }
                else if(proc == 0)
                {
                    char filePath[UPLoadFileLength];
                    char *pfile2;
                    memset(filePath, 0, sizeof(char)*UPLoadFileLength);
                    //strcpy(filePath, "/home/pi/zhlog/");
                    //strcpy(filePath, "/home/pi/works/NCR236/");
                    strcpy(filePath, UPLoadFile_3);
                    pfile2 = filePath;                       
                    printf("%s\n", pfile2);
                    execl("../.nvm/v0.10.25/bin/node", "node", "../mongodb/SendDataClient.js", filePath, (char *)0);
                    //execl("../../.nvm/v0.10.25/bin/node", "node", "../../mongodb/SendDataClient.js", filePath, (char *)0);
                }
                else
                {
                    int result = -1;
                    wait(&result);
                }
            }
            /*else if(file_info.st_size > 0)
            {
                strcat(Remote_url,UPLoadFile_3);
                fsize = (curl_off_t)file_info.st_size;

                curl_global_init(CURL_GLOBAL_ALL);

                curl = curl_easy_init();
                if(curl)
                {
                    hd_src = fopen(UPLoadFile_3, "rb");
                    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
                    //curl_easy_setopt(curl, CURLOPT_USERPWD, "raspberry:1234");
                    curl_easy_setopt(curl, CURLOPT_USERPWD, "taicon_ftp:2769247");
                    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
                    curl_easy_setopt(curl,CURLOPT_URL, Remote_url);
                    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);
                    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
                    res = curl_easy_perform(curl);

                    if(res != CURLE_OK)
                    {
                        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
#ifdef LogMode
                        Log(s, __func__, __LINE__, " FTP fail_2\n");
#endif
                        digitalWrite (WiringPiPIN_15, LOW);
                        digitalWrite (WiringPiPIN_16, LOW);
                        digitalWrite (WiringPiPIN_18, LOW);
                    }
                    else
                    {
                        digitalWrite (WiringPiPIN_15, LOW);
                        digitalWrite (WiringPiPIN_16, HIGH);
                        digitalWrite (WiringPiPIN_18, LOW);
                    }

                    //curl_slist_free_all (headerlist);
                    curl_easy_cleanup(curl);
                    if(hd_src)
                    {
                        fclose(hd_src);
                    }
                }
                curl_global_cleanup();
            }*/
            else;
            unlink(UPLoadFile_3);
        }
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, " FTP exit\n");
#endif
}

void * ButtonListenFunction(void *argument)
{
    while(ButtonFlag)
    {
       if(digitalRead(WiringPiPIN_22) == 0)
       {
           MasterFlag = 0;
           printf("MasterFlag = 0\n");
       } 
    }
}
