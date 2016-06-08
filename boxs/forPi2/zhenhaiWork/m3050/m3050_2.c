#include "m3050.h"

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
    int fd, forCount = 0;
	struct timeval now;
    static struct  timeval changeIntoRepairmodeTimeStemp;
	struct ifreq ifr;
    
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
                    if(NewDataIncome == 0) NewDataIncome = 1;
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
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRING);
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
    int count1 , count2;
    struct termios options;
    struct timeval now;
    struct timespec outtime;
    short errorCheckCount[EVENTSIZE];
    int watchdogCooldown = WATCHDOGVALUE;

    memset(errorCheckCount, 0, sizeof(short)*EVENTSIZE);
    count1 = count2 = 0;

    if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
    {
        printf ("Unable to open serial device: %s\n", strerror (errno)) ;
        pthread_exit((void*)"fail");
    }

    tcgetattr(fd, &options);
    options.c_cflag |= PARENB;
    options.c_cflag &= ~PARODD;
    options.c_cflag |= CSTOPB;
    //options.c_cflag |= PARENB;
    //options.c_cflag &= ~PARODD;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;
    tcsetattr(fd, TCSANOW, &options); 
    //tcsetattr(fd, &options); 

    while(SerialFunctionFlag)
    {
        char arraySender[RS232LengthSender];
        int forCount = 0;
        int charCount = 0;
        unsigned char arrayReceiver[RS232LengthReceiver];
        unsigned char prefixCheck[8];
        short flagPack = 0;
        memset(Count, 0, sizeof(long)*EVENTSIZE);

        //3
        memset(arraySender, 0, sizeof(char)*RS232LengthSender);
        memset(arrayReceiver, 0 , sizeof(unsigned char)*RS232LengthReceiver);
        memset(prefixCheck, 0, sizeof(unsigned char)*8);
        //strcpy(arraySender, "@11RD5420001054*");
        strcpy(arraySender, "@11RD542100085C*");
        arraySender[16] = 0x0d;
        arraySender[17] = 0x0a;
        charCount = 0;
        count1 = 0;
        flagPack = 0;
          
        for(forCount = 0; forCount < 18; ++forCount)
        {
            serialPutchar(fd, arraySender[forCount]);
        }
        usleep(100000);
        //sleep(1);


        printf("\n3\n");
        while(serialDataAvail(fd))
        {
            char tempChar1;
            tempChar1 = serialGetchar(fd);
            printf("%x ", tempChar1);
            if(flagPack == 1 && tempChar1 == '*')
            {   
                int localCharCount = 0;
                forCount = 0;
                printf("\nlength: %d\n", charCount);
                for(localCharCount = 0; localCharCount <= charCount-3 ; localCharCount = localCharCount + 4)
                {
                    Count[forCount+6] = 
                            TransferFormatLongFunction(arrayReceiver[localCharCount]) * 4096 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+1]) * 256 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+2]) * 16 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+3]);
                    forCount++;
                }
                flagPack = 0;
            }else if(flagPack == 1 && tempChar1 != '*')
            {
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
                //printf("%c ", tempChar1);
            }else if(count1 == 7 && strncmp(prefixCheck, "@11RD00",7) == 0)
            {
                flagPack = 1;
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
                printf("check\n");
            }else if(count1 != 0 && count1 < 7)
            {
                prefixCheck[count1] = tempChar1;
                count1++;
                printf("%d\n", count1);
            }else if(tempChar1 == 0x40)
            {
                count1 = 1;
                prefixCheck[0] = tempChar1;
                printf("check\n");
            }else
            {
                memset(prefixCheck, 0, sizeof(unsigned char)*8);
                memset(arrayReceiver, 0, sizeof(unsigned char)*RS232LengthReceiver);
                count1 = 0;
                charCount = 0;
                flagPack = 0;
                printf("clean\n");
            }
            fflush(stdout);
        }
        usleep(100000);
        //sleep(1);
        printf("\n3 size: %d\n", charCount);
 
        //send to Server
        //1
        memset(arraySender, 0 , sizeof(char)*RS232LengthSender);
        memset(arrayReceiver, 0 , sizeof(unsigned char)*RS232LengthReceiver);
        memset(prefixCheck, 0, sizeof(unsigned char)*8);
        strcpy(arraySender, "@11RD4900000853*");
        arraySender[16] = 0x0d;
        arraySender[17] = 0x0a;
        charCount = 0;
        count1 = 0;
        flagPack = 0;

        for(forCount = 0; forCount < 18; ++forCount)
        {
            serialPutchar(fd, arraySender[forCount]);
        }
        //usleep(100000);
        //nanosleep((const struct timespec[]){{0, 50000000L}}, NULL);
        sleep(1);

        printf("\n1\n");
        while(serialDataAvail(fd))
        {
            char tempChar1;
            tempChar1 = serialGetchar(fd);
            printf("%x ", tempChar1);
            if(flagPack == 1 && tempChar1 == '*')
            {
                int localCharCount = 0;
                forCount = 0;
                for(localCharCount = 0; localCharCount <= charCount-3 ; localCharCount = localCharCount + 8)
                {
                    Count[forCount] = 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+4]) * 268435456 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+5]) * 16777216 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+6]) * 1048576 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+7]) * 65536 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount]) * 4096 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+1]) * 256 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+2]) * 16 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+3]);
                
                   forCount++;
                   //printf("\nans %ld %ld %ld %ld\n",(long)arrayReceiver[localCharCount], (long)arrayReceiver[localCharCount+1], 
                   //         (long)arrayReceiver[localCharCount+2] , (long)arrayReceiver[localCharCount+3]);
                }
                flagPack = 0;
            }else if(flagPack == 1 && tempChar1 != '*')
            {
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
            }else if(count1 == 7 && strncmp(prefixCheck, "@11RD00", 7) == 0)
            {
                flagPack = 1;
                arrayReceiver[charCount] = tempChar1;
                ++charCount; 
            }else if(count1 != 0 && count1 < 7)
            {
                prefixCheck[count1] = tempChar1;
                count1++;
            }else if(tempChar1 == 0x40)
            {
                count1 = 1;
                prefixCheck[0] = tempChar1;
                printf("check\n");
            }else
            {
                memset(prefixCheck, 0, sizeof(unsigned char)*8);
                memset(arrayReceiver, 0, sizeof(unsigned char)*RS232LengthReceiver);
                count1 = 0;
                charCount = 0;
                flagPack = 0;
                printf("clean\n");
            }
            fflush(stdout);
        }
        //usleep(200000);
        printf("  1111 size:%d\n",charCount);
        //nanosleep((const struct timespec[]){{0, 50000000L}}, NULL);
        sleep(1);
        
        //2
        memset(arraySender, 0, sizeof(char)*RS232LengthSender);
        memset(arrayReceiver, 0 , sizeof(unsigned char)*RS232LengthReceiver);
        memset(prefixCheck, 0, sizeof(unsigned char)*8);
        strcpy(arraySender, "@11RD5410000254*");
        arraySender[16] = 0x0d;
        arraySender[17] = 0x0a;
        charCount = 0;
        count1 = 0;
        flagPack = 0;

        for(forCount = 0; forCount < 18; ++forCount)
        {
            serialPutchar(fd, arraySender[forCount]);
        }
        //usleep(100000);
        //nanosleep((const struct timespec[]){{0, 50000000L}}, NULL);
        sleep(1);

        while(serialDataAvail(fd))
        {
            char tempChar1;
            tempChar1 = serialGetchar(fd);
            printf("%x ", tempChar1);
            if(flagPack == 1 && tempChar1 == '*')
            {
                Count[4] = 
                    TransferFormatLongFunction(arrayReceiver[4]) * 268435456 + 
                    TransferFormatLongFunction(arrayReceiver[5]) * 16777216 + 
                    TransferFormatLongFunction(arrayReceiver[6]) * 1048576 + 
                    TransferFormatLongFunction(arrayReceiver[7]) * 65536 + 
                    TransferFormatLongFunction(arrayReceiver[0]) * 4096 + 
                    TransferFormatLongFunction(arrayReceiver[1]) * 256 + 
                    TransferFormatLongFunction(arrayReceiver[2]) * 16 + 
                    TransferFormatLongFunction(arrayReceiver[3]);

                   //printf("%c %c %c %c\n", arrayReceiver[0], arrayReceiver[1],  arrayReceiver[2], arrayReceiver[3]);
                flagPack = 0;
            }
            else if(flagPack == 1 && tempChar1 != '*')
            {
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
                //printf("%c ", tempChar1);
            }else if(count1 == 7 && strncmp(prefixCheck, "@11RD00", 7) == 0)
            {
                flagPack = 1;
                arrayReceiver[charCount] = tempChar1;
                ++charCount; 
            }else if(count1 != 0 && count1 < 7)
            {
                prefixCheck[count1] = tempChar1;
                count1++;
            }else if(tempChar1 == 0x40)
            {
                count1 = 1;
                prefixCheck[0] = tempChar1;
            }else
            {
                memset(prefixCheck, 0, sizeof(unsigned char)*8);
                memset(arrayReceiver, 0, sizeof(unsigned char)*RS232LengthReceiver);
                count1 = 0;
                charCount = 0;
                flagPack = 0;
            }
            fflush(stdout);
        }
        printf(" 2222 size:%d\n",charCount);
        //nanosleep((const struct timespec[]){{0, 200000000L}}, NULL);
        sleep(1);
        
         
        //printf("\n\n");
       
        //4
        memset(arraySender, 0, sizeof(char)*RS232LengthSender);
        memset(arrayReceiver, 0 , sizeof(unsigned char)*RS232LengthReceiver);
        memset(prefixCheck, 0, sizeof(unsigned char)*8);
        //strcpy(arraySender, "@11RD6200002959*");
        strcpy(arraySender, "@11RD6200001053*");
        arraySender[16] = 0x0d;
        arraySender[17] = 0x0a;
        charCount = 0;
        count1 = 0;
        flagPack = 0;

        for(forCount = 0; forCount < 18; ++forCount)
        {
            serialPutchar(fd, arraySender[forCount]);
        }
        usleep(100000);
        //sleep(1);

        while(serialDataAvail(fd))
        {
            char tempChar1;
            tempChar1 = serialGetchar(fd);
            //printf("%c ", tempChar1);
            if(flagPack == 1 && tempChar1 == '*')
            {   
                int localCharCount = 0;
                forCount = 0;
                for(localCharCount = 0; localCharCount <= charCount-3 ; localCharCount = localCharCount + 4)
                {
                    Count[forCount+15] = 
                            TransferFormatLongFunction(arrayReceiver[localCharCount]) * 4096 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+1]) * 256 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+2]) * 16 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+3]);

                    forCount++;
                }
                flagPack = 0; 
            }
            else if(flagPack == 1 && tempChar1 != '*')
            {
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
                //printf("%c ", tempChar1);
            }else if(count1 == 7 && strncmp(prefixCheck, "@11RD00", 7) == 0)
            {
                flagPack = 1;
                arrayReceiver[charCount] = tempChar1;
                ++charCount; 
            }else if(count1 != 0 && count1 < 7)
            {
                prefixCheck[count1] = tempChar1;
                count1++;
            }else if(tempChar1 == 0x40)
            {
                count1 = 1;
                prefixCheck[0] = tempChar1;
            }else
            {
                memset(prefixCheck, 0, sizeof(unsigned char)*8);
                memset(arrayReceiver, 0, sizeof(unsigned char)*RS232LengthReceiver);
                count1 = 0;
                charCount = 0;
            }
            fflush(stdout);
        }
        //printf("\n");
        usleep(100000);
        //sleep(1);
        //nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);

        //5
        memset(arraySender, 0, sizeof(char)*RS232LengthSender);
        memset(arrayReceiver, 0 , sizeof(unsigned char)*RS232LengthReceiver);
        memset(prefixCheck, 0, sizeof(unsigned char)*8);
        //strcpy(arraySender, "@11RD6200002959*");
        strcpy(arraySender, "@11RD6210001052*");
        arraySender[16] = 0x0d;
        arraySender[17] = 0x0a;
        charCount = 0;
        count1 = 0;
        flagPack = 0;

        for(forCount = 0; forCount < 18; ++forCount)
        {
            serialPutchar(fd, arraySender[forCount]);
        }
        usleep(100000);
        //nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
        //sleep(1);

        while(serialDataAvail(fd))
        {
            char tempChar1;
            tempChar1 = serialGetchar(fd);
            //printf("%c ", tempChar1);
            if(flagPack == 1 && tempChar1 == '*')
            {   
                int localCharCount = 0;
                forCount = 0;
                for(localCharCount = 0; localCharCount <= charCount-3 ; localCharCount = localCharCount + 4)
                {
                    Count[forCount+25] = 
                        TransferFormatLongFunction(arrayReceiver[localCharCount]) * 4096 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+1]) * 256 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+2]) * 16 + 
                        TransferFormatLongFunction(arrayReceiver[localCharCount+3]);
                    forCount++;
                }
                flagPack = 0; 
            }
            else if(flagPack == 1 && tempChar1 != '*')
            {
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
                //printf("%c ", tempChar1);
            }else if(count1 == 7 && strncmp(prefixCheck, "@11RD00", 7) == 0)
            {
                flagPack = 1;
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
            }else if(count1 != 0 && count1 < 7)
            {
                prefixCheck[count1] = tempChar1;
                count1++;
            }else if(tempChar1 == 0x40)
            {
                count1 = 1;
                prefixCheck[0] = tempChar1;
            }else
            {
                memset(prefixCheck, 0, sizeof(unsigned char)*8);
                memset(arrayReceiver, 0, sizeof(unsigned char)*RS232LengthReceiver);
                count1 = 0;
                charCount = 0;
            }
            fflush(stdout);
        }
        //printf("\n");
        usleep(100000);
        //nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
        //sleep(1);

        //6
        memset(arraySender, 0, sizeof(char)*RS232LengthSender);
        memset(arrayReceiver, 0 , sizeof(unsigned char)*RS232LengthReceiver);
        memset(prefixCheck, 0, sizeof(unsigned char)*8);
        //strcpy(arraySender, "@11RD6200002959*");
        strcpy(arraySender, "@11RD6220001051*");
        arraySender[16] = 0x0d;
        arraySender[17] = 0x0a;
        charCount = 0;
        count1 = 0;
        flagPack = 0;

        for(forCount = 0; forCount < 18; ++forCount)
        {
            serialPutchar(fd, arraySender[forCount]);
        }
        usleep(100000);
        //nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
        //sleep(1);

        while(serialDataAvail(fd))
        {
            char tempChar1;
            tempChar1 = serialGetchar(fd);
            //printf("%c ", tempChar1);
            if(flagPack == 1 && tempChar1 == '*')
            {   
                int localCharCount = 0;
                forCount = 0;
                for(localCharCount = 0; localCharCount <= charCount-3 ; localCharCount = localCharCount + 4)
                {
                    if(localCharCount == 4 || localCharCount == 8 || localCharCount == 12 || localCharCount == 16);
                    else
                    {
                        Count[forCount+35] = 
                            TransferFormatLongFunction(arrayReceiver[localCharCount]) * 4096 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+1]) * 256 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+2]) * 16 + 
                            TransferFormatLongFunction(arrayReceiver[localCharCount+3]);
                         forCount++;
                    }
                }
                flagPack = 0; 
            }
            else if(flagPack == 1 && tempChar1 != '*')
            {
                arrayReceiver[charCount] = tempChar1;
                ++charCount;
                //printf("%c ", tempChar1);
            }else if(count1 == 7 && strncmp(prefixCheck, "@11RD00", 7) == 0)
            {
                flagPack = 1;
                arrayReceiver[charCount] = tempChar1;
                ++charCount; 
            }else if(count1 != 0 && count1 < 7)
            {
                prefixCheck[count1] = tempChar1;
                count1++;
            }else if(tempChar1 == 0x40)
            {
                count1 = 1;
                prefixCheck[0] = tempChar1;
            }else
            {
                memset(prefixCheck, 0, sizeof(unsigned char)*8);
                memset(arrayReceiver, 0, sizeof(unsigned char)*RS232LengthReceiver);
                count1 = 0;
                charCount = 0;
            }
            fflush(stdout);
        }
        while(serialDataAvail(fd))
        {
            char tempChar1;
            tempChar1 = serialGetchar(fd);
            printf("useless %c\n ", tempChar1);
            fflush(stdout);
        }
 
        //printf("\n");
        for(forCount = 0; forCount < 39; ++forCount)
        {
            printf("%ld ", Count[forCount]);
        }
        printf("\n");

        for(forCount = 0; forCount < EVENTSIZE; ++forCount)
        {
            //need set ExCount
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
        
        //avoid wrong info send form machine
        for(forCount = 0; forCount < EVENTSIZE; ++forCount)
        {
            if(abs(ExCount[forCount] - Count[forCount]) > 15 && errorCheckCount[forCount] < ERRORCHECKMAXRETRY)
            {
                Count[forCount] = ExCount[forCount];
                errorCheckCount[forCount]++;
            }
            else
            {
                errorCheckCount[forCount] = 0;
            }
        }

        pthread_mutex_lock(&MutexFile);
        WriteFile(MachRUNNING);
        pthread_mutex_unlock(&MutexFile);
        
        memcpy(ExCount, Count, sizeof(long)*EVENTSIZE);
        TotalBadCount = ExCount[4]; 
        printf("%s %s %s %s %s %s|Good Count: %ld|Total Bad: %ld\n",
                     MachineNo, ISNo, ManagerCard, UserNo, CountNo, UploadFilePath, ExCount[GOODCOUNT], TotalBadCount);

        if(ScreenIndex == 1)
        {
           UpdateScreenFunction(1);  
        } 

        pthread_mutex_lock(&MutexSerial);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + WATCHDOGPERIOD;
        outtime.tv_nsec= now.tv_usec * 1000;
        pthread_cond_timedwait(&CondSerial, &MutexSerial, &outtime);
        pthread_mutex_unlock(&MutexSerial);
        
        //a timeout mechanism
        if(NewDataIncome == 1)
        {
            watchdogCooldown = WATCHDOGVALUE;
            NewDataIncome = 0;
        }else 
        {
            watchdogCooldown = watchdogCooldown - WATCHDOGPERIOD;
            printf("%d\n", watchdogCooldown);   
        }if(watchdogCooldown <= 0)
        {
            ZHResetFlag = 1;
        }
        /*
        //check network status
        int fd2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        struct ifreq ethreq;
        memset(&ethreq, 0, sizeof(ethreq));
        strncpy(ethreq.ifr_name, ZHNETWORKTYPE, IFNAMSIZ);
        ioctl(fd2, SIOCGIFFLAGS, &ethreq);

        if(ethreq.ifr_flags & IFF_RUNNING)
        {
            //digitalWrite (ZHPIN15, HIGH);
            //digitalWrite (ZHPIN16, HIGH);
            //digitalWrite (ZHPIN18, LOW);
        }else
        {   
            //digitalWrite (ZHPIN15, HIGH);
            //digitalWrite (ZHPIN16, LOW);
            //digitalWrite (ZHPIN18, LOW);
        }
        close(fd2);
        */
    }
    if(fd >= 0)
    {
        serialClose(fd);
    }
    printf("serial function exit\n");
}
unsigned long TransferFormatLongFunction(unsigned char x)
{
    unsigned long ans = 0;
    if(x == 0x31) ans = 1;
    else if(x == 0x32) ans = 2;
    else if(x == 0x33) ans = 3;
    else if(x == 0x34) ans = 4;
    else if(x == 0x35) ans = 5;
    else if(x == 0x36) ans = 6;
    else if(x == 0x37) ans = 7;
    else if(x == 0x38) ans = 8;
    else if(x == 0x39) ans = 9;
    else if(x == 0x41) ans = 10;
    else if(x == 0x42) ans = 11;
    else if(x == 0x43) ans = 12;
    else if(x == 0x44) ans = 13;
    else if(x == 0x45) ans = 14;
    else if(x == 0x46) ans = 15;
    else ans = 0;
   
    return ans;
}
