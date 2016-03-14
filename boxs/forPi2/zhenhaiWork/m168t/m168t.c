#include "m168t.h"

int SetI2cConfig()
{
    int fd, r;
    fd = open(I2CDEVICEADDR, O_RDWR);
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, I2CIOEXTEND1);
    if(r < 0)
    {
        perror("Selecting i2c device");
        return 1;
    }
    i2c_smbus_write_byte_data(fd, OUTP0, 0x00);
    i2c_smbus_write_byte_data(fd, INVP0, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIGP0, 0x00);

    i2c_smbus_write_byte_data(fd, OUTP1, 0x00);
    i2c_smbus_write_byte_data(fd, INVP1, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIGP1, 0x00); 
    i2c_smbus_read_byte_data(fd, INP0);
    i2c_smbus_read_byte_data(fd, INP1);
    close(fd);
         
    fd = open(I2CDEVICEADDR, O_RDWR);      
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, I2CIOEXTEND2);
    if(r < 0)
    {
        perror("Selection i2c device fail");
        return 1;
    }
    i2c_smbus_write_byte_data(fd, OUTP0, 0x00);
    i2c_smbus_write_byte_data(fd, INVP0, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIGP0, 0x00);

    i2c_smbus_write_byte_data(fd, OUTP1, 0x00);
    i2c_smbus_write_byte_data(fd, INVP1, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIGP1, 0x00);
    i2c_smbus_read_byte_data(fd, INP0);
    i2c_smbus_read_byte_data(fd, INP1);
    close(fd);

    fd = open(I2CDEVICEADDR, O_RDWR);
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, I2CIOEXTEND3);
    if(r < 0)
    {
       perror("Selection i2c device fail");
       return 1;
    }
    i2c_smbus_write_byte_data(fd, OUTP0, 0x00);
    i2c_smbus_write_byte_data(fd, INVP0, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIGP0, 0x00);
        
    i2c_smbus_write_byte_data(fd, OUTP1, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIGP1, 0x00);
    i2c_smbus_read_byte_data(fd, INP0);
    i2c_smbus_read_byte_data(fd, INP1);
    close(fd);

    return 0;
}

/*
int WriteFile(int mode)
{
    FILE *filePtr;
    int forCount = 0;
    struct timeval now;
    static struct  timeval changeIntoRepairmodeTimeStemp;
    struct ifreq ifr;
    int fd;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ZHNETWORKTYPE, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    
    filePtr = fopen(UploadFilePath, "a");
    switch(mode)
    {
        case MachRUNNING:
            gettimeofday(&now, NULL);            
            for(forCount = 0; forCount < EVENTSIZE; ++forCount)
            {
                if(Count[forCount] != ExCount[forCount])
                {
                    if(forCount == GOODCOUNT)
                    {
                        if(abs(Count[forCount] - ExCount[forCount]) > 10)
                        {
                            fprintf(filePtr, "%s %s %s -1 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachRUNNING);
                        }
                        else
                        {
                            fprintf(filePtr, "%s %s %s %ld %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ISNo, ManagerCard, CountNo, 
                                                                Count[forCount] - ExCount[forCount], (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachRUNNING);
                        }
                    }
                    else
                    {
                        fprintf(filePtr, "%s %s %s 0 %ld %ld %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                Count[forCount] - ExCount[forCount],
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                forCount+1, MachineNo, UserNo, MachRUNNING);
                    }
                }else;
            }
        break;
        case MachREPAIRING:
            gettimeofday(&changeIntoRepairmodeTimeStemp, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ISNo, ManagerCard, CountNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, RepairNo, MachREPAIRING);
            
        break;
        case MachREPAIRING2:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                atoi(FixItemNo), MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec,
                                                                MachREPAIRING);
        break;
        case MachREPAIRDone:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s %ld 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRDone);
        break;
        case MachJobDone:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachJobDone);
        break;
        case MachLOCK:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachLOCK);            
        break;
        case MachUNLOCK:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachUNLOCK);        
        break;
        case MachSTOPForce1:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachSTOPForce1);        
        break;
        case MachSTOPForce2:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachSTOPForce2);        
        break;
        case MachSTART:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachSTART);             
        break;
        case MachSTANDBY:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s 0 0 0 0 %02d\n", (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, MachSTANDBY);
        break;
        case MachRESUMEFROMPOWEROFF:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s 0 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, MachRESUMEFROMPOWEROFF);
        break;
        case MachPOWEROFF:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, UserNo, MachPOWEROFF);
        break;
        default:
        ;
    }
    //write data into file
    fclose(filePtr);
    return 0;
}
*/

void * ZHSerialFunction(void *argument)
{
    int fd;
    char tempOutput[RS232LENGTH];
    int frontCount = 0;
    int backCount = 0;
    int stringCount = 0;

    memset(tempOutput, 0, sizeof(char)*RS232LENGTH);

    if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
    {
        printf ("Unable to open serial device: %s\n", strerror (errno)) ;
        pthread_exit((void*)"fail");
    }

    while(SerialFunctionFlag)
    {
        while(serialDataAvail(fd))
        {
            char tempChar = serialGetchar(fd);
            if(frontCount < 4)
            {
                if(tempChar == 0xdd)
                {
                    frontCount++;
                }else
                {
                    frontCount = 0;
                }
            }else if(backCount == 3 && tempChar == 0xee && stringCount < RS232LENGTH)
            {
                tempOutput[stringCount] = tempChar;
                stringCount++;
                int forCount;

                pthread_mutex_lock(&MutexSerial);
                memset(FileOutput, 0, sizeof(char)*RS232LENGTH);
                for(forCount = 0; forCount < RS232LENGTH; ++forCount)
                {
                    FileOutput[forCount] = tempOutput[forCount];
                }
                memset(tempOutput, 0, sizeof(char)*RS232LENGTH);
                frontCount = backCount = 0;
                stringCount = 0;
                UpdateFlag = 1;  
                pthread_mutex_unlock(&MutexSerial);
            }else if(stringCount < RS232LENGTH)
            {
                if(tempChar == 0xee)
                {
                    backCount++;
                    tempOutput[stringCount] = tempChar;
                    stringCount++;
                }else
                {
                    backCount = 0;
                    tempOutput[stringCount] = tempChar;
                    stringCount++;
                }
            }else
            {
                printf("array overflow\n");
                memset(tempOutput, 0, sizeof(char)*RS232LENGTH);
                frontCount = backCount = stringCount = 0;
            }
            fflush (stdout) ;
            //if (SerialThreadFlag == 0) break;
        }
    }
    if(fd >= 0)
    {
        serialClose(fd);
    }
    printf("serial function exit\n");  
}

void * WatchdogFunction(void *argument)
{
    struct timeval now;
    struct timespec outtime;
    struct ifreq ethreq;
    
    int watchdogCoolDown = WATCHDOGVALUE;
    int fd2;
    int writeFileCount = 0;
    char tempFileOutput[RS232LENGTH];
    unsigned char newDataIncome = 0;

    int forCount = 0;
    short errorCheckCount[4];

    memset(errorCheckCount, 0, sizeof(short)*4);
    
    while(WatchdogThreadFlag)
    {
        pthread_mutex_lock(&MutexWatchdog);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec;
        outtime.tv_nsec= (now.tv_usec + (WATCHDOGPERIOD * 1000) ) * 1000;
        
        if(outtime.tv_nsec > 1000000000)
        {
            outtime.tv_sec += 1;
            outtime.tv_nsec = outtime.tv_nsec % 1000000000;
        }
        pthread_cond_timedwait(&CondWatchdog, &MutexWatchdog, &outtime);
        pthread_mutex_unlock(&MutexWatchdog);

        if(UpdateFlag == 1)
        {
            memset(tempFileOutput, 0, sizeof(char)*RS232LENGTH);
            Count[GOODCOUNT] = Count[1] = Count[2] = Count[3] = 0;
            //printf("%ld %ld %ld %ld\n", Count[GOODCOUNT], Count[1], Count[2], Count[3]);
    
            pthread_mutex_lock(&MutexSerial);
            for(forCount = 0; forCount < RS232LENGTH; ++forCount)
            {
                tempFileOutput[forCount] = FileOutput[forCount];
                printf("%x|", tempFileOutput[forCount]);
            }
            printf("\n");
            UpdateFlag = 0;
            pthread_mutex_unlock(&MutexSerial);
            MessageArray[BadLayout1] = TransferFormatIntFunction(tempFileOutput[18]);
            MessageArray[BadLayout2] = TransferFormatIntFunction(tempFileOutput[19]);
            MessageArray[BadLayout3] = TransferFormatIntFunction(tempFileOutput[20]);
            MessageArray[BadLayout4] = TransferFormatIntFunction(tempFileOutput[21]);

            for(forCount = 0; forCount < 4; ++forCount)
            {
                if(MessageArray[forCount] != 0)
                {
                    int messagePaser = MessageArray[forCount]; 
                    int exMessagePaser  = ExMessageArray[forCount];
                    int flag = 0;
                    int forCount2;

                    for(forCount2 = 0; forCount2 < 8; ++forCount2)
                    {
                        if((messagePaser & 1) == 1 && ((exMessagePaser) & 1) == 0)
                        {
                            Count[(forCount*8)+forCount2+4]++;
                            flag = 1;
                        }
                        messagePaser = messagePaser >> 1;
                        exMessagePaser = exMessagePaser >> 1;
                    }
                    if(flag == 1)ExMessageArray[forCount] = MessageArray[forCount];
                }
            }

            for(forCount = 0; forCount < 3; ++forCount)
            {
                Count[GOODCOUNT] = Count[GOODCOUNT] + TransferFormatLongFunction(tempFileOutput[6+forCount]) * pow(256, forCount);
            }

            for(forCount = 0; forCount < 2; ++forCount)
            {
                Count[1] = Count[1] + TransferFormatLongFunction(tempFileOutput[9+forCount]) * pow(256, forCount);
                Count[2] = Count[2] + TransferFormatLongFunction(tempFileOutput[11+forCount]) * pow(256, forCount);
                Count[3] = Count[3] + TransferFormatLongFunction(tempFileOutput[13+forCount]) * pow(256, forCount);
            }
            printf("%ld %ld %ld %ld\n", Count[GOODCOUNT], Count[1], Count[2], Count[3]);

            for(forCount = 0; forCount < 4; ++forCount)
            {
                if(abs(ExCount[forCount] - Count[forCount]) > 220 && (errorCheckCount[forCount] < ERRORCHECKMAXRETRY))
                {
                    Count[forCount] = ExCount[forCount];
                    errorCheckCount[forCount]++;
                }else
                {
                    errorCheckCount[forCount] = 0;
                }   
            }
        }
        /*if(WatchdogResetFlag)
        {
            watchdogCoolDown = WATCHDOGVALUE;
            WatchdogResetFlag = 0;
        }else
        {
            watchdogCoolDown = watchdogCoolDown - WATCHDOGPERIOD;
            printf("%d\n", watchdogCoolDown);
        }
        if(watchdogCoolDown <= 0)
        {
            ZHResetFlag = 1;
        }*/
        writeFileCount = (writeFileCount + WATCHDOGPERIOD) % WRITEFILECOUNTVALUE; 
        if(writeFileCount == 0 || WatchdogThreadFlag == 0)
        {   
            printf("%ld %ld %ld %ld\n", Count[GOODCOUNT], Count[1], Count[2], Count[3]);
            //for(forCount = 4; forCount < EVENTSIZE; ++forCount)
            //{
            //    printf("%ld ", Count[forCount]);
            //}
            //printf("\n");

            //[vers|2014.10.25 | initial count number]
            for(forCount = 0; forCount < 4; ++forCount)
            {
                //need set ExproductCountArray
                if(Count[forCount] < ExCount[forCount])
                {
                    // count reset
                    ExCount[forCount] = 0;
                }else if((Count[forCount] != 0) && (ExCount[forCount] == 0))
                {
                    ExCount[forCount] = Count[forCount];
                }
                else;
            }
            //[vers|2014.10.25|end] 
            //FILE I/O
            pthread_mutex_lock(&MutexFile);
            WriteFile(MachRUNNING);
            pthread_mutex_unlock(&MutexFile);

            for(forCount = 0; forCount < EVENTSIZE; ++forCount)
            {
                if(ExCount[forCount]!= Count[forCount])
                {
                    ExCount[forCount] = Count[forCount];
                    if(newDataIncome == 0)newDataIncome = 1;
                }   
            }
            //memcpy(ExCount, Count, sizeof(unsigned long)*EVENTSIZE);
            TotalBadCount = ExCount[1] + ExCount[2] + ExCount[3]; 
            printf("%s %s %s %s %s %s|Good Count: %ld|Total Bad: %ld\n",
                     MachineNo, ISNo, ManagerCard, UserNo, CountNo, UploadFilePath, ExCount[GOODCOUNT], TotalBadCount);

            if(ScreenIndex == 1)
            {
                UpdateScreenFunction(1);  
            } 
        
            //a timeout mechanism
            if(newDataIncome == 1)
            {
                watchdogCoolDown = WATCHDOGVALUE;
                newDataIncome = 0;
            }else 
            {
                watchdogCoolDown = watchdogCoolDown - WRITEFILECOUNTVALUE;
                printf("%d\n", watchdogCoolDown);   
            }
            if(watchdogCoolDown <= 0)
            {
                ZHResetFlag = 1;
            }
            //check network status
            fd2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            memset(&ethreq, 0, sizeof(ethreq));
            strncpy(ethreq.ifr_name, ZHNETWORKTYPE, IFNAMSIZ);
            ioctl(fd2,  SIOCGIFFLAGS, &ethreq);
        
            if(ethreq.ifr_flags & IFF_RUNNING)
            {
                //connect
                //digitalWrite (ZHPIN15, HIGH);
                //digitalWrite (ZHPIN16, HIGH);
                //digitalWrite (ZHPIN18, LOW);
            }else
            {
                //disconnect
                //digitalWrite (ZHPIN15, HIGH);
                //digitalWrite (ZHPIN16, LOW);
                //digitalWrite (ZHPIN18, LOW);
            }
            close(fd2);
        }       
    }
}

unsigned long TransferFormatLongFunction(unsigned char x)
{
    unsigned long ans = (unsigned long)x;
    return ans;
}

unsigned int TransferFormatIntFunction(unsigned char x)
{
    unsigned int ans = (unsigned int)x;
    return ans;
}
