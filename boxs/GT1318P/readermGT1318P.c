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
#include <wiringSerial.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <errno.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <net/if.h>

#define I2C_IO_Extend_1 0x20
#define I2C_IO_Extend_2 0x21
#define I2C_IO_Extend_3 0x22

#define WiringPiPIN_11 0
#define WiringPiPIN_12 1
#define WiringPiPIN_13 2
#define WiringPiPIN_15 3
#define WiringPiPIN_16 4
#define WiringPiPIN_18 5
#define WiringPiPIN_22 6

#define IN_P0 0x00
#define IN_P1 0x01
#define OUT_P0 0x02
#define OUT_P1 0x03
#define INV_P0 0x04
#define INV_P1 0x05
#define CONFIG_P0 0x06
#define CONFIG_P1 0x07

#define InputLength 30 
#define UPLoadFileLength 21
#define CountPeriod 100
#define BusSize 1
#define SHMSZ 2000
#define WriteFilePerSize 300
#define RS232_Length 55
#define AgeCountNumber 5
#define goodrate 1

#define uart_length 48

#define FTPCountValue 300
#define FTPWakeUpValue 60

#define Log(s,func, line, opt) StringCat(func);StringCat(opt)
//#define Logmode
#define PrintInfo
#define PrintMode

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

enum
{
   Good = 0,
   CX_Bad,
   DX_Bad,
   LC_Over,
   Total
};

short zhResetFlag = 0;
short zhTelnetFlag = 0;
short FTPFlag = 0;

int SerialThreadFlag = 0;
int FileFlag = 0;
char *shm, *s, *tail;
char *shm_pop;
int updateFlag = 0;
unsigned char output[RS232_Length];
long Count[AgeCountNumber];
long ExCount[AgeCountNumber];
int PrintLeftLog = 0;

pthread_cond_t cond, condFTP;
pthread_mutex_t mutex, mutex_2, mutex_log, Mutexlinklist, mutexFTP;

typedef struct INPUTNODE
{
  char ISNo[InputLength];
  char ManagerCard[InputLength];
  char MachineCode[InputLength];
  char UserNo[InputLength];
  char CountNo[InputLength];
  char UPLoadFile[UPLoadFileLength];

  int BusLength;

  struct InputNode *link;

} InputNode;

InputNode *list = NULL;

void * zhLogFunction(void *argument);
void StringCat(const char *str);
void * SerialFunction(void *argument);
void * FTPFunction(void *argument);

double CXResult(unsigned char CX1, unsigned char CX2, unsigned char CX3, unsigned char CX4, unsigned char CX5, unsigned char CX6);
float DXResult(unsigned char DS1, unsigned char DS2, unsigned char DS3, unsigned char DS4, unsigned char DS5);
float LCResult(unsigned char LC1, unsigned char LC2, unsigned char LC3, unsigned char LC4, unsigned char LC5);

float LCSetter(unsigned char LC1, unsigned char LC2, unsigned char LC3, unsigned char LC4);
double CXSetter(unsigned char CX1, unsigned char CX2, unsigned char CX3);
double CXUpBoundSetter(unsigned char CX1, unsigned char CX2, unsigned char CX3);
double CXLowBoundSetter(unsigned char CX1, unsigned char CX2, unsigned char CX3);
float DXSetter(unsigned char DX1, unsigned char DX2, unsigned char DX3);

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);

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
    fprintf(pfile,"GT1318P\t%s\t0\t", buff);
    fclose(pfile);
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
            printf("ready to write log\n");
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
                    fprintf(pfile,"GT1318P\t%s\t%ld\t",buff,Count[Good]);
                }
            }      
    
            fclose(pfile);
            
            memset(LogString, 0, sizeof(char)*300);
            WriteFileCount = 0;
        }
        strncat(LogString, s_pop, 1);
        WriteFileCount++;
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

void * SerialFunction(void *argument)
{
#ifdef LogMode
    Log(s, __func__, __LINE__, "Serial Function entry\n");
#endif
    int fd;
    unsigned char temp_output[RS232_Length];
    int string_count = 0;
    int getString = 0;
    int readyToStart = 0;


    if((fd = serialOpen("/dev/ttyAMA0", 19200)) < 0)
    {
       printf("Unable to open serial device: %s\n", strerror(errno));
       pthread_exit((void*)"fail");
    }
    /*if(wiringPiSetup() == -1)
    {
        printf("Unable to start wiringPi: %s\n", strerror(errno)); 
        pthread_exit((void*)"fail");
    }*/
    while(SerialThreadFlag)
    {
        while(serialDataAvail(fd))
        {    
            unsigned char temp_char_1;
            temp_char_1 = serialGetchar(fd);
            //printf("%x ", temp_char_1);
            //if(temp_char_1 == 0x7D) printf("\n");
            if (temp_char_1 == 0x7D && getString == 0 && readyToStart == 1)
            {
               //count plus one and memset
               Count[Total]++;
               memset(temp_output, 0, sizeof(char)*RS232_Length);
               string_count = 0;
            }
            else if (temp_char_1 == 0x7B)
            {
                //default
                memset(temp_output,0, sizeof(char)*RS232_Length);
                getString = 1;
                Count[Total]++;
                temp_output[string_count] = temp_char_1;
                string_count++;
                readyToStart = 1;
            }
            else if(temp_char_1 == 0x7D && getString == 1)
            {
                //package
                if(string_count == 48)
                {
                    pthread_mutex_lock(&mutex_2);
                    
                    memset(output, 0, sizeof(char)*RS232_Length);
                    /*int vers_test_count = 0;
                    for(vers_test_count = 0; vers_test_count < string_count; vers_test_count++)
                    {
                        output[vers_test_count] = temp_output[vers_test_count];
                    }*/                
                    memcpy(output, temp_output, string_count);
               
                    updateFlag = 1; 
                    pthread_mutex_unlock(&mutex_2);
                }
                memset(temp_output, 0, sizeof(char)*RS232_Length);
                string_count = 0;
                getString = 0;
            }
            else if( getString == 1 && string_count < 48)
            {
                temp_output[string_count] = temp_char_1;
                string_count++;
            }
            else;
            fflush (stdout) ;
        }
    }
    if(fd >= 0)
    {
        serialClose(fd);
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, "Serial Function exit\n");
#endif
}

void *FileFunction(void *argument)
{
#ifdef LogMode
    Log(s, __func__, __LINE__, "File Function entry \n");
#endif

    struct timeval now;
    struct timespec outtime;
    struct ifreq ifr;

    int fd;

    int ForCount;
    int setAtFirstTime = 1;
    unsigned char charOutput[RS232_Length];
    unsigned char SetOutput[18];
    FILE *pfile;
    float LCCheck, DXUpperBond;
    double CXLowerBond, CXUpperBond, CXCheck;
    LCCheck = CXLowerBond = 0;
    CXUpperBond = CXCheck = DXUpperBond = 0;
 
    memset(SetOutput, 0, sizeof(char)*18);

    while(FileFlag)   
    {
        pthread_mutex_lock(&mutex);

        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
        ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);
        
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec;
        outtime.tv_nsec = ((now.tv_usec + (CountPeriod*1000)) * 1000);
        if( outtime.tv_nsec  > 1000000000)
        {
            outtime.tv_sec = outtime.tv_sec + 1;
            outtime.tv_nsec = outtime.tv_nsec % 1000000000;
        }
        pthread_cond_timedwait(&cond, &mutex, &outtime);
        pthread_mutex_unlock(&mutex);
        //printf("%ld.%ld\n", outtime.tv_sec, outtime.tv_nsec);

        memset(charOutput, 0, sizeof(char)*RS232_Length);
        gettimeofday(&now, NULL);
        
        if(updateFlag == 1)
        {
            pthread_mutex_lock(&mutex_2);
            int resetFlag = 0;
            
            memcpy(charOutput, output,RS232_Length);

            updateFlag = 0;
            memset(output, 0, sizeof(char)*RS232_Length);
            pthread_mutex_unlock(&mutex_2);
            /*for(ForCount = 0; ForCount < 49; ForCount++)
            {
                printf("%x ", charOutput[ForCount]);
            }
            printf("\n");*/
            for(ForCount = 0; ForCount < 18; ForCount++)
            {
                if(charOutput[ForCount] != SetOutput[ForCount])
                {
                    resetFlag = 1;
                    break;
                }
            }
            if(resetFlag == 1 && setAtFirstTime == 0)
            {
                memcpy(SetOutput, charOutput, sizeof(char)*18);
                LCCheck = LCSetter(SetOutput[1], SetOutput[2], SetOutput[3], SetOutput[4]);
                CXCheck = CXSetter(SetOutput[12], SetOutput[13], SetOutput[14]);
                CXUpperBond = CXCheck * CXUpBoundSetter(SetOutput[9], SetOutput[10], SetOutput[11]);
                CXLowerBond = CXCheck * CXLowBoundSetter(SetOutput[6], SetOutput[7], SetOutput[8]);
                DXUpperBond = DXSetter(SetOutput[15], SetOutput[16], SetOutput[17]);

                //reset input
                //[vers | we do not have real input so ~ mask it]
                /*pthread_mutex_lock(&Mutexlinklist);
                InputNode *p = list;           
                pfile = fopen(p->UPLoadFile, "a");
                for(ForCount = 0; ForCount < (AgeCountNumber-1); ForCount++)
                {
                    if((ExCount[ForCount] != Count[ForCount]) && ForCount == 0)
                    {
                        fprintf(pfile, "%s\t%s\t%s\t%ld\t%ld\t0\t%s\t1\t%s\t%s\n", 
                                                                             p->ISNo, p->ManagerCard, p->CountNo, Count[Good], (long)now.tv_sec,
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             p->MachineCode, p->UserNo);
                    }else if(ExCount[ForCount] != Count[ForCount])
                    {
                        fprintf(pfile, "%s\t%s\t%s\t%ld\t%ld\t%ld\t%s\t%d\t%s\t%s\n", 
                                                                             p->ISNo, p->ManagerCard, p->CountNo, Count[Good], (long)now.tv_sec,
                                                                             Count[ForCount],
                                                                             inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                             ForCount+1, p->MachineCode, p->UserNo);
                    }
                    else;
                }
                fclose(pfile);
        
                stat(p->UPLoadFile, &st);
                size = st.st_size;      

                strcpy(UPLoadFile_2, p->UPLoadFile);
                if(size > 0){
                    rc = pthread_create(&FTPThread, NULL, FTPFunction, NULL);
                    assert(rc == 0);
                }
                if(list->link != NULL)
                {    
                    list = list->link;
                    gettimeofday(&now, NULL);
                    sprintf(list->UPLoadFile,"%ld%s.txt",(long)now.tv_sec, list->MachineCode); 
                }
                else
                {
                    list = NULL;
                    // if 
                    if(size > 0){
                        pthread_join(FTPThread, NULL);
                    }
                }
                pthread_mutex_unlock(&Mutexlinklist);
                free(p);*/
                
                memset(Count, 0, sizeof(long)*AgeCountNumber);
                memset(ExCount, 0, sizeof(long)*AgeCountNumber);
                //[vers |之前已經將total 加一, 清掉的話會少一]                
                Count[Total] = Count[Total] +1 ; 
                //[vers | end]
            }
            else if(resetFlag == 1 && setAtFirstTime == 1)
            {
                memcpy(SetOutput, charOutput, sizeof(char)*18);
                LCCheck = LCSetter(SetOutput[1], SetOutput[2], SetOutput[3], SetOutput[4]);
                CXCheck = CXSetter(SetOutput[12], SetOutput[13], SetOutput[14]);
                CXUpperBond = CXCheck * CXUpBoundSetter(SetOutput[9], SetOutput[10], SetOutput[11]);
                CXLowerBond = CXCheck * CXLowBoundSetter(SetOutput[6], SetOutput[7], SetOutput[8]);
                DXUpperBond = DXSetter(SetOutput[15], SetOutput[16], SetOutput[17]);
                setAtFirstTime = 0;
            }
            else;
            double cx = CXResult(charOutput[25], charOutput[26], charOutput[27], charOutput[28], charOutput[29], charOutput[30]);
            float dx = DXResult(charOutput[31], charOutput[32], charOutput[33], charOutput[34], charOutput[35]);
            float lc = LCResult(charOutput[36], charOutput[37], charOutput[38], charOutput[39], charOutput[40]);
#ifdef PrintInfo
            printf("cx:%f %f %f\n", CXLowerBond, cx, CXUpperBond);
            printf("dx:%f %f\n", DXUpperBond, dx);
            printf("lc:%f %f\n", LCCheck, lc);            
#endif
            if (lc >= LCCheck)//大於設定
            {
                Count[LC_Over] = Count[LC_Over] + 1;
            }else if(lc < 0) // over
            {
                Count[LC_Over] = Count[LC_Over] + 1;
            }else if(cx >= CXUpperBond || cx <= CXLowerBond) //cx upper/lower bound
            {
                Count[CX_Bad] = Count[CX_Bad] + 1;
            }else if(cx < 0) // cx high/low
            {
                Count[CX_Bad] = Count[CX_Bad] + 1;
            }else if (dx >= DXUpperBond) //dx upperbond
            {
                Count[DX_Bad] = Count[DX_Bad]+ 1;
            }else if(dx < 0)
            {
                if(dx != -3)
                {
                    Count[DX_Bad] = Count[DX_Bad]+ 1;
                }
            }else 
            {
                Count[Good] = Count[Good] + 1;
            }
            pthread_mutex_lock(&Mutexlinklist);
            InputNode *p = list;

            if(p != NULL)
            {
                pfile = fopen(p->UPLoadFile, "a");
                for(ForCount = 0; ForCount < (AgeCountNumber-1) ; ForCount++)
                {
                    if((ExCount[ForCount] != Count[ForCount]) && ForCount == 0)
                    {
#ifdef PrintMode
                        fprintf(pfile, "%s %s %s %ld %ld 0 %s 1 %s %s %lf %lf %lf %02d\n", 
                                                                               p->ISNo, p->ManagerCard, p->CountNo, 
                                                                               Count[Good] - ExCount[Good], 
                                                                               (long)now.tv_sec,
                                                                               inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                               p->MachineCode, p->UserNo, cx, dx, lc, MachRUNNING);
#else
                        fprintf(pfile, "%s %s %s %ld %ld 0 %s 1 %s %s %lf %lf %lf %02d\n", 
                                                                               p->ISNo, p->ManagerCard, p->CountNo, Count[Good], (long)now.tv_sec,
                                                                               inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), 
                                                                               p->MachineCode, p->UserNo, cx, dx, lc, MachRUNNING);
#endif
                        break; // only have one in all for loop
                    }else if(ExCount[ForCount] != Count[ForCount])
                    {
#ifdef PrintMode
                        fprintf(pfile, "%s %s %s 0 %ld %ld %s %d %s %s %lf %lf %lf %02d\n", 
                                                               p->ISNo, p->ManagerCard, p->CountNo, (long)now.tv_sec,
                                                               Count[ForCount] - ExCount[ForCount],
                                                               inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ForCount+1, 
                                                               p->MachineCode, p->UserNo, cx, dx, lc, MachRUNNING);
#else
                        fprintf(pfile, "%s %s %s %ld %ld %ld %s %d %s %s %lf %lf %lf %02d\n",
                                                               p->ISNo, p->ManagerCard, p->CountNo, Count[Good], (long)now.tv_sec,Count[ForCount],
                                                               inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), ForCount+1, 
                                                               p->MachineCode, p->UserNo, cx, dx, lc, MachRUNNING);
#endif
                        break;
                    }else;
                }
                fclose(pfile); 
                memcpy(ExCount, Count, sizeof(long)*AgeCountNumber);
#ifdef PrintInfo
                printf("%s %s %s %s %s %ld %ld %ld %ld %ld\n",
                     p->ISNo, p->ManagerCard, p->CountNo, p->MachineCode, p->UserNo ,Count[Good], Count[CX_Bad], Count[DX_Bad],  Count[LC_Over], Count[Total]);
#endif
            }
            else printf("node = null\n");
            pthread_mutex_unlock(&Mutexlinklist);
        }
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, "File Function exit\n");
#endif
}

int main(int argc ,char *argv[])
{
    char *dev = "/dev/i2c-1";
    int rc;    
    pthread_t LogThread, SerialThread, FileThread, TelnetControlThread, FTPThread;
    int shmid;
    key_t key;

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_2, NULL);
    pthread_mutex_init(&mutex_log, NULL);
    pthread_mutex_init(&Mutexlinklist, NULL);
    pthread_mutex_init(&mutexFTP, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&condFTP, NULL);

    wiringPiSetup(); 

    pinMode(WiringPiPIN_15, OUTPUT);
    pinMode(WiringPiPIN_16, OUTPUT);
    pinMode(WiringPiPIN_18, OUTPUT);
    pinMode(WiringPiPIN_22, INPUT);
    pullUpDnControl (WiringPiPIN_22, PUD_UP); 

    int fd, r;
    int MasterFlag;
    long goodCount;
    char tempString[InputLength], *tempPtr;
    struct timeval now;
    FILE *pfile;
    
    /*scanner check
    * 1. ISNO
    * 2. manager card
    * 3. machine code
    * 4. user No
    * 5. Count No
    */

#ifdef LogMode
    rc = pthread_create(&LogThread, NULL, zhLogFunction, NULL);
    assert(rc == 0);
    key = 8765;
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
    //tail = shm + (SHMSZ - 1);
#endif    

#ifdef LogMode
    Log(s, __func__, __LINE__, " ready to init\n");
#endif

    /*fd = open(dev, O_RDWR);
    if(fd < 0)
    {
        error("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, I2C_IO_Extend_1);
    if(r < 0)
    {
        perror("Selecting i2c device");
        return 1;
    }
    i2c_smbus_write_byte_data(fd, OUT_P0, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIG_P0, 0x03);

    i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00); 

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
    i2c_smbus_write_byte_data(fd, CONFIG_P0, 0x03);

    i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
    close(fd);*/
    
    //[vers|start age thread and now will not stop]
    /*[vers| block for auto run]
    SerialThreadFlag = 1;
    rc = pthread_create(&SerialThread, NULL, SerialFunction, NULL);
    assert(rc == 0);

    FileFlag = 1;
    rc = pthread_create(&FileThread, NULL, FileFunction, NULL);
    assert(rc == 0);
    [vers|end]*/

    //[end]
    //the mechine always standby
    while(1)
    {
        unsigned char isNormalStop = 0;
        InputNode *node;
        node = (InputNode *) malloc(sizeof(InputNode));
        if(node == NULL)
        {
            continue;
        }

        node->link = NULL;

        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, HIGH);

        /*fd = open(dev, O_RDWR);
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
        i2c_smbus_write_byte_data(fd, CONFIG_P0, 0x03);
        
        i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
        i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
        close(fd);*/

        //3rd i3c board will control 3*8 control
        printf("Ready to work!!\n");
        /*
        while(1)
        {
            memset(tempString, 0, sizeof(char)* InputLength);
            gets(tempString);
            if(strncmp(tempString, "YYY", 3) == 0)
            {
                memset(node->ISNo, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(node->ISNo, tempPtr, sizeof(tempString)-2);
                // ready for light some led;
                break;
            }
            printf("scan ISNo error code\n");
        }
        while(1)
        {
            memset(tempString, 0, sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "QQQ", 3) == 0)
            {
                memset(node->ManagerCard, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(node->ManagerCard, tempPtr, sizeof(tempString)-2);
                break;
            }
            printf("ManagerCard scan error code\n");
        }
        while(1)
        {
            memset(tempString, 0 , sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "ZZZ", 3) == 0)
            {
                memset(node->MachineCode, 0 , sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(node->MachineCode, tempPtr, sizeof(tempString)-2);
                break;
            }
            printf("MachineCode scan error code\n");
        }
        while(1)
        {
            memset(tempString, 0, sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "WWW", 3) == 0)
            {
                memset(node->CountNo, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(node->CountNo, tempPtr, sizeof(tempString)-2);
                
                int tempCount = atoi(node->CountNo);
                node->BusLength = tempCount/BusSize + 6;

                if(tempCount %BusSize != 0) node->BusLength = node->BusLength + 1;

                break;
            }
            printf("CountNo scan error code\n");
        }
        while(1)
        {
            memset(tempString, 0, sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "XXXP", 4) == 0)
            {
                memset(node->UserNo, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 4;
                memcpy(node->UserNo, tempPtr, sizeof(tempString)-3);
                break;
            }
            printf("UserNo scan error code\n");
        }*/
 

        
        char FakeInput[5][InputLength];
        memset(FakeInput, 0, sizeof(char)*(5*InputLength));
        int filesize, FakeInputNumber = 0;
        int FakeInputNumber_2 = 0;
        char * buffer, * charPosition;
        short FlagNo = 0;        

        pfile = fopen("/home/pi/works/GT1318P/barcode","r");
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
        memset(node->ISNo, 0, sizeof(char)*InputLength);
        strcpy(node->ISNo, FakeInput[0]);
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, HIGH);

        sleep(1);
        memset(node->ManagerCard, 0, sizeof(char)*InputLength);
        strcpy(node->ManagerCard, FakeInput[1]);
        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, LOW);
        digitalWrite (WiringPiPIN_18, HIGH);

        sleep(1);
        memset(node->MachineCode, 0 , sizeof(char)*InputLength);
        strcpy(node->MachineCode, FakeInput[2]);
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, LOW);
        digitalWrite (WiringPiPIN_18, HIGH);

        sleep(1);
        memset(node->CountNo, 0, sizeof(char)*InputLength);
        strcpy(node->CountNo, FakeInput[3]);
        goodCount = (atoi(node->CountNo)*goodrate);
        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, LOW);

        sleep(1);
        memset(node->UserNo, 0, sizeof(char)*InputLength);
        strcpy(node->UserNo, FakeInput[4]);
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, LOW);
        
        gettimeofday(&now, NULL);

        memset(node->UPLoadFile, 0, sizeof(char)*UPLoadFileLength);
        sprintf(node->UPLoadFile,"%ld%s.txt", (long)now.tv_sec, node->MachineCode); 
        //[vers| avoid no data income casue ftp upload will fail]
        //pfile = fopen(node->UPLoadFile, "a");
        //fclose(pfile);
        //[vers| end]
 
        printf("%s %s %s %s %s %s\n", node->ISNo, node->ManagerCard, node->MachineCode, node->UserNo, node->CountNo, node->UPLoadFile);
        memset(Count, 0, sizeof(long)*AgeCountNumber);
        memset(ExCount, 0, sizeof(long)*AgeCountNumber);
        MasterFlag = 1;

        pthread_mutex_lock(&Mutexlinklist);
        if(list == NULL) 
        {
            list = node;
        }
        else
        {
            InputNode *p = list;
            while(p->link!=NULL)
            {
                p = p->link;
            }
            p->link = node;
            
        }
        pthread_mutex_unlock(&Mutexlinklist);

        if(zhTelnetFlag == 0)
        {
            //zhTelnetFlag = 1;
            //rc = pthread_create(&TelnetControlThread, NULL, RemoteControl, NULL);
            //assert(rc == 0);
        }

        while(MasterFlag)
        {
            PrintLeftLog = 0;

            zhResetFlag = 0; //reset flag clean
            SerialThreadFlag = 1;
            rc = pthread_create(&SerialThread, NULL, SerialFunction, NULL);
            assert(rc == 0);

            FileFlag = 1;
            rc = pthread_create(&FileThread, NULL, FileFunction, NULL);
            assert(rc == 0);
           
            FTPFlag = 1;
            rc = pthread_create(&FTPThread, NULL, FTPFunction, NULL);
            assert(rc == 0);

            while(zhResetFlag == 0)
            {
                usleep(100000);
                /*if(Count[Good] >= atoi(node->CountNo))
                {
                    //finish job
                    printf("Houston we are ready to back!\n");
                    zhResetFlag = 1;
                    MasterFlag = 0;
                    //free(node);
                    //list = NULL;
                }*/
                if (list == NULL)
                {
                    printf("Houston we are ready to back!\n");
                    zhResetFlag = 1;
                    MasterFlag = 0;
                }else if(digitalRead(WiringPiPIN_22) == 0)
                {
                    printf("Houston we are ready to back!(PIN_22)\n");
                    if(list->link != NULL)
                    {
                        InputNode *p = list;
                        list = list->link;
                        free(p);
                        SerialThreadFlag = 0;
                        pthread_join(SerialThread, NULL);
                        sleep(1);

                        FileFlag = 0;
                        pthread_mutex_lock(&mutex);
                        pthread_cond_signal(&cond);
                        pthread_mutex_unlock(&mutex);
                        pthread_join(FileThread, NULL);
                        sleep(1);

                        FTPFlag = 0;
                        pthread_mutex_lock(&mutexFTP);
                        pthread_cond_signal(&condFTP);
                        pthread_mutex_unlock(&mutexFTP);
                        pthread_join(FTPThread, NULL);
                        sleep(1);

                        memset(list->UPLoadFile, 0, sizeof(char)*UPLoadFileLength);
                        gettimeofday(&now, NULL);
                        sprintf(list->UPLoadFile,"%ld%s.txt", (long)now.tv_sec, list->MachineCode); 

                        memset(Count, 0, sizeof(long)*AgeCountNumber);
                        memset(ExCount, 0, sizeof(long)*AgeCountNumber);

                        SerialThreadFlag = 1;
                        rc = pthread_create(&SerialThread, NULL, SerialFunction, NULL);
                        assert(rc == 0);

                        FileFlag = 1;
                        rc = pthread_create(&FileThread, NULL, FileFunction, NULL);
                        assert(rc == 0);
           
                        FTPFlag = 1;
                        rc = pthread_create(&FTPThread, NULL, FTPFunction, NULL);
                        assert(rc == 0);

                    }else
                    {
                        InputNode *p = list;
                        list = NULL;
                        free(p);
                        zhResetFlag = 1;
                        MasterFlag = 0;
                    }                
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
                    while(list != NULL)
                    {
                        InputNode *p = list;
                        list = list->link;
                        free(p);
                    }
                }*/
                else;
            }
            
            SerialThreadFlag = 0;
            pthread_join(SerialThread, NULL);
            sleep(1);

            FileFlag = 0;
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            pthread_join(FileThread, NULL);
            sleep(1);

            FTPFlag = 0;
            pthread_mutex_lock(&mutexFTP);
            pthread_cond_signal(&condFTP);
            pthread_mutex_unlock(&mutexFTP);
            pthread_join(FTPThread, NULL);
            sleep(1);
            PrintLeftLog = 1;
            
            /*if(MasterFlag)
            {
                digitalWrite (WiringPiPIN_15, HIGH);
                digitalWrite (WiringPiPIN_16, HIGH);
                digitalWrite (WiringPiPIN_18, LOW);
                while(1)
                {
                    memset(tempString, 0, sizeof(char)*InputLength);
                    gets(tempString);
                    if(strncmp(tempString, "XXX", 3) == 0)
                    {
                        memset(UserNo, 0, sizeof(char)*InputLength);
                        tempPtr = tempString + 3;
                        memcpy(UserNo, tempPtr, sizeof(tempString)-2);
                        break;
                   
                        digitalWrite (WiringPiPIN_15, LOW);
                        digitalWrite (WiringPiPIN_16, HIGH);
                        digitalWrite (WiringPiPIN_18, LOW);
                    }
                    printf("UserNo scan error code\n");
                }
            }*/
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
    short checkFlag = 0;

    while(FTPFlag){
        //char Remote_url[80] = "ftp://192.168.20.254:21/home/";
        //char Remote_url[80] = "ftp://192.168.2.223:8888/";
        long size = 0;
        pthread_mutex_lock(&mutexFTP);
        //struct curl_slist *headerlist=NULL;
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + FTPWakeUpValue;
        outtime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&condFTP, &mutexFTP, &outtime);
        pthread_mutex_unlock(&mutexFTP);
       
        FTPCount = (FTPCount + FTPWakeUpValue) % FTPCountValue;
        /*pthread_mutex_lock(&Mutexlinklist);
        InputNode *p = list;
        if (p != NULL){
            if(!stat(UPLoadFile, &file_info_2))
            {
                size = file_info_2.st_size;
                //printf("size:%ld\n", size);
            }
        }
        pthread_mutex_unlock(&Mutexlinklist);
        */
        if(FTPCount == 0 || FTPFlag == 0 || size > 100000)
        {
            pthread_mutex_lock(&Mutexlinklist);
            
            InputNode *p = list;
            if(p != NULL)
            {
                memset(UPLoadFile_3, 0, sizeof(char)*UPLoadFileLength);
                strcpy(UPLoadFile_3, p->UPLoadFile);
                gettimeofday(&now, NULL);
                sprintf(p->UPLoadFile,"%ld%s.txt",(long)now.tv_sec, p->MachineCode);
                //hd_src = fopen(p->UPLoadFile, "a");
                //if (hd_src != NULL)
                //{
                //    fclose(hd_src);
                //}
                checkFlag = 1;
            }
            pthread_mutex_unlock(&Mutexlinklist);
            if(checkFlag == 1)
            {
                printf("%s\n", UPLoadFile_3);
                if(stat(UPLoadFile_3, &file_info) < 0) 
                {
                    printf("Couldnt open %s: %s\n", UPLoadFile_3, strerror(errno));
#ifdef LogMode
                    Log(s, __func__, __LINE__, " FTP fail_1\n");
#endif
                    digitalWrite (WiringPiPIN_15, LOW);
                    digitalWrite (WiringPiPIN_16, LOW);
                    digitalWrite (WiringPiPIN_18, LOW);
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
                        char filePath[80];
                        char *pfile2;
                        memset(filePath, 0, sizeof(char)*80);
                        //strcpy(filePath, "/home/pi/zhlog/");
                        //strcpy(filePath, "/home/pi/works/GT1318P/");
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
                checkFlag = 0;
            }
        }
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, " FTP exit\n");
#endif
}

double CXResult(unsigned char CX1, unsigned char CX2, unsigned char CX3, unsigned char CX4, unsigned char CX5, unsigned char CX6)
{ 
    if (CX1 == 0x20 && CX2 == 0x48 && CX3 == 0x49 && CX4 == 0x47 && CX5 == 0x48  && CX6 == 0x20 )
    {
        return -1;
    }
    else if(CX1 == 0x20  &&  CX2 == 0x20 && CX3 == 0x4c && CX4 == 0x4f && CX5 == 0x57 && CX6 == 0x20 )
    {
        return -2;
    }
    else
    {
        double ans = 0;
        double plus = 1;
        unsigned char a1, a2, a3, a4, a5;
        
        if(CX6 == 0x75) plus = 1000;
        else if(CX6 == 0x6D) plus = 1000000;
        else;

        if(CX1 == 0x2e)
        {
            a1 = (CX2 & 0x0f);
            a2 = (CX3 & 0x0f);
            a3 = (CX4 & 0x0f);
            a4 = (CX5 & 0x0f);
            //printf("ans: %x%x%x%x\n",a1,a2,a3,a4);
            ans = (((double) a1) * 0.1 + ((double)a2) * 0.01 + ((double)a3) * 0.001 + ((double)a4) * 0.0001 )*plus;
            return ans;
        }else if(CX2 == 0x2e)
        {
            a1 = (CX1 & 0x0f);
            a2 = (CX3 & 0x0f);
            a3 = (CX4 & 0x0f);
            a4 = (CX5 & 0x0f);
            //printf("ans: %x%x%x%x\n",a1,a2,a3,a4);
            ans = (((double) a1) + ((double)a2) * 0.1 + ((double)a3) * 0.01 + ((double)a4) * 0.001 )*plus;
            return ans;
        }else if(CX3 == 0x2e)
        {
            a1 = (CX1 & 0x0f);
            a2 = (CX2 & 0x0f);
            a3 = (CX4 & 0x0f);
            a4 = (CX5 & 0x0f);
            //printf("ans: %x%x%x%x\n",a1,a2,a3,a4);
            ans = (((double) a1) * 10 + ((double)a2) + ((double)a3) * 0.1 + ((double)a4) * 0.01 )*plus;
            return ans;
        }else if(CX4 == 0x2e)
        {
            a1 = (CX1 & 0x0f);
            a2 = (CX2 & 0x0f);
            a3 = (CX3 & 0x0f);
            a4 = (CX5 & 0x0f);
            //printf("ans: %x%x%x%x\n",a1,a2,a3,a4);
            ans = (((double) a1) * 100 + ((double)a2) * 10 + ((double)a3) + ((double)a4) * 0.1 )*plus;
            return ans;
        }else
        {
            a1 = (CX1 & 0x0f);
            a2 = (CX2 & 0x0f);
            a3 = (CX3 & 0x0f);
            a4 = (CX4 & 0x0f);
            a5 = (CX5 & 0x0f);
            //printf("ans: %x%x%x%x\n",a1,a2,a3,a4);
            ans = (((double) a1) * 10000 + ((double)a2) * 1000 + ((double)a3) * 100 + ((double)a4)*10 + ((double)a5) )*plus;
            return ans;
        }
    }
}

float DXResult(unsigned char DS1, unsigned char DS2, unsigned char DS3, unsigned char DS4, unsigned char DS5)
{
    if(DS1 == 0x48 && DS2 == 0x49)
    {
        return -1;
    }else if(DS1 == 0x4c && DS2 == 0x4f)
    {
        return -2;
    }else if(DS1 == 0x30 && DS2 == 0x2e)
    {
        float ans;
        DS3 = (DS3 & 0x0f);
        DS4 = (DS4 & 0x0f);
        DS5 = (DS5 & 0x0f);
        ans = ((float)DS3) * 10 + ((float)DS4) + ((float)DS5) * 0.1;
        return ans;
    }else return -3;
}

float LCResult(unsigned char LC1, unsigned char LC2, unsigned char LC3, unsigned char LC4, unsigned char LC5)
{
    unsigned char a1,a2,a3,a4;    
    float ans =0;
    if(LC1 == 0x20 && LC2 == 0x4f  && LC3 == 0x56 && LC4 == 0x45 && LC5 ==  0x42)
        return -1;
    else 
    {
        if (LC1 == 0x2e)
        {
            a1 = (LC2 & 0x0f);
            a2 = (LC3 & 0x0f);
            a3 = (LC4 & 0x0f);
            a4 = (LC5 & 0x0f);
            ans = ((float) a1) * 0.1 + ((float)a2) * 0.01 + ((float)a3) * 0.001 + ((float)a4) * 0.0001;
            return ans;
        }else if(LC2 == 0x2e)
        {
            a1 = (LC1 & 0x0f);
            a2 = (LC3 & 0x0f);
            a3 = (LC4 & 0x0f);
            a4 = (LC5 & 0x0f);
            ans = ((float)a1) + ((float)a2) * 0.1 + ((float)a3)*0.01 + ((float)a4) * 0.001;
            return ans;
        }else if(LC3 == 0x2e)
        {
            a1 = (LC1 & 0x0f);
            a2 = (LC2 & 0x0f);
            a3 = (LC4 & 0x0f);
            a4 = (LC5 & 0x0f);
            ans = ((float)a1)* 10 + ((float)a2) + ((float)a3) * 0.1 + ((float)a4)* 0.01;
            return ans;
        }else if(LC4 == 0x02e)
        { 
            a1 = (LC1 & 0x0f);
            a2 = (LC2 & 0x0f);
            a3 = (LC3 & 0x0f);
            a4 = (LC5 & 0x0f);
            ans = ((float)a1)* 100 + ((float)a2) * 10 + ((float)a3) + ((float)a4) * 0.1;
            return ans;
        }else
        {
            a1 = (LC1 & 0x0f);
            a2 = (LC2 & 0x0f);
            a3 = (LC3 & 0x0f);
            a4 = (LC4 & 0x0f);
            ans = ((float)a1) * 1000 + ((float)a2) * 100 + ((float)a3) * 10 + ((float)a4);
            return ans;
        }
    }
}

float LCSetter(unsigned char LC1, unsigned char LC2, unsigned char LC3, unsigned char LC4) 
{
    float ans = 0;
    LC1 = (LC1 & 0x0f);
    LC2 = (LC2 & 0x0f);
    LC3 = (LC3 & 0x0f);
    LC4 = (LC4 & 0x0f);
    ans = ((float)LC1) * 100 + ((float)LC2) *10 + ((float)LC3) + ((float)LC4)* 0.1;
    return ans;
}

double CXSetter(unsigned char CX1, unsigned char CX2, unsigned char CX3)
{
    double ans = 0;
    CX1 = (CX1 & 0x0f);
    CX2 = (CX2 & 0x0f);
    CX3 = (CX3 & 0x0f);
    ans = (((double)CX1) * 10 + ((double)CX2)) * pow(10 ,(double)CX3);
    return ans;
}

double CXUpBoundSetter(unsigned char CX1, unsigned char CX2, unsigned char CX3)
{
    double ans = 0;
    CX1 = (CX1 & 0x0f);
    CX2 = (CX2 & 0x0f);
    CX3 = (CX3 & 0x0f);
    ans = 1 + ((double)CX1) * 0.1 + ((double)CX2) * 0.01 + ((double)CX3) * 0.001;
    return ans;
}

double CXLowBoundSetter(unsigned char CX1, unsigned char CX2, unsigned char CX3)
{
    double ans = 0;
    CX1 = (CX1 & 0x0f);
    CX2 = (CX2 & 0x0f);
    CX3 = (CX3 & 0x0f);
    ans = 1 - ((double)CX1)*0.1 - ((double)CX2) * 0.01 - ((double)CX3) * 0.001;
    //printf("%f\n", ans);
    return ans;
}

float DXSetter(unsigned char DX1, unsigned char DX2, unsigned char DX3)
{
    float ans = 0;
    DX1 = (DX1 & 0x0f);
    DX2 = (DX2 & 0x0f);
    DX3 = (DX3 & 0x0f);
    ans = ((float)DX1) *10 + ((float)DX2) + ((float)DX3)*0.1;
    return ans;
}
