#include "tsw303.h"

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
            for(forCount = 0; forCount < EVENTSIZE; forCount++)
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
                                                                GOODCOUNT+2, MachineNo, UserNo, MachRUNNING);
                        }
                        else
                        {
                            fprintf(filePtr, "%s %s %s %ld %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ISNo, ManagerCard, CountNo, 
                                                                Count[forCount] - ExCount[forCount], (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachRUNNING);
                        }
                    }
                    else
                    {
                        fprintf(filePtr, "%s %s %s 0 %ld %ld %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                Count[forCount] - ExCount[forCount],
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                forCount+2, MachineNo, UserNo, MachRUNNING);
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
                                                                GOODCOUNT+2, MachineNo, RepairNo, MachREPAIRING);
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
                                                                GOODCOUNT+2, MachineNo, RepairNo, 
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRDone);
        break;
        case MachJobDone:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachJobDone);
        break;
        case MachLOCK:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachLOCK);            
        break;
        case MachUNLOCK:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachUNLOCK);        
        break;
        case MachSTOPForce1:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachSTOPForce1);        
        break;
        case MachSTOPForce2:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachSTOPForce2);        
        break;
        case MachSTART:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachSTART);             
        break;
        case MachSTANDBY:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s 0 0 0 0 %02d\n", (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, MachSTANDBY);
        break;
        case MachPOWEROFF:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s 0 0 0 0 %02d\n", (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, MachPOWEROFF);
        break;
        case MachRESUMEFROMPOWEROFF:
             gettimeofday(&now, NULL);
             fprintf(filePtr, "0 0 0 0 %ld 0 %s %d %s 0 0 0 0 %02d\n", (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, MachRESUMEFROMPOWEROFF);
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
    int charCount = 0;
    int stringCount = 0;
    struct termios options;

    memset(tempOutput, 0, sizeof(char)*RS232LENGTH);

    if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
    {
        printf ("Unable to open serial device: %s\n", strerror (errno)) ;
        pthread_exit((void*)"fail");
    }

    tcgetattr(fd, &options);
    options.c_cflag |= PARENB;
    tcsetattr(fd, TCSANOW, &options);

    while(SerialFunctionFlag)
    {
        while(serialDataAvail(fd))
        {
            char tempChar1 = serialGetchar(fd);
            if(charCount == 2 && stringCount < RS232LENGTH - 1)
            {
                tempOutput[stringCount] = tempChar1;
                stringCount++;
            }else if(charCount == 2 && stringCount == (RS232LENGTH - 1))
            {
                //package
                tempOutput[stringCount] = '\0';
                pthread_mutex_lock(&MutexSerial);
                memset(FileOutput, 0,sizeof(char)*RS232LENGTH);
                int forCount = 0;
                for(forCount = 0; forCount < RS232LENGTH; forCount++)
                {
                    FileOutput[forCount] = tempOutput[forCount];                    
                }
                memset(tempOutput, 0, sizeof(char)*RS232LENGTH);
                charCount = 0;
                stringCount = 0;
                UpdateFlag = 1;
                pthread_mutex_unlock(&MutexSerial);
            }else if(tempChar1 == 0x08)
            {
                charCount = 1;
            }else if(charCount == 1 && tempChar1 == 0x06)
            {
                charCount = 2;                
            }else charCount = 0;
    
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
    unsigned char countArray[9];
    unsigned char newDataIncome = 0;

    int forCount = 0;
    short errorCheckCount[3];
    char result[3]; //remeber last stage 

    memset(errorCheckCount, 0, sizeof(short)*3);    
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
            pthread_mutex_lock(&MutexSerial);
            for(forCount = 0; forCount < RS232LENGTH; forCount++)
            {
                tempFileOutput[forCount] = FileOutput[forCount];
            }
            UpdateFlag = 0;
            pthread_mutex_unlock(&MutexSerial);
            if(tempFileOutput[1] == 0x06)
            {
                printf("0x06: ");
                for(forCount = 0; forCount < 5; forCount++)
                {
                    printf("%x ", tempFileOutput[forCount]);
                }
                printf("\n");
                countArray[3] = tempFileOutput[2];
                countArray[2] = tempFileOutput[3];
            }else if(tempFileOutput[1] == 0x05)
            {
                printf("0x05: ");
                for(forCount = 0; forCount < 5; forCount++)
                {
                    printf("%x ", tempFileOutput[forCount]);
                }
                printf("\n");
                countArray[1] = tempFileOutput[2];
                countArray[0] = tempFileOutput[3];  
            }else if(tempFileOutput[1] == 0x0C)
            {
                printf("0x0C: ");
                for(forCount = 0; forCount < 5; forCount++)
                {
                    printf("%x ", tempFileOutput[forCount]);
                }
                printf("\n");

                countArray[6] = tempFileOutput[2];
                countArray[5] = tempFileOutput[3];
            }else if(tempFileOutput[1] == 0x0D)
            {
                printf("0x0D: ");
                for(forCount = 0; forCount < 5; forCount++)
                {
                    printf("%x ", tempFileOutput[forCount]);
                }
                printf("\n");

                countArray[8] = tempFileOutput[2];
                countArray[7] = tempFileOutput[3];
            }else if(tempFileOutput[1] == 0x07)
            {
                printf("0x07: %x %x %x %x\n", tempFileOutput[0], tempFileOutput[1], tempFileOutput[2], tempFileOutput[3]);
                countArray[4] = tempFileOutput[3];
            }else if(tempFileOutput[1] == 0x00)
            {
                printf("0x00: ");
                for(forCount = 0; forCount < 5; forCount++)
                {
                    printf("%x ", tempFileOutput[forCount]);
                }
                printf("\n");

                char result1 = result[0] ^ 0xff;
                result1 = result1 & tempFileOutput[3];
                char result2 = result[1] ^ 0xff;
                result2 = result2 & tempFileOutput[2];
                
                result[0] = tempFileOutput[3];
                result[1] = tempFileOutput[2];

                for(forCount = 0; forCount < 8; forCount++)
                {
                    //3 4 5 6 7 8 9 10 
                    if((result1 & 1) == 1)
                    {
                        Count[forCount+3] = Count[forCount+3] + 1;
                    }
                    //11 12 13 14 15 16 17 18
                    if((result2 & 1) == 1)
                    {
                        Count[forCount+11] = Count[forCount+11] + 1;
                    }
                    result1 = result1 >> 1;
                    result2 = result2 >> 1; 
                }
            }else if(tempFileOutput[1] == 0x01)
            {
                printf("0x01: ");
                for(forCount = 0; forCount < 5; forCount++)
                {
                    printf("%x ", tempFileOutput[forCount]);
                }
                printf("\n");

                char result3 = result[2] ^ 0xff;
                result3 = result3 & tempFileOutput[3];
                result[2] = tempFileOutput[3];
                for(forCount = 19; forCount < EVENTSIZE; forCount++)
                {
                    if((result3 & 1) == 1)
                    {
                        Count[forCount] = Count[forCount] + 1;
                    }
                    result3 = result3 >> 1;
                }
            }else;
            Count[GoodNumber] = Count[GoodTotalNumber] = 0;
            for(forCount = 0; forCount < 5; ++forCount)
            {
                if(forCount == 4)
                {
                    Count[BadNumber] = TransferFormatLongFunction(countArray[4]); 
                }else
                {
                    Count[GoodNumber] = Count[GoodNumber] + TransferFormatLongFunction(countArray[forCount]) * pow(256, forCount);
                    Count[GoodTotalNumber] = Count[GoodTotalNumber] + TransferFormatLongFunction(countArray[forCount+5]) * pow(256, forCount);
                }
            }
            for(forCount = 0; forCount < 3; ++forCount)
            {
                if(abs(ExCount[forCount] - Count[forCount]) > 220 && errorCheckCount[forCount] < ERRORCHECKMAXRETRY)
                {
                    Count[forCount] = ExCount[forCount];
                    errorCheckCount[forCount]++;
                }
                else
                {
                    errorCheckCount[forCount] = 0;
                }
            }
            /*
            if( tempFileOutput[1] == 0x00 || tempFileOutput[1] == 0x01 || tempFileOutput[1] == 0x05 || 
                tempFileOutput[1] == 0x06 || tempFileOutput[1] == 0x07 || tempFileOutput[1] == 0x0C ||
                tempFileOutput[1] == 0x0D)
            {
                ;
            }*/
        }

        writeFileCount = (writeFileCount + WATCHDOGPERIOD) % WRITEFILECOUNTVALUE;   
        if(writeFileCount == 0 || WatchdogThreadFlag == 0)
        {   

            //[vers|2014.10.25 | initial count number]
            for(forCount = 0; forCount < 3; ++forCount)
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
                if(Count[forCount] != ExCount[forCount])
                {
                    ExCount[forCount] = Count[forCount];
                    if(newDataIncome == 0) newDataIncome = 1;
                }
            }
            //memcpy(ExCount, Count, sizeof(unsigned long)*EVENTSIZE);

            TotalBadCount = ExCount[BadNumber]; 
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
