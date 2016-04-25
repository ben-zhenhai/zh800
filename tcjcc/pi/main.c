#include "zhFunc.h"


#define INPUTSERIALLENGTH 40
#define OUTPUTSERIALLENGTH 40
#define REQUESTDATAPERIOD 60
#define UPLOADDATAPERIOD 300
#define SAMPLERATE 5
#define CONFIGPATH "/home/pi/works/waterqulity/tcjccConfig"
#define MachSTOPRUNNING 32
#define MachRUNNING 10
#define STRINGLENGTH 40

pthread_cond_t CONDSERIALSEND, CONDSERIALRECV, CONDFTP;
pthread_mutex_t MUTEXSERIALSEND, MUTEXSERIALRECV, MUTEXFTP, MUTEXDATA;

float Result[5];
long double zhFlow = 0;
int SumCount = 1;
long double getValue = 0;
short UpdateFlag = 0;
char Name[STRINGLENGTH];
char TargetId[STRINGLENGTH];
char MachineId[STRINGLENGTH];
char AccPw[STRINGLENGTH];
char MachineType[STRINGLENGTH];
char FtpServer[80];

int main()
{
    pthread_t sendSerialThread, recvSerialThread, ftpThread, remoteThread;
    int rc = 0;
    FILE *fptr;
    char configString[80];
    char *buffer, *charPosition;
    int filesize = 0;
    int arrayIndex = 0;

    memset(Name, 0, sizeof(char)*STRINGLENGTH);
    memset(TargetId, 0, sizeof(char)*STRINGLENGTH);
    memset(MachineId, 0, sizeof(char)*STRINGLENGTH);
    memset(MachineType, 0, sizeof(char)*STRINGLENGTH);
    memset(AccPw, 0, sizeof(char)*STRINGLENGTH);
    memset(FtpServer, 0, sizeof(char)*80);
    memset(configString, 0, sizeof(char)*80);

    fptr = fopen(CONFIGPATH, "r");
    if(fptr != NULL)
    {
        fseek(fptr, 0, SEEK_END);
        filesize = ftell(fptr);
        rewind(fptr);
        buffer = (char *)malloc(sizeof(char)*filesize);
        fread(buffer, 1, filesize, fptr);
        fclose(fptr);
        charPosition = buffer;

        while(filesize > 0)
        {
            if(*charPosition == '\n')
            {
                if(strncmp(configString, "FTPServer:", 10) == 0)
                {
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex-10; ++forCount)
                    {
                        FtpServer[forCount] = configString[forCount+10];
                    }
                    printf("%s|\n", FtpServer);
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "TargetId:", 9) == 0)
                {
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 9; ++forCount)
                    {
                        TargetId[forCount] = configString[forCount+9];
                    }
                    printf("%s|\n", TargetId);
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "CompanyNo:", 10) == 0)
                {
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 10; ++forCount)
                    {
                        Name[forCount] = configString[forCount+10];
                    }
                    printf("%s|\n", Name);
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "MachineId:", 10) == 0)
                {
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 10; ++forCount)
                    {
                        MachineId[forCount] = configString[forCount+10];
                    }
                    printf("%s|\n", MachineId);
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "MachineType:", 12) == 0)
                {
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 12; ++forCount)
                    {
                        MachineType[forCount] = configString[forCount+12];
                    }
                    printf("%s|\n", MachineType);
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "AccPw:", 6) == 0)
                {
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 6; ++forCount)
                    {
                        AccPw[forCount] = configString[forCount+6];
                    }
                    //printf("%s|\n", AccPw);
                    memset(configString, 0, sizeof(char)*80);
                }else;
                arrayIndex = 0;
            }else
            {
                configString[arrayIndex] = *charPosition;
                ++arrayIndex;
            }
            ++charPosition;
            --filesize;
        }
        if(buffer != NULL)
        {
            free(buffer);
            charPosition = NULL;
        }

        pthread_mutex_init(&MUTEXSERIALSEND, NULL);
        pthread_mutex_init(&MUTEXSERIALRECV, NULL);
        pthread_mutex_init(&MUTEXFTP, NULL);
        pthread_mutex_init(&MUTEXDATA, NULL);

        pthread_cond_init(&CONDSERIALSEND, NULL);
        pthread_cond_init(&CONDSERIALRECV, NULL);
        pthread_cond_init(&CONDFTP, NULL);

        memset(Result, 0 , sizeof(float)*5);

        wiringPiSetup();

        rc = pthread_create(&sendSerialThread, NULL, SendSerialFunction, NULL);
        assert(rc == 0);

        //rc = pthread_create(&recvSerialThread, NULL, ReceiveSerialFunction, NULL);
        //assert(rc == 0);

        rc = pthread_create(&ftpThread, NULL, FtpFunction, NULL);
        assert(rc == 0);

        while(1)
        {
            ;
        }

        pthread_join(sendSerialThread, NULL);
        sleep(1);
        //pthread_join(recvSerialThread, NULL);
        //sleep(1);
        pthread_join(ftpThread, NULL);
        sleep(1);
        //pthread_join(remoteThread, NULL);
        //sleep(1);
    }
    else
    {
        printf("can not open config file\n");
    }
    return 0;
}

void * ReceiveSerialFunction(void * argument)
{
    int fd;
    unsigned char output[OUTPUTSERIALLENGTH];
    struct timeval now;
    struct timespec outTime;

    memset(output, 0, sizeof(char)*OUTPUTSERIALLENGTH);
    while(1)
    {
        if((fd = serialOpen("/dev/ttyAMA0", 9600)) < 0)
        {
            printf("Unable to open serial device: %s\n", strerror(errno));
        }
        else
        {
            while(serialDataAvail(fd))
            {
                char temp_char = serialGetchar(fd);
                printf("%x ",temp_char);
            }
            serialClose(fd);
        }
        pthread_mutex_lock(&MUTEXSERIALRECV);
        gettimeofday(&now, NULL);
        outTime.tv_sec = now.tv_sec + REQUESTDATAPERIOD;
        outTime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&CONDSERIALRECV, &MUTEXSERIALRECV, &outTime);
        pthread_mutex_unlock(&MUTEXSERIALRECV);
   }
}

void * SendSerialFunction(void * argument)
{
    int fd;
    int forCount = 0;
    unsigned char input[INPUTSERIALLENGTH];
    unsigned char output[OUTPUTSERIALLENGTH];
    unsigned int crcResult = 0;
    long double exFlow = 0;
    long double * pLong;

    pLong = &exFlow;

    memset(input, 0, sizeof(char)*INPUTSERIALLENGTH);
    memset(output, 0, sizeof(char)*INPUTSERIALLENGTH);

    struct timeval now;
    struct timespec outTime;
    unsigned char checkCrc[2];


    if(strcmp(MachineType, "WaterAnalyse") == 0)
    {
        input[0] = atoi(MachineId);
        input[1] = 0x03;
        input[2] = 0x00;
        input[3] = 0x00;
        input[4] = 0x00;
        input[5] = 0x09;

        crcResult = zhCRCCheck(input, 6);

        input[6] = (0xff & (crcResult >> 8)) ;
        input[7] = (0xff & crcResult);
    }else
    {
        //water flow
        input[0] = atoi(MachineId);
        input[1] = 0x03;
        //input[2] = 0x03;
        //input[3] = 0x10;
        input[2] = 0x02;
        input[3] = 0x52;
        input[4] = 0x00;
        input[5] = 0x02;
        crcResult = zhCRCCheck(input, 6);

        input[6] = (0xff & (crcResult >> 8));
        input[7] = (0xff & crcResult);
        //end
    }

    //hard code for testing
    /*if(strcmp(MachineType, "WaterAnalyse") == 0)
    {
        input[0] = atoi(MachineId);
        input[1] = 0x03;
        input[2] = 0x12;
        input[3] = 0x02;
        input[4] = 0xbc;
        input[5] = 0x03;
        input[6] = 0xe7;
        input[7] = 0x00;
        input[8] = 0x00;
        input[9] = 0x00;
        input[10] = 0x00;
        input[11] = 0x00;
        input[12] = 0x00;
        input[13] = 0x03;
        input[14] = 0xe8;
        input[15] = 0x03;
        input[16] = 0xda;
        input[17] = 0x03;
        input[18] = 0xe8;
        input[19] = 0x00;
        input[20] = 0xfb;

        crcResult = zhCRCCheck(input, 21);

        input[21] = (0xff & (crcResult >> 8));
        input[22] = (0xff & crcResult);
        //end
    }else
    {
        input[0] = atoi(MachineId);
        input[1] = 0x03;
        input[2] = 0x04;
        input[3] = 0x00;
        input[4] = 0x01;
        input[5] = 0x86;
        input[6] = 0xb7;

        crcResult = zhCRCCheck(input, 7);

        input[7] = (0xff & (crcResult >> 8));
        input[8] = (0xff & crcResult);
    }
    */
    while(1)
    {
        int arrayCount = 0;
        if((fd = serialOpen("/dev/ttyAMA0", 9600)) < 0)
        {
            printf("Unable to open serial device: %s\n", strerror(errno));
        }
        else
        {
            printf("ready to printf:\n");
            for(forCount = 0; forCount < 8; ++forCount)
            //for(forCount = 0; forCount < 23; ++forCount)
            {
                serialPutchar(fd, input[forCount]);
                printf("%x ", input[forCount]);
            }
            printf("\n");
            printf("ready to receive\n");
            sleep(2);
            while(serialDataAvail(fd))
            {
                output[arrayCount]= serialGetchar(fd);
                printf("%x ", output[arrayCount]);
                ++arrayCount;
            }
            serialFlush(fd);
            printf("\n");

            if(arrayCount > 8)
            {
                memset(checkCrc, 0 ,sizeof(unsigned char)*2);
                crcResult = zhCRCCheck(output, arrayCount-2);

                checkCrc[0] = (0xff & (crcResult) >> 8);
                checkCrc[1] = (0xff & (crcResult));

                if(checkCrc[0] == output[arrayCount-2] && checkCrc[1] == output[arrayCount-1])
                {
                    pthread_mutex_lock(&MUTEXDATA);
                    if(strcmp(MachineType, "WaterAnalyse") == 0 && arrayCount > 22)
                    {
                        PerserFunction1(output);
                        SumCount = (SumCount % SAMPLERATE) + 1;
                        UpdateFlag = 1;
                    }else if(strcmp(MachineType, "WaterFlow") == 0)
                    {
                        PerserFunction2(output, &pLong);
                        SumCount = (SumCount % SAMPLERATE) + 1;
                        UpdateFlag = 1;
                        getValue = getValue + 1;
                    }else;
                    pthread_mutex_unlock(&MUTEXDATA);
                    arrayCount = 0;

                }
                else
                {
                    printf("\n%x %x||%x %x check sum does not match\n", checkCrc[0], checkCrc[1], input[arrayCount-2], input[arrayCount-1]);
                }
            }
            else
            {
                printf("receive error data!! \n");
            }
        }
        if(fd >= 0)
        {
            serialClose(fd);
        }
        pthread_mutex_lock(&MUTEXSERIALSEND);
        gettimeofday(&now, NULL);
        if(strcmp(MachineType, "WaterFlow") == 0)
        {
            outTime.tv_sec = now.tv_sec + (REQUESTDATAPERIOD/5);
        }else{
            outTime.tv_sec = now.tv_sec + REQUESTDATAPERIOD;
        }
        outTime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&CONDSERIALSEND, &MUTEXSERIALSEND, &outTime);
        pthread_mutex_unlock(&MUTEXSERIALSEND);

        /*input[6] = input[6] + 1;
        crcResult = zhCRCCheck(input, 7);

        input[7] = (0xff & (crcResult >> 8));
        input[8] = (0xff & crcResult);*/
    }
}

void * FtpFunction(void * argument)
{
    FILE *fptr;
    struct timeval now;

    struct tm timeStamp;
    char currentYear[STRINGLENGTH];
    char currentTime[STRINGLENGTH];
    char upLoadFile[STRINGLENGTH];
    CURL *curl;
    CURLcode res;
    curl_off_t fsize;
    struct stat fileInfo;
    struct timespec outTime;

    memset(currentTime, 0, sizeof(char)*STRINGLENGTH);
    memset(upLoadFile, 0, sizeof(char)*STRINGLENGTH);
    memset(currentYear, 0 ,sizeof(char)*STRINGLENGTH);


    while(1)
    {
        char remoteUrl[80];
        memset(remoteUrl, 0, sizeof(char)*80);
        memcpy(remoteUrl,FtpServer, 80);

        pthread_mutex_lock(&MUTEXFTP);
        gettimeofday(&now, NULL);
        outTime.tv_sec = now.tv_sec + UPLOADDATAPERIOD;
        outTime.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&CONDFTP, &MUTEXFTP, &outTime);
        pthread_mutex_unlock(&MUTEXFTP);

        gettimeofday(&now, NULL);
        timeStamp = *localtime(&now);
        strftime(currentTime, sizeof(currentTime), "%m%d%H%M", &timeStamp);
        strftime(currentYear, sizeof(currentYear), "%Y", &timeStamp);
        //strcpy(upLoadFile, currentTime);
        //strcat(upLoadFile, ".");
        strcpy(upLoadFile,TargetId);
        fptr = fopen(upLoadFile, "w");
        if(fptr != NULL)
        {
            pthread_mutex_lock(&MUTEXDATA);
            if(UpdateFlag == 1)
            {
                //fprintf(fptr, "100%sWAR\n", Name);
                if(strcmp(MachineType, "WaterAnalyse") == 0)
                {
                    fprintf(fptr, "246%-6s%d%s%-10.2f%d\n", TargetId, atoi(currentYear) - 1911, currentTime, Result[zhPH], MachRUNNING);
                    fprintf(fptr, "247%-6s%d%s%-10.2f%d\n", TargetId, atoi(currentYear) - 1911, currentTime, Result[zhEC], MachRUNNING);
                    fprintf(fptr, "259%-6s%d%s%-10.1f%d\n", TargetId, atoi(currentYear) - 1911, currentTime, Result[zhTEMP], MachRUNNING);
                }else
                {
                    fprintf(fptr, "248%-6s%d%s%-10lf%d\n", TargetId, atoi(currentYear) - 1911, currentTime, zhFlow/(getValue*12), MachRUNNING);
                }
            }
            else
            {
                //fprintf(fptr, "100%sWAR\n",Name);
                if(strcmp(MachineType, "WaterAnalyse") == 0)
                {
                    fprintf(fptr, "246%-6s%d%s%-10.2f%d\n", TargetId, atoi(currentYear) - 1911, currentTime, 0.00, MachSTOPRUNNING);
                    fprintf(fptr, "247%-6s%d%s%-10.2f%d\n", TargetId, atoi(currentYear) - 1911, currentTime, 0.00, MachSTOPRUNNING);
                    fprintf(fptr, "259%-6s%d%s%-10.1f%d\n", TargetId, atoi(currentYear) - 1911, currentTime, 0.0, MachSTOPRUNNING);
                }else
                {
                    fprintf(fptr, "248%-6s%d%s%-10f%d\n", TargetId, atoi(currentYear) - 1911, currentTime, 0.00, MachSTOPRUNNING);
                }
            }

            SumCount = 1;
            UpdateFlag = 0;
            pthread_mutex_unlock(&MUTEXDATA);

            fclose(fptr);
            strcat(remoteUrl, upLoadFile);

            //ftp process
            if(stat(upLoadFile, &fileInfo))
            {
                printf("Could not open %s %s\n", upLoadFile, strerror(errno));
            }
            if(fileInfo.st_size > 0)
            {
                fsize = (curl_off_t)fileInfo.st_size;
                curl_global_init(CURL_GLOBAL_ALL);
                curl = curl_easy_init();
                if(curl)
                {
                    fptr = fopen(upLoadFile, "rb");
                    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
                    curl_easy_setopt(curl, CURLOPT_USERPWD, AccPw);
                    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
                    curl_easy_setopt(curl, CURLOPT_URL, remoteUrl);
                    curl_easy_setopt(curl, CURLOPT_READDATA, fptr);
                    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
                    res = curl_easy_perform(curl);
                    if(res != CURLE_OK)
                    {
                        printf("sorry upload fail %d\n", res);
                    }

                    curl_easy_cleanup(curl);
                    if(fptr)
                    {
                        fclose(fptr);
                    }
                }
                curl_global_cleanup();
            }
            memset(Result, 0, sizeof(float)*5);
            zhFlow = 0;
            getValue = 0;
            unlink(upLoadFile);
        }
    }
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

void PerserFunction1(unsigned char *data)
{
    Result[zhPH] = (Result[zhPH] * (SumCount-1) + (float) (data[3]*256 + data[4]) / 100) / SumCount;
    Result[zhTEMP] = (Result[zhTEMP] * (SumCount-1) + (float) (data[19]*256 + data[20]) / 10) / SumCount;
    Result[zhEC] = (Result[zhEC] * (SumCount-1) + (float) (data[15]*256 + data[16]) / 100) / SumCount;
    printf(" %.2f %.2f %.2f\n", Result[zhPH], Result[zhTEMP], Result[zhEC]);
}
//void PerserFunction2(unsigned char *data, unsigned long **plong)
void PerserFunction2(unsigned char *data, long double **plong)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = 32 - 8 - 1; // -1 for sign bit
    uint32_t i = 0;

    i = (uint32_t)data[3] << 24 |
        (uint32_t)data[4] << 16 |
        (uint32_t)data[5] << 8  |
        (uint32_t)data[6];

    if (i == 0) **plong = 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(8-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<8)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(32-1))&1? -1.0: 1.0;

    zhFlow = zhFlow + result;
    **plong = result;
    printf("Total Flow:%lf ,zhFlow: %lf\n",**plong,zhFlow);
}

void * RemoteFunction(void * argument)
{
    ;
}

unsigned int zhCRCCheck(unsigned char* data, int dataLength)
{
    int forCount = 0;
    unsigned int regCRC = 0xffff;

    while(dataLength--)
    {
        //printf("%3x", *data);
        regCRC ^= *data++;
        for(forCount = 0; forCount < 8; forCount++)
        {
            if(regCRC & 0x01)
                regCRC = (regCRC>>1) ^ 0xA001;
            else
                regCRC = regCRC >> 1;

        }
    }
    unsigned int outCRC=((regCRC<<8)&0xFF00)|((regCRC>>8)&0x00FF);
    //printf(" %3x %3x\n", regCRC, outCRC);
    return outCRC;
}

