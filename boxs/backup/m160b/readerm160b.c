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

#define InputLength 20
#define UPLoadFileLength 21
#define FTPPeriod 4
#define FilePeriod 300 //unit:ms
#define FTPCountValue 300
#define ProductCount 8
#define RS232_Length 60

#define Log(s,func, line, opt) StringCat(func);StringCat(opt)


//#define LogMode
#define PrintInfo

enum
{
    GoodAccumulate = 0,
    PostiveWire,
    NegtiveWire,
    Good,
    RollingBad,
    BodyBad,
    ShortBad,
    ForceOutput
};


int SerialThreadFlag = 0;
int FTPFlag = 0;
short zhResetFlag = 0;
int PrintLeftLog = 0;
int FileFlag = 0;
int updateFlag = 0;
short zhTelnetFlag = 0;

char *shm, *s, *tail;
char *shm_pop;
char output[RS232_Length];

pthread_cond_t cond, cond_2;
pthread_mutex_t mutex, mutex_log, mutex_2, mutex_ftp, mutexFile;

long ExProductCountArray[ProductCount];
long ExzhError[4][16];

char ISNo[InputLength], ManagerCard[InputLength], MachineCode[InputLength], UserNo[InputLength], CountNo[InputLength];
char UPLoadFile[UPLoadFileLength];

void * zhLogFunction(void *argument);
void * FTPFunction(void *arguemnt);
void StringCat(const char *str);
void * SerialFunction(void *argument);
void * RemoteControl(void *argument);

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
long transferFormat(char w, char x, char y, char z);

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
    fprintf(pfile,"m2600\t%s\t0\t", buff);
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
                    fprintf(pfile,"m160b\t%s\t%ld\t",buff,ExProductCountArray[4]);
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
    char temp_output[RS232_Length];
    int string_count = 0;

    memset(temp_output, 0, sizeof(char)*RS232_Length);

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
        while(serialDataAvail(fd))
        {   
            char temp_char_1;
            temp_char_1 = serialGetchar(fd);
            temp_output[string_count] = temp_char_1;
            string_count++;
            if(string_count >= RS232_Length )
            {
                string_count = 0;
                memset(temp_output, 0, sizeof(char)*RS232_Length);
            }
            else if(temp_output[string_count - 1] == 0x0a && temp_output[string_count - 2] == 0x0d && temp_output[string_count - 3] == 0x03)
            {
                //printf("%d\n", string_count);
                if ((string_count == 58 || string_count == 26))
                {
                    int cpStringCount = 0;
    		    pthread_mutex_lock(&mutex_2);
                    memset(output, 0, sizeof(char)*RS232_Length);
                    //printf("send length %d\n", string_count);
                    for(cpStringCount = 0; cpStringCount < string_count; cpStringCount++)
                    {
                        output[cpStringCount] = temp_output[cpStringCount];
                        //printf("%2x ", temp_output[cpStringCount]);
                    }
                    //printf("\n");
                    updateFlag = 1;
                    pthread_mutex_unlock(&mutex_2);
                }
                memset(temp_output, 0, sizeof(char)*RS232_Length);
                string_count = 0;
            }
            else;
            fflush (stdout) ;
            //if (SerialThreadFlag == 0) break;
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
    char file_output_temp[RS232_Length];
    long ProductCountArray[ProductCount];
    long zhError[4][16]; //counting
    char charLastError[4][16]; //last state

    int fd;
    struct ifreq ifr;
    
    memset(zhError, 0, sizeof(long)*64);
    memset(charLastError, 0, sizeof(char)*64);
    memset(ProductCountArray, 0, sizeof(long)*ProductCount);
    
    while(FileFlag)
    {
        pthread_mutex_lock(&mutexFile);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec;
        outtime.tv_nsec = (now.tv_usec + FilePeriod * 1000) * 1000;
 
        if(outtime.tv_nsec > 1000000000)
        {
            outtime.tv_sec +=1;
            outtime.tv_nsec = outtime.tv_nsec % 1000000000;
        }

        pthread_cond_timedwait(&cond_2, &mutexFile, &outtime);
        pthread_mutex_unlock(&mutexFile);
        //gettimeofday(&now, NULL);
        //printf("%ld.%ld\n", now.tv_sec, now.tv_usec);

        if(updateFlag == 1)
        {
            int vers_count;
            
            fd = socket(AF_INET, SOCK_DGRAM, 0);
            ifr.ifr_addr.sa_family = AF_INET;
            strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
            ioctl(fd, SIOCGIFADDR, &ifr);
            close(fd);
            memset(file_output_temp, 0, sizeof(char)*RS232_Length);

            pthread_mutex_lock(&mutex_2);
            //strncpy(file_output_temp, output, RS232_Length);
            for(vers_count = 0; vers_count < RS232_Length; vers_count++)
            {
                file_output_temp[vers_count] = output[vers_count];
            }
            updateFlag = 0;
            pthread_mutex_unlock(&mutex_2);
            if(strlen(file_output_temp) == 58)
            {
                if(file_output_temp[7] == 0x49)
                {
                   ProductCountArray[0] = transferFormat(file_output_temp[22], file_output_temp[23], file_output_temp[24], file_output_temp[25]); 
                   ProductCountArray[1] = transferFormat(file_output_temp[26], file_output_temp[27], file_output_temp[28], file_output_temp[29]); 
                   ProductCountArray[2] = transferFormat(file_output_temp[30], file_output_temp[31], file_output_temp[32], file_output_temp[33]); 
                   ProductCountArray[3] = transferFormat(file_output_temp[34], file_output_temp[35], file_output_temp[36], file_output_temp[37]); 
                   ProductCountArray[4] = transferFormat(file_output_temp[38], file_output_temp[39], file_output_temp[40], file_output_temp[41]); 
                   ProductCountArray[5] = transferFormat(file_output_temp[42], file_output_temp[43], file_output_temp[44], file_output_temp[45]); 
                   ProductCountArray[6] = transferFormat(file_output_temp[47], file_output_temp[48], file_output_temp[49], file_output_temp[50]); 
                   ProductCountArray[7] = transferFormat(file_output_temp[51], file_output_temp[51], file_output_temp[52], file_output_temp[53]); 
                }
            }
            else if(strlen(file_output_temp) == 26)
            {
                if(file_output_temp[7] == 0x4d && file_output_temp[22] == 0x4e)
                {
                    int errorNumber = (int)file_output_temp[15];
                    //printf("errorNumber : %d\n", errorNumber);
                    if (errorNumber >= 65)
                    {
                        errorNumber = errorNumber - 55; 
                    }
                    else
                    {
                        errorNumber = errorNumber - 48;
                    }
                    if(file_output_temp[15] == 0x36 && charLastError[0][errorNumber] == 0x00)
                    {
                        zhError[0][errorNumber]++;
                        charLastError[0][errorNumber] = 0x01;
                    }
                    else if(file_output_temp[15] == 0x37 && charLastError[1][errorNumber] == 0x00)
                    {
                        zhError[1][errorNumber]++;
                        charLastError[1][errorNumber] = 0x01;
                    }
                    else if(file_output_temp[15] == 0x38 && charLastError[2][errorNumber] == 0x00)
                    {
                        zhError[2][errorNumber]++;
                        charLastError[2][errorNumber] = 0x01;
                    }
                    else if(file_output_temp[15] == 0x39 && charLastError[3][errorNumber] == 0x00)
                    {
                        zhError[3][errorNumber]++;
                        charLastError[3][errorNumber] = 0x01;
                    }
                    else;
                }
                else if(file_output_temp[7] == 0x4d && file_output_temp[22] == 0x46)
                {
                    int errorNumber = (int)file_output_temp[15];
                    if (errorNumber >= 65)
                    { 
                        errorNumber = errorNumber - 55;
                    }
                    else
                    {
                        errorNumber = errorNumber - 48;
                    }
                    
                    if(file_output_temp[15] == 0x36)
                    {
                        charLastError[0][errorNumber] = 0x00;
                    }else if(file_output_temp[15] == 0x37)
                    {
                        charLastError[1][errorNumber] = 0x00;
                    }else if(file_output_temp[15] == 0x38)
                    {
                        charLastError[2][errorNumber] = 0x00;
                    }else
                    {
                        charLastError[3][errorNumber] = 0x00;
                    }
                }
                else;
            }
            else;
            
            pthread_mutex_lock(&mutex_ftp); 
            file_dst = fopen(UPLoadFile, "a");
            gettimeofday(&now, NULL);
            int ForCount,ForCount2;
            for(ForCount = 0; ForCount < ProductCount; ForCount++)
            {
                if(ProductCountArray[ForCount] != ExProductCountArray[ForCount] && ForCount == 4)
                {
                    fprintf(file_dst, "%s\t%s\t%s\t%ld\t%ld\t0\t%s\t%d\t%s\t%s\n", ISNo, ManagerCard, CountNo, ProductCountArray[4], (long)now.tv_sec,
                                                                                   inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                                   ForCount+1, MachineCode, UserNo);
                    ExProductCountArray[ForCount] = ProductCountArray[ForCount];
#ifdef PrintInfo
                    //printf("ExProductCountArray %d: %ld\n", ForCount, ExProductCountArray[ForCount]);
#endif
                }
                else if(ProductCountArray[ForCount] != ExProductCountArray[ForCount])
                {
                    fprintf(file_dst, "%s\t%s\t%s\t%ld\t%ld\t%ld\t%s\t%d\t%s\t%s\n", ISNo, ManagerCard, CountNo, ProductCountArray[4], (long)now.tv_sec,
                                                                                   ProductCountArray[ForCount],
                                                                                   inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                                   ForCount+1, MachineCode, UserNo);
                    ExProductCountArray[ForCount] = ProductCountArray[ForCount];
#ifdef PrintInfo
                    //printf("ExProductCountArray %d: %ld\n", ForCount, ExProductCountArray[ForCount]); 
#endif
                }
                else;
            }
#ifdef PrintInfo
            //printf("\n");
#endif
            for(ForCount = 0; ForCount < 4; ForCount++)
            {
                for(ForCount2 = 0; ForCount2 < 16; ForCount2++)
                {
                    if(zhError[ForCount][ForCount2] != ExzhError[ForCount][ForCount2])
                    {
                      fprintf(file_dst, "%s\t%s\t%s\t%ld\t%ld\t%ld\t%s\t%d\t%s\t%s\n", ISNo, ManagerCard, CountNo, ProductCountArray[4], (long)now.tv_sec,
                                                                                   zhError[ForCount][ForCount2],
                                                                                   inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                                   (ForCount+1)*16+ForCount2+ProductCount, MachineCode, UserNo);
                       ExzhError[ForCount][ForCount2] = zhError[ForCount][ForCount2];
#ifdef PrintInfo
                       //printf("errorNumber %d : %ld\n", ForCount*ForCount2+ProductCount, ExzhError[ForCount][ForCount2]);
#endif
                    }
                }
            }
            fclose(file_dst);
            pthread_mutex_unlock(&mutex_ftp);
            printf("%ld %ld %ld %ld %ld %ld %ld %ld || %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld || %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld || %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld || %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",                
             ProductCountArray[0], ProductCountArray[1], ProductCountArray[2], ProductCountArray[3], ProductCountArray[4],
             ProductCountArray[5], ProductCountArray[6], ProductCountArray[7],
             zhError[0][0], zhError[0][1], zhError[0][2], zhError[0][3], zhError[0][4], zhError[0][5], zhError[0][6], zhError[0][7],
             zhError[0][8], zhError[0][9], zhError[0][10], zhError[0][11], zhError[0][12], zhError[0][13], zhError[0][14], zhError[0][15],
             zhError[1][0], zhError[1][1], zhError[1][2], zhError[1][3], zhError[1][4], zhError[0][6], zhError[1][6], zhError[1][7],
             zhError[1][8], zhError[1][9], zhError[1][10], zhError[1][11], zhError[1][12], zhError[1][13], zhError[1][14], zhError[1][15],
             zhError[2][0], zhError[2][1], zhError[2][2], zhError[2][3], zhError[2][4], zhError[2][5], zhError[2][6], zhError[2][7],
             zhError[2][8], zhError[2][9], zhError[2][10], zhError[2][11], zhError[2][12], zhError[2][13], zhError[2][14], zhError[2][15],
             zhError[3][0], zhError[3][1], zhError[3][2], zhError[3][3], zhError[3][4], zhError[3][6], zhError[3][6], zhError[3][7],
             zhError[3][8], zhError[3][9], zhError[3][10], zhError[3][11], zhError[3][12], zhError[3][13], zhError[3][14], zhError[3][15]);
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
    pthread_t LogThread, SerialThread, FileThread, FTPThread, TelnetControlThread;
    
    //[vers|add for fake barcode input]
    char FakeInput[5][InputLength];
    FILE *pfile;
    char *buffer, *charPosition;
    short FlagNo = 0;
    //[end]
    int shmid;
    key_t key;

    //Log(str, "part_1",__FILE__, __func__,__LINE__, "");

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_2, NULL);
    pthread_mutex_init(&mutexFile, NULL);
    pthread_mutex_init(&mutex_log, NULL);
    pthread_mutex_init(&mutex_ftp, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond_2, NULL);

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
        i2c_smbus_write_byte_data(fd, OUT_P1, 0x01);
        i2c_smbus_write_byte_data(fd, CONFIG_P1, 0x00);
        close(fd);*/

        //3rd i3c board will control 3*8 control
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
                digitalWrite(WiringPiPIN_15, HIGH);
                digitalWrite(WiringPiPIN_16, LOW);
                digitalWrite(WiringPiPIN_18, LOW);
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
                digitalWrite (WiringPiPIN_15, LOW);
                digitalWrite (WiringPiPIN_16, HIGH);
                digitalWrite (WiringPiPIN_18, LOW);
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
                digitalWrite (WiringPiPIN_15, HIGH);
                digitalWrite (WiringPiPIN_16, HIGH);
                digitalWrite (WiringPiPIN_18, LOW);
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
                digitalWrite (WiringPiPIN_15, LOW);
                digitalWrite (WiringPiPIN_16, LOW);
                digitalWrite (WiringPiPIN_18, HIGH);
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
                digitalWrite (WiringPiPIN_15, HIGH);
                digitalWrite (WiringPiPIN_16, LOW);
                digitalWrite (WiringPiPIN_18, HIGH);
                break;
            }
            printf("UserNo scan error code\n");
        }
       */
        //[vers| add for fake barcode input]
        int FileSize, FakeInputNumber_1 = 0, FakeInputNumber_2 = 0;
        memset(FakeInput, 0, sizeof(char)*(5*InputLength));
        pfile = fopen("/home/pi/works/m160b/barcode", "r");
        fseek(pfile, 0,SEEK_END);
        FileSize = ftell(pfile);
        rewind(pfile);
        buffer = (char *) malloc(sizeof(char)*FileSize);
        charPosition = buffer;
        fread(buffer, 1, FileSize, pfile);
        fclose(pfile);
        while(FileSize > 1)
        {
            if(*charPosition == ' ')
            {
                 FlagNo = 1;
            }
            else if(*charPosition != ' ' && FlagNo == 1) //2nd ~ n unit
            {
                FakeInputNumber_1++;
                FakeInputNumber_2 = 0;
                FakeInput[FakeInputNumber_1][FakeInputNumber_2] = *charPosition;
                FakeInputNumber_2++;
                FlagNo = 0;
            }else //1st unit
            {
                FakeInput[FakeInputNumber_1][FakeInputNumber_2] = *charPosition;
                FakeInputNumber_2++;
            }
            FileSize--;
            charPosition++;
        }
 
        free(buffer);
        //[vers|end]
      
        sleep(1);
        memset(ISNo, 0, sizeof(char)*InputLength);
        //[vers|add for fake barcode input]
        //strcpy(ISNo, "01");
        strcpy(ISNo, FakeInput[0]);
        //[vers|end]
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, HIGH);
        sleep(1);

        memset(ManagerCard, 0, sizeof(char)*InputLength);
        //[vers|add for fake barcode input]
        //strcpy(ManagerCard, "BL20E88");
        strcpy(ManagerCard, FakeInput[1]);
        //[vers|end]
        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, LOW);
        digitalWrite (WiringPiPIN_18, HIGH);
        sleep(1);

        memset(MachineCode, 0 , sizeof(char)*InputLength);
        //[vers| add for fake barcode input]
        //strcpy(MachineCode, "E88");
        strcpy(MachineCode, FakeInput[2]);
        //[vers|end]
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, LOW);
        digitalWrite (WiringPiPIN_18, HIGH);
        sleep(1);

        memset(CountNo, 0, sizeof(char)*InputLength);
        //[vers| add for fake barcode input]
        //strcpy(CountNo, "1000000");
        strcpy(CountNo, FakeInput[3]);
        //[vers| end]
        digitalWrite (WiringPiPIN_15, HIGH);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, LOW);
        sleep(1);
 
        memset(UserNo, 0, sizeof(char)*InputLength);
        //[vers| add for fake barcode input]
        //strcpy(UserNo, "6957");
        strcpy(UserNo, FakeInput[4]);
        //[vers| end]
        digitalWrite (WiringPiPIN_15, LOW);
        digitalWrite (WiringPiPIN_16, HIGH);
        digitalWrite (WiringPiPIN_18, LOW);
        sleep(1);

        memset(UPLoadFile, 0, sizeof(char)*UPLoadFileLength);
        gettimeofday(&now, NULL);
        sprintf(UPLoadFile,"%ld%s.txt",(long)now.tv_sec, MachineCode); 
        
        printf("%s %s %s %s %s %s\n", ISNo, ManagerCard, MachineCode, UserNo, CountNo, UPLoadFile);
 
        MasterFlag = 1;
        memset(ExProductCountArray, 0, sizeof(long)*ProductCount);
        memset(ExzhError, 0, sizeof(long)*64);
 
        if(zhTelnetFlag == 0)
        {
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
                if(ExProductCountArray[Good]  >= atoi(CountNo))
                {
                    //finish job
                    printf("Houston we are ready to back!\n");
                    zhResetFlag = 1;
                    MasterFlag = 0;
                }
/*              else if(digitalRead(WiringPiPIN_22) == 1)
                {
                    //finish job
#ifdef LogMode
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
    Log(s, __func__, __LINE__, " PIN_22 call me back\n");
#endif
                    printf("Houston, someone call us back!\n");
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
    
            SerialThreadFlag = 0;
            PrintLeftLog = 1;
            sleep(1);
            pthread_join(SerialThread, NULL);

            FileFlag = 0;
            pthread_mutex_lock(&mutexFile);
            pthread_cond_signal(&cond_2);
            pthread_mutex_unlock(&mutexFile);
            pthread_join(FileThread, NULL);
            sleep(1);

            FTPFlag = 0;
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            pthread_join(FTPThread, NULL);
            sleep(1);

            //vers end
            
            /*if(MasterFlag)
            {
                digitalWrite (WiringPiPIN_15, LOW);
                digitalWrite (WiringPiPIN_16, LOW);
                digitalWrite (WiringPiPIN_18, HIGH);

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

                        digitalWrite (WiringPiPIN_15, HIGH);
                        digitalWrite (WiringPiPIN_16, LOW);
                        digitalWrite (WiringPiPIN_18, HIGH);

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
    int ftpCount = FTPCountValue;
    CURL *curl;
    CURLcode res;
    FILE *hd_src;
    struct stat file_info, st;
    curl_off_t fsize;

    struct timeval now;
    struct timespec outtime;
    long filesize  = 0;
    char UPLoadFile_2[UPLoadFileLength];

    memset(UPLoadFile_2, 0, sizeof(char)*UPLoadFileLength);
    
    while(FTPFlag)
    {   
        pthread_mutex_lock(&mutex);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + FTPPeriod;
        outtime.tv_nsec= now.tv_usec * 1000;

        pthread_cond_timedwait(&cond, &mutex, &outtime);
        pthread_mutex_unlock(&mutex);
        
        if(stat(UPLoadFile, &st)!=0)
        {
#ifdef PrintInfo 
            //printf("error! upload file not exist\n");
            ;
#endif
        }
        else
        { 
            filesize = st.st_size;
            //printf("%d %s filesize %ld\n", ftpCount, UPLoadFile ,st.st_size);

            if(filesize > 100000)
            {
                strcpy(UPLoadFile_2, UPLoadFile);
                ftpCount = 0;
                gettimeofday(&now, NULL);
                pthread_mutex_lock(&mutex_ftp);
                sprintf(UPLoadFile,"%ld%s.txt",(long)now.tv_sec, MachineCode); 
                pthread_mutex_unlock(&mutex_ftp);
            }
            else if((ftpCount == 0 || FileFlag == 0) && filesize > 0)
            {
                strcpy(UPLoadFile_2, UPLoadFile);
                ftpCount = 0;
                gettimeofday(&now, NULL);
                pthread_mutex_lock(&mutex_ftp);
                sprintf(UPLoadFile,"%ld%s.txt",(long)now.tv_sec, MachineCode);
                pthread_mutex_unlock(&mutex_ftp);
            }
            else;
            if(ftpCount == 0 && filesize > 0)
            {
                //char Remote_url[80] = "ftp://192.168.2.223:8888/";
                char Remote_url[80] = "ftp://192.168.10.254:21/home/";
                strcat(Remote_url,UPLoadFile_2);
                if(stat(UPLoadFile_2, &file_info)) 
                {
                    printf("Couldnt open %s: %s\n", UPLoadFile_2, strerror(errno));
#ifdef LogMode
                    Log(s, __func__, __LINE__, " FTP fail_1\n");
#endif
                    digitalWrite(WiringPiPIN_15, LOW);
                    digitalWrite(WiringPiPIN_16, LOW);
                    digitalWrite(WiringPiPIN_18, LOW);
                    //pthread_exit((void*)"Fail");
                    continue;
                }
   
                fsize = (curl_off_t)file_info.st_size;

                curl_global_init(CURL_GLOBAL_ALL);

                curl = curl_easy_init();
                if(curl)
                {
                    hd_src = fopen(UPLoadFile_2, "rb");
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
                        digitalWrite(WiringPiPIN_15, LOW);
                        digitalWrite(WiringPiPIN_16, LOW);
                        digitalWrite(WiringPiPIN_18, LOW);
                    }
                    else
                    {
                        digitalWrite(WiringPiPIN_15, LOW);
                        digitalWrite(WiringPiPIN_16, HIGH);
                        digitalWrite(WiringPiPIN_18, LOW);
                    }
        
                    //curl_slist_free_all (headerlist);
	            curl_easy_cleanup(curl);
                    fclose(hd_src);
                }
                curl_global_cleanup();
                ftpCount = FTPCountValue + FTPPeriod;
                unlink(UPLoadFile_2);  
            }
        } 
        if(ftpCount > 0)
        {
            ftpCount = ftpCount - FTPPeriod;
        }
        else
        {
            ftpCount = FTPCountValue + FTPPeriod;
        }
    }
#ifdef LogMode
        Log(s, __func__, __LINE__, " FTP exit\n");
#endif
}

long transferFormat(char w, char x, char y, char z)
{
    long ans;
    ans = (w/16)*10 + (w%16) + (x/16)*1000 + (x%16)* 100 + (y/16)*100000 + (y%16)*10000;
    //printf("%ld\n",ans);
    return ans;
}
