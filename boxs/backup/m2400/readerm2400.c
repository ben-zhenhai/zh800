#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <time.h>
#include <math.h>


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

#define SHMSZ 2000

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

#define WatchDogCountValue 120
#define InputLength 20
#define UPLoadFileLength 21
#define CountPeriod 1
#define FTPCountValue 300
#define WriteFileCountValue 4
#define FTPWakeUpValue 60

#define Log(s,func, line, opt) StringCat(func);StringCat(opt)
#define RS232_Length 100

//#define LogMode
#define PrintInfo
#define ProductCount 5
#define EventCount 16

enum
{
    Good = 0,
    Insert,
    //Total,
    Start,
    Stop,
    Reset
};

enum
{
    Bad_A = 0,
    Bad_B,
    Bad_C,
    Bad_D,
    Position, //素子位置
    Left,     //素子殘留
    Shell,    //外殼
    Tube,     //套管
    InsertError, //插入
    ExportWhite, //露白
    FrontBreak,  //前段
    BackBreak,   //後段
    Seal, //封口
    Liquid, //液面A
    Vacuum, //真空A
    Eraser    //像皮
};

int SerialThreadFlag = 0;
int WatchDogFlag = 0;
short zhResetFlag = 0;
int PrintLeftLog = 0;
int FileFlag = 0;
int updateFlag = 0;
int string_count = 0;
short zhTelnetFlag = 0;
short InterruptEnable = 0;
short FTPFlag = 0;

char *shm_pop;
char *shm, *s, *tail;
char output[RS232_Length];

pthread_cond_t cond, condFTP;
pthread_mutex_t mutex, mutex_3, mutex_log, mutex_2, mutexFTP, mutexFile;

long productCountArray[ProductCount];
long ExproductCountArray[ProductCount];
long messageArray[EventCount];
long ExmessageArray[EventCount];

char ISNo[InputLength], ManagerCard[InputLength], MachineCode[InputLength], UserNo[InputLength], CountNo[InputLength];
char UPLoadFile[UPLoadFileLength];

long PINCount[2][8];
long PINEXCount[2][8];
int I2CEXValue[2];
short InterruptCount[4];

void * zhLogFunction(void *argument);
void * FTPFunction(void *arguemnt);
void StringCat(const char *str);
void * SerialFunction(void *argument);
void * RemoteControl(void *argument);
void * zhINTERRUPT1(void *argument);
int transferFormatINT(char x);
long transferFormatLONG(char x);

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);

void * zhINTERRUPT1(void * argument)
{
#ifdef LogMode 
    Log(s, __func__, __LINE__, " entry\n");
#endif
    while(InterruptEnable)
    {
        char *dev = "/dev/i2c-1";

        int fd , r;

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
        int x, y;
        int ForCount ,first, second;

        x = i2c_smbus_read_byte_data(fd, IN_P0);
        y = i2c_smbus_read_byte_data(fd, IN_P1);
        close(fd);
        if(x != I2CEXValue[0] || y != I2CEXValue[1])
        {
            first = I2CEXValue[0] ^ 0xff;
            first = first & x;
            second = I2CEXValue[1] ^ 0xff;
            second = second & y;

            I2CEXValue[0] = x;
            I2CEXValue[1] = y;

            //printf("%3d %3d %3d %3d\n", x, y, first, second);
            for(ForCount = 0; ForCount < 8; ++ForCount)
            {
                if (ForCount == 0 && InterruptCount[ForCount] == 0  && (first & 1) == 0 )
                {
                    InterruptCount[ForCount] = 1;
                }else if((ForCount == 2 || ForCount == 3) && InterruptCount[ForCount] == 0 &&(first & 1) == 1)
                {
                    InterruptCount[ForCount] = 1;
                }else if(ForCount > 3 && ForCount != 5 && (second & 1) == 1 )
                {
                    PINCount[0][ForCount - 4] = PINCount[0][ForCount - 4] + InterruptCount[ForCount - 4];
                    InterruptCount[ForCount - 4] = 0;
                }else if (ForCount == 5 && (second & 1)== 1 )
                {
                    //a hard code
                    int tempForForCount1 = (x >> 1);
                    tempForForCount1 = (tempForForCount1 & 1);
                    PINCount[0][1] = PINCount[0][1]+tempForForCount1;
                }else;
                //PINCount[0][ForCount] = PINCount[0][ForCount] + (first & 1);
                first = first >> 1;
                //PINCount[1][ForCount] = PINCount[1][ForCount] + (second & 1);
                second = second >> 1;
            }
#ifdef PrintInfo
            printf("reader 1: %3d, %3d | %ld %ld %ld %ld \n",x , y, PINCount[0][0], PINCount[0][1], PINCount[0][2], PINCount[0][3] );
#endif
        }
    }
#ifdef LogMode 
    Log(s, __func__, __LINE__, " exit\n");
#endif

}

void * RemoteControl(void * argument)
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
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)))
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
    /*if(close(socket_desc) < 0)
    {
        printf("erron %d",errno);
    }*/
    close(socket_desc);
    //free(socket_desc);
#ifdef PrintInfo
    printf("Telnet close\n"); 
#endif
}

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
    fprintf(pfile,"m2400\t%s\t0\t", buff);
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
                    fprintf(pfile,"m2400\t%s\t%ld\t",buff,productCountArray[Good]);
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
    short StageFlag = 0;


    if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
    {
        printf ("Unable to open serial device: %s\n", strerror (errno)) ;
        pthread_exit((void*)"fail");
    }
    /*if (wiringPiSetup () == -1)
    {
        printf ("Unable to start wiringPi: %s\n", strerror (errno)) ;
        pthread_exit((void*)"fail");
    }*/
    
    while(SerialThreadFlag)
    {
        /*int www = serialDataAvail(fd);
        if(www != 0){
        printf("www: %d\n", www);
        }*/
        while(serialDataAvail(fd))
        {   
            char temp_char_1;
            temp_char_1 = serialGetchar(fd);
            if(temp_char_1 == 0x74 || temp_char_1 == 0x76)
            {
                StageFlag = 1;
                fflush (stdout) ;
                continue;
            }
            if(StageFlag == 1)
            {
               StageFlag = 0;
            }
            else if( string_count < RS232_Length)
            {
               //printf("%x ", temp_char_1);
               if(temp_char_1 == 0x72 || temp_char_1 == 0x64) continue; //72 接 計數 所以直接pass 64 為 reset 來太快
               pthread_mutex_lock(&mutex_2);
               output[string_count]= temp_char_1;
               string_count++;
               updateFlag = 1;
               pthread_mutex_unlock(&mutex_2);
               StageFlag = 0;
            }
            else;//{ printf("\n--------\n");};
            fflush (stdout) ;
            //if (SerialThreadFlag == 0) break;*/
        }
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, "Serial Function exit\n");
#endif
}

void * FileFunction(void *argement)
{
#ifdef LogMode
    Log(s, __func__, __LINE__, "File Function entry\n");
#endif
    struct timeval now;
    struct timespec outtime;
    FILE *file_dst;
    int file_output_temp_length = 0;
    char file_output_temp[RS232_Length];

    int fd;
    int WriteFileCount = 0;
    struct ifreq ifr;
    
    while(FileFlag)
    {
        pthread_mutex_lock(&mutex);
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
        ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);
 
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + CountPeriod;
        outtime.tv_nsec= now.tv_usec * 1000;

        pthread_cond_timedwait(&cond, &mutex, &outtime);
        pthread_mutex_unlock(&mutex);
        if(updateFlag == 1)
        {
            int vers_count;

            pthread_mutex_lock(&mutex_2);

            file_output_temp_length = string_count;
            string_count = 0;
            memcpy(file_output_temp, output, file_output_temp_length); 
            for(vers_count = 0; vers_count < RS232_Length; vers_count++)
            {
                //printf("%x ", output[vers_count]); 
                file_output_temp[vers_count] = output[vers_count];
                
            }
            //printf("\n");
            updateFlag = 0;
            memset(output, 0 , sizeof(char)*RS232_Length);
            pthread_mutex_unlock(&mutex_2);
            //printf(" %d %d\n", string_count, file_output_temp_length);
            for(vers_count = 0; vers_count < file_output_temp_length ; vers_count++)
            {
                char output_char = file_output_temp[vers_count];
                //printf("%x ",output_char);
                switch(output_char)
                {
                     case 0x62:
                         productCountArray[Stop] = productCountArray[Stop] + 1;
                         break;
                     case 0x64:
                         //productCountArray[Reset] = productCountArray[Reset] + 1;
                         break;
                     case 0x6a:
                         productCountArray[Insert] = productCountArray[Insert] + 1;
                         break;
                     case 0x68:
                         productCountArray[Good] = productCountArray[Good] + 1;
                         //productCountArray[Total] = productCountArray[Total] + 1;
                         break;
                     case 0x86:
                         messageArray[Position] = messageArray[Position] + 1;  
                         break;
                     case 0xB2:
                         messageArray[Bad_C] = messageArray[Bad_C] + 1;
                         break;
                     case 0xAC:
                         messageArray[Bad_B] = messageArray[Bad_B] + 1;
                         break;
                     case 0xAA:
                         messageArray[Bad_D] = messageArray[Bad_D] + 1;
                         break;
                     case 0xAE:
                         messageArray[Tube] = messageArray[Tube] + 1;
                         break;
                     case 0x98:
                         messageArray[Left] = messageArray[Left] + 1;
                         break;
                     case 0xA4:
                         messageArray[Bad_A] = messageArray[Bad_A] + 1;
                         break;
                     case 0xB0:
                         messageArray[ExportWhite] = messageArray[ExportWhite] + 1;
                         break;
                     case 0xA2:
                         messageArray[InsertError] = messageArray[InsertError] + 1;
                         break;
                     case 0x82:
                         messageArray[FrontBreak] = messageArray[FrontBreak] + 1;
                         break;
                     case 0x94:
                         messageArray[BackBreak] = messageArray[BackBreak] + 1;
                         break;
                     case 0xA6:
                         messageArray[Shell] = messageArray[Shell] + 1;
                         break;
                     case 0x8A:
                         messageArray[Vacuum] = messageArray[Vacuum] + 1;
                         break;
                     case 0x88:
                         messageArray[Liquid] = messageArray[Liquid] + 1;
                         break;
                     case 0xA0:
                         messageArray[Eraser] = messageArray[Eraser] + 1;
                         break;
                     case 0xA8:
                         messageArray[Seal] =messageArray[Seal] + 1;
                         break;
                     default:
                         //file_dst = fopen("DataLog", "a");
                         //fprintf(file_dst,"0x%x\n",output_char);
                         //fclose(file_dst);
                         break;
               } 

            }
            memset(file_output_temp,0, sizeof(char)*RS232_Length);

#ifdef PrintInfo
            printf("Good:%ld Insert:%ld || %ld %ld %ld %ld\n",productCountArray[Good],productCountArray[Insert], 
                                                             PINCount[0][0], PINCount[0][1], PINCount[0][2], PINCount[0][3]);
            
            for(vers_count = 0; vers_count < EventCount; vers_count++)
            {
                 printf("%d : %ld| ", vers_count, messageArray[vers_count]);
            }
            printf("\n");
#endif
       }
       WriteFileCount = (WriteFileCount + CountPeriod) % WriteFileCountValue;
       if(WriteFileCount == 0 || FileFlag == 0)
       {
            pthread_mutex_lock(&mutexFile);
            file_dst = fopen(UPLoadFile, "a");
            int ForCount = 0, ForCount2 = 0;
            for(ForCount = 0; ForCount < ProductCount ; ForCount++)
            {
                if(ExproductCountArray[ForCount] != productCountArray[ForCount] && ForCount == 0)
                {
                    fprintf(file_dst, "%s\t%s\t%s\t%ld\t%ld\t0\t%s\t%d\t%s\t%s\n", ISNo, ManagerCard, CountNo, productCountArray[Good], (long)now.tv_sec,
                                                                                   inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                                   ForCount+1, MachineCode, UserNo);
                }
                else if(ExproductCountArray[ForCount] != productCountArray[ForCount])
                {
                    fprintf(file_dst, "%s\t%s\t%s\t%ld\t%ld\t%ld\t%s\t%d\t%s\t%s\n", ISNo, ManagerCard, CountNo, productCountArray[Good], (long)now.tv_sec,
                                                                                   productCountArray[ForCount],
                                                                                   inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                                   ForCount+1, MachineCode, UserNo);
                }
                else;
            }
            for(ForCount = 0; ForCount < EventCount; ForCount++)
            {
                if(ExmessageArray[ForCount] != messageArray[ForCount])
                {
                     fprintf(file_dst, "%s\t%s\t%s\t%ld\t%ld\t%ld\t%s\t%d\t%s\t%s\n", ISNo, ManagerCard, CountNo, productCountArray[Good], (long)now.tv_sec,
                                                                                   messageArray[ForCount],
                                                                                   inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                                   ForCount+ProductCount+1, MachineCode, UserNo);
                }
            }
            for(ForCount = 0; ForCount < 1 ; ForCount++)
            {
                for(ForCount2 = 0 ; ForCount2 < 4; ForCount2++)
                {
                    if(PINEXCount[ForCount][ForCount2] != PINCount[ForCount][ForCount2])
                    {
                        fprintf(file_dst, "%s\t%s\t%s\t%ld\t%ld\t%ld\t%s\t%d\t%s\t%s\n", ISNo, ManagerCard, CountNo, productCountArray[Good], (long)now.tv_sec,
                                                                                         PINCount[ForCount][ForCount2],
                                                                                         inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                                      ForCount * 8 + ForCount2 +(ProductCount+EventCount+1) , MachineCode, UserNo);
                    }
                }
            } 
            fclose(file_dst);
            pthread_mutex_unlock(&mutexFile);
            memcpy(ExproductCountArray, productCountArray, sizeof(long)*ProductCount);
            memcpy(ExmessageArray, messageArray, sizeof(long)*EventCount);
            memcpy(PINEXCount, PINCount, sizeof(long)*16);
       }
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, "FileFunction exit\n");
#endif
}

int main(int argc ,char *argv[])
{
    char *dev = "/dev/i2c-1";
    int rc;    
    pthread_t LogThread, SerialThread, FileThread, TelnetControlThread, InterruptThread, FTPThread;
    int shmid;
    key_t key;

    //[vers| add for fake barcode input]
    char FakeInput[5][InputLength];
    FILE *pfile;
    char *buffer, *charPosition;
    short FlagNo = 0;
    //[vers| end]

    //Log(str, "part_1",__FILE__, __func__,__LINE__, "");

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_2, NULL);
    //pthread_mutex_init(&mutex_3, NULL);
    pthread_mutex_init(&mutex_log, NULL);
    pthread_mutex_init(&mutexFTP, NULL);
    pthread_mutex_init(&mutexFile, NULL);
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
    char tempString[InputLength], *tempPtr;
    struct timeval now;
   
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
    Log(s, __func__, __LINE__, " ready to init\n");
#endif
    //the mechine always standby
    while(1)
    {
        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, HIGH);
        
        /*
        while(1)
        {
            sleep(1);
            memset(tempString, 0, sizeof(char)* InputLength);
            gets(tempString);
            if(strncmp(tempString, "YYY", 3) == 0)
            {
                memset(ISNo, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(ISNo, tempPtr, sizeof(tempString)-2);
                // ready for light some led;
                digitalWrite(WiringPiPIN_15, LOW);
                digitalWrite(WiringPiPIN_16, HIGH);
                digitalWrite(WiringPiPIN_18, HIGH);
                break;
            }
            printf("scan ISNo error code\n");
        }
        while(1)
        {
            sleep(1);
            memset(tempString, 0, sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "QQQ", 3) == 0)
            {
                memset(ManagerCard, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(ManagerCard, tempPtr, sizeof(tempString)-2);
                digitalWrite (WiringPiPIN_15, HIGH);
                digitalWrite (WiringPiPIN_16, LOW);
                digitalWrite (WiringPiPIN_18, HIGH);
                break;
            }
            printf("ManagerCard scan error code\n");
        }
        while(1)
        {
            sleep(1);
            memset(tempString, 0 , sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "ZZZ", 3) == 0)
            {
                memset(MachineCode, 0 , sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(MachineCode, tempPtr, sizeof(tempString)-2);
                digitalWrite (WiringPiPIN_15, LOW);
                digitalWrite (WiringPiPIN_16, LOW);
                digitalWrite (WiringPiPIN_18, HIGH);
                break;
            }
            printf("MachineCode scan error code\n");
        }
        while(1)
        {
            sleep(1);
            memset(tempString, 0, sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "WWW", 3) == 0)
            {
                memset(CountNo, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(CountNo, tempPtr, sizeof(tempString)-2);
                digitalWrite (WiringPiPIN_15, HIGH);
                digitalWrite (WiringPiPIN_16, HIGH);
                digitalWrite (WiringPiPIN_18, LOW);
                break;
            }
            printf("CountNo scan error code\n");
        } 
 
        while(1)
        {
            sleep(1);
            memset(tempString, 0, sizeof(char)*InputLength);
            gets(tempString);
            if(strncmp(tempString, "XXX", 3) == 0)
            {
                memset(UserNo, 0, sizeof(char)*InputLength);
                tempPtr = tempString + 3;
                memcpy(UserNo, tempPtr, sizeof(tempString)-2);
                digitalWrite (WiringPiPIN_15, LOW);
                digitalWrite (WiringPiPIN_16, HIGH);
                digitalWrite (WiringPiPIN_18, LOW);
                break;
            }
            printf("UserNo scan error code\n");
        }
       */
        //[vers| add for fake barcode input]
        int FileSize, FakeInputNumber_1 = 0, FakeInputNumber_2 = 0;
        memset(FakeInput, 0, sizeof(char)*(5*InputLength));
        pfile = fopen("/home/pi/works/m2400/barcode", "r");
        fseek(pfile, 0, SEEK_END);
        FileSize = ftell(pfile);
        rewind(pfile);
        buffer = (char *)malloc(sizeof(char)*FileSize);
        charPosition = buffer;
        fread(buffer, 1, FileSize, pfile);
        fclose(pfile); 

        while(FileSize > 1)
        {
            if(*charPosition == ' ')
            {
                FlagNo = 1;
            }
            else if(*charPosition != ' ' && FlagNo == 1)
            {
                FakeInputNumber_1++;
                FakeInputNumber_2 = 0;
                FakeInput[FakeInputNumber_1][FakeInputNumber_2] = *charPosition;
                FakeInputNumber_2++;
                FlagNo = 0;
            }
            else
            {
                FakeInput[FakeInputNumber_1][FakeInputNumber_2] = *charPosition;
                FakeInputNumber_2++;
            }
            FileSize--;
            charPosition++;
        }
        free(buffer);
        //[vers| end]
        sleep(1);
        memset(ISNo, 0, sizeof(char)*InputLength);
        //[vers| add for fake barcode input]
        //strcpy(ISNo, "01");
        strcpy(ISNo, FakeInput[0]); 
        //[vers|end]
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, HIGH);
        sleep(1);

        memset(ManagerCard, 0, sizeof(char)*InputLength);
        //[vers|add for fake barcode input]
        //strcpy(ManagerCard, "BL20G58");
        strcpy(ManagerCard, FakeInput[1]);
        //[vers|end]
        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, LOW);
        digitalWrite (WiringPiPIN_18, HIGH);
        sleep(1);

        memset(MachineCode, 0 , sizeof(char)*InputLength);
        //[vers|add for fake barcode input]
        //strcpy(MachineCode, "G58");
        strcpy(MachineCode, FakeInput[2]);
        //[vers|end]
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, LOW);
        digitalWrite (WiringPiPIN_18, HIGH);
        sleep(1);

        memset(CountNo, 0, sizeof(char)*InputLength);
        //[vers|add for fake barcode input]
        //strcpy(CountNo, "1000000");
        strcpy(CountNo, FakeInput[3]);
        //[vers|end]
        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, LOW);
        sleep(1);

        memset(UserNo, 0, sizeof(char)*InputLength);
        //[vers|add for fake barcode input]
        //strcpy(UserNo, "6957");
        strcpy(UserNo, FakeInput[4]);
        //[vers|end]
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, LOW);
        sleep(1);

        memset(UPLoadFile, 0, sizeof(char)*UPLoadFileLength);
        gettimeofday(&now, NULL);
        sprintf(UPLoadFile,"%ld%s.txt",(long)now.tv_sec, MachineCode); 
        
        printf("%s %s %s %s %s %s\n", ISNo, ManagerCard, MachineCode, UserNo, CountNo, UPLoadFile);
 
        MasterFlag = 1;
        
        memset(ExproductCountArray, 0, sizeof(long)*ProductCount);
        memset(productCountArray, 0, sizeof(long)*ProductCount);
        memset(ExmessageArray, 0, sizeof(long)*EventCount);
        memset(messageArray, 0, sizeof(long)*EventCount);
        memset(InterruptCount, 0 , sizeof(short)*4); 
        memset(PINCount, 0, sizeof(long)*16);
        memset(PINEXCount, 0, sizeof(long)*16); 

        if(zhTelnetFlag == 0){
            //zhTelnetFlag = 1;
            //rc = pthread_create(&TelnetControlThread, NULL, RemoteControl, NULL);
            //assert(rc == 0);
        }

        while(MasterFlag)
        {
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
            i2c_smbus_write_byte_data(fd, INV_P0, 0x0f);
            i2c_smbus_write_byte_data(fd, CONFIG_P0, 0x0f);

            i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
            i2c_smbus_write_byte_data(fd, INV_P1, 0xf0);
            i2c_smbus_write_byte_data(fd, CONFIG_P1, 0xf0); 

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
            i2c_smbus_write_byte_data(fd, OUT_P1, 0x00);
            i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
            close(fd);
      
            PrintLeftLog = 0;
            zhResetFlag = 0; //reset flag clean

            InterruptEnable = 1;
            rc = pthread_create(&InterruptThread , NULL, zhINTERRUPT1,NULL);
            assert(rc == 0);
            
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
                sleep(1);
                if(productCountArray[Good]  >= atoi(CountNo))
                //if(productCountArray[Good]  >= 0)
                {
                    //finish job
                    printf("Houston we are ready to back!\n");
                    zhResetFlag = 1;
                    MasterFlag = 0;
                }
/*              else  if(digitalRead(WiringPiPIN_22) == 1)
                {
                    //finish job
#ifdef LogMode
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
#endif
                    printf("Houston PIN_22 are ask us  back!\n");
                    zhResetFlag = 1;
                    MasterFlag = 0;
                }
*/
                /*else if(zhTelnetFlag == 0)
                {
                    printf("Houston remote user call me back to base!\n");
#ifdef LogMode
                    Log(s, __func__, __LINE__, " remote user call me back\n");
                    Log(s, __func__, __LINE__, " remote user call me back\n");
                    Log(s, __func__, __LINE__, " remote user call me back\n");
#endif
                    pthread_join(TelnetControlThread, NULL);
                    zhResetFlag = 1;
                    MasterFlag = 0;
                }*/
                else;
            }
   
            PrintLeftLog = 1;
            InterruptEnable = 0;
            pthread_join(InterruptThread, NULL);
            sleep(1); 

            FileFlag = 0;
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            pthread_join(FileThread, NULL);
            sleep(1);

            SerialThreadFlag = 0;
            pthread_join(SerialThread, NULL);
            sleep(1);

            FTPFlag = 0;
            pthread_mutex_lock(&mutexFTP);
            pthread_cond_signal(&condFTP);
            pthread_mutex_unlock(&mutexFTP);
            pthread_join(FTPThread, NULL);
            sleep(1);


            //vers end
            
            /*if(MasterFlag)
            {
                digitalWrite (WiringPiPIN_15, HIGH);
                digitalWrite (WiringPiPIN_16, HIGH);
                digitalWrite (WiringPiPIN_18, LOW);

                while(1)
                {
                    sleep(1);
                    memset(tempString, 0, sizeof(char)*InputLength);
                    gets(tempString);
                    if(strncmp(tempString, "XXX", 3) == 0)
                    {
                        memset(UserNo, 0, sizeof(char)*InputLength);
                        tempPtr = tempString + 3;
                        memcpy(UserNo, tempPtr, sizeof(tempString)-2);
                        digitalWrite (WiringPiPIN_15, LOW);
                        digitalWrite (WiringPiPIN_16, HIGH);
                        digitalWrite (WiringPiPIN_18, LOW);

                        break;
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
    CURL *curl;
    CURLcode res;
    FILE *hd_src;
    struct stat file_info, file_info_2;
    curl_off_t fsize;
    char UPLoadFile_3[21];
    struct timeval now;
    struct timespec outtime;
    int FTPCount = 0;

    while(FTPFlag){
        char Remote_url[80] = "ftp://192.168.10.254:21/home/";
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
        pthread_mutex_lock(&mutexFile);
        if(!stat(UPLoadFile, &file_info_2))
        {
            size = file_info_2.st_size;
            //printf("size:%ld\n", size);
        }
        pthread_mutex_unlock(&mutexFile);

        if(FTPCount == 0 || FTPFlag == 0 || size > 100000)
        {
            pthread_mutex_lock(&mutexFile);
            memset(UPLoadFile_3, 0, sizeof(char)*21);
            strcpy(UPLoadFile_3, UPLoadFile);
            gettimeofday(&now, NULL);
            sprintf(UPLoadFile,"%ld%s.txt",(long)now.tv_sec, MachineCode);
            pthread_mutex_unlock(&mutexFile);

            printf("%s\n", UPLoadFile_3);
            strcat(Remote_url,UPLoadFile_3);
            if(stat(UPLoadFile_3, &file_info)) {
                printf("Couldnt open %s: %s\n", UPLoadFile_3, strerror(errno));
#ifdef LogMode
                Log(s, __func__, __LINE__, " FTP fail_1\n");
#endif
                digitalWrite (WiringPiPIN_15, LOW);
                digitalWrite (WiringPiPIN_16, LOW);
                digitalWrite (WiringPiPIN_18, LOW);
            }
            if(file_info.st_size > 0)
            {
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
            }
            unlink(UPLoadFile_3);
        }
    }
#ifdef LogMode
    Log(s, __func__, __LINE__, " FTP exit\n");
#endif
}


int transferFormatINT(char x)
{
    //int ans = ((int)x /16) * 10 + ((int) x % 16);
    int ans = (int)x;
    return ans;
}
long transferFormatLONG(char x)
{
    long ans = (long)x;
    return ans;
}
