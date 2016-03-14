#include "taicon.h"

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
    i2c_smbus_write_byte_data(fd, INVP0, 0x80);
    i2c_smbus_write_byte_data(fd, CONFIGP0, 0x80);

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
                        if(abs(Count[forCount] - ExCount[forCount]) > ZHMAXOUTPUT )
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
                                                                (long)changeIntoRepairmodeTimeStemp.tv_sec, MachREPAIRING);
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
        case MachRESUMEFROMPOWEROFF:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s 0 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, MachRESUMEFROMPOWEROFF);
        break;
        case MachPOWEROFF:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ISNo, ManagerCard, CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+2, MachineNo, UserNo, MachPOWEROFF);
        break;
        default:
        ;
    }
    //write data into file
    fclose(filePtr);
    return 0;
}
*/

void * ZHI2cReaderFunction1(void *argument)
{
    int fd, r;
     
    while(1)
    {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        fd = open(I2CDEVICEADDR, O_RDWR);
        int x, y;
        int forCount, first, second;
                
        if(fd < 0)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            perror("Open fail");
            exit(0);
        }
        r = ioctl(fd, I2C_SLAVE, I2CIOEXTEND1);
        if(r < 0)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            perror("Selecting i2c device fail");
            exit(0);
        }

        x = i2c_smbus_read_byte_data(fd, INP0);
        y = i2c_smbus_read_byte_data(fd, INP1);
        close(fd);

        if(I2CEXValue[0] != x || I2CEXValue[1] != y)
        {
            first = I2CEXValue[0] ^ 0xff;
            first = first & x;
            
            second = I2CEXValue[1] ^ 0xff;
            second = second & y;
            
            I2CEXValue[0] = x;
            I2CEXValue[1] = y;
            
            for(forCount = 0; forCount < 8; ++forCount)
            {
                if(forCount == 7)
                {
                    Count[forCount] = Count[forCount] + (first & 1);
                }
                first = first >> 1;
                second = second >> 1;
            }
            WatchdogResetFlag = 1;
        }
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        usleep(USLEEPTIMEUNITVERYSHORT);
    }
}
/*void * ZHI2cReaderFunction2(void *argument)
{
    int fd, r, forCount, first, second;
    
    while(1)
    {
        int x, y;
        x = y = 0;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        fd = open(I2CDEVICEADDR, O_RDWR);
        
        if(fd < 0)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            perror("Open fail");
            exit(0);
        }
        r = ioctl(fd, I2C_SLAVE, I2CIOEXTEND2);
        if(r < 0)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            perror("Selecting i2c device fail");
            exit(0);
        }
        
        x = i2c_smbus_read_byte_data(fd, INP0);
        y = i2c_smbus_read_byte_data(fd, INP1);
        close(fd);
        
        if(I2CEXValue[2] != x || I2CEXValue[3] != y)
        {
            first = I2CEXValue[2] ^ 0xff;
            first = first & x;
            second = I2CEXValue[3] ^ 0xff;
            second = second & y;
            
            I2CEXValue[2] = x;
            I2CEXValue[3] = y;
            
            for(forCount = 0; forCount < 8; ++forCount)
            {
                Count[forCount+16] = Count[forCount+16] + (first & 1);
                first = first >> 1;
                Count[forCount+24] = Count[forCount+24] + (second & 1);
                second = second >> 1;
            }
            WatchdogResetFlag = 1;
        }
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        usleep(USLEEPTIMEUNITVERYSHORT);
    }
}*/
/*void * ZHI2cReaderFunction3(void *argument)
{
    int fd, r, forCount, first;
    
    while(1)
    {
        int x = 0;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        fd = open(I2CDEVICEADDR, O_RDWR);
        if(fd < 0)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            perror("Open fail");
            exit(0);
        }
        
        r = ioctl(fd, I2C_SLAVE, I2CIOEXTEND3);
        if(r < 0)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
            perror("Selecting i2c device fail");
            exit(0);
        }
        x = i2c_smbus_read_byte_data(fd, INP0);
        close(fd);
        
        if(I2CEXValue[4] != x)
        {
            first = I2CEXValue[4] ^ 0xff;
            first = first & x;
            
            I2CEXValue[4] = x;
            
            for(forCount = 0; forCount < 8; ++forCount)
            {
                Count[32+forCount] = Count[32+forCount] + (first & 1);
                first = first >> 1;
            }
            WatchdogResetFlag = 1;
        }
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        usleep(USLEEPTIMEUNITVERYSHORT);
    }
}*/

void * WatchdogFunction(void *argument)
{
    struct timeval now;
    struct timespec outtime;
    struct ifreq ethreq;
    
    int watchdogCoolDown = WATCHDOGVALUE;
    int fd2;
    
    while(WatchdogThreadFlag)
    {
        pthread_mutex_lock(&MutexWatchdog);
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec + WATCHDOGPERIOD;
        outtime.tv_nsec = now.tv_usec * 1000;

        pthread_cond_timedwait(&CondWatchdog, &MutexWatchdog, &outtime);
        pthread_mutex_unlock(&MutexWatchdog);
        if(WatchdogResetFlag)
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
        }
        //FILE I/O
        pthread_mutex_lock(&MutexFile);
        WriteFile(MachRUNNING);
        pthread_mutex_unlock(&MutexFile);

        memcpy(ExCount, Count, sizeof(unsigned long)*EVENTSIZE);
        printf("%s %s %s %s %s %s|Good Count: %ld|Total Bad: %ld\n",
                     MachineNo, ISNo, ManagerCard, UserNo, CountNo, UploadFilePath, ExCount[GOODCOUNT], TotalBadCount);

        if(ScreenIndex == 1)
        {
           UpdateScreenFunction(1);  
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
