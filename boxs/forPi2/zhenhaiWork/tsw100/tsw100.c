#include "tsw100.h"

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
    i2c_smbus_write_byte_data(fd, INVP0, 0xff);
    i2c_smbus_write_byte_data(fd, CONFIGP0, 0xff);

    i2c_smbus_write_byte_data(fd, OUTP1, 0x00);
    i2c_smbus_write_byte_data(fd, INVP1, 0xf3);
    i2c_smbus_write_byte_data(fd, CONFIGP1, 0xf3); 
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
    i2c_smbus_write_byte_data(fd, INVP0, 0xff);
    i2c_smbus_write_byte_data(fd, CONFIGP0, 0xff);

    i2c_smbus_write_byte_data(fd, OUTP1, 0x00);
    i2c_smbus_write_byte_data(fd, INVP1, 0x7f);
    i2c_smbus_write_byte_data(fd, CONFIGP1, 0x7f);
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
    i2c_smbus_write_byte_data(fd, INVP0, 0xf4);
    i2c_smbus_write_byte_data(fd, CONFIGP0, 0xf4);
        
    i2c_smbus_write_byte_data(fd, OUTP1, 0x00);
    i2c_smbus_write_byte_data(fd, CONFIGP1, 0x00);
    i2c_smbus_read_byte_data(fd, INP0);
    i2c_smbus_read_byte_data(fd, INP1);
    close(fd);
    return 0;
}

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
            printf("ans1:%d %d\n", x,y);
            first = I2CEXValue[0] ^ 0xff;
            first = first & x;
            
            second = I2CEXValue[1] ^ 0xff;
            second = second & y;
            
            I2CEXValue[0] = x;
            I2CEXValue[1] = y;
            
            for(forCount = 0; forCount < 8; ++forCount)
            {
                Count[forCount] = Count[forCount] + (first & 1);
                first = first >> 1;
                switch(forCount)
                {
                    case 0:
                        if((second & 1))
                        {
                            CutRoll[0] = 1;
                        }
                        break;
                    case 1:
                        if((second & 1))
                        {
                            Count[8+forCount] = Count[8+forCount] + CutRoll[0];
                            CutRoll[0] = 0;                            
                        }
                        break;
                    case 2:
                    case 3:
                        break;
                    case 4:
                        if((second & 1))
                        {
                            CutRoll[1] = 1;
                        }
                        break;
                    case 5:
                        if((second & 1))
                        {
                            Count[8+forCount] = Count[8+forCount] + CutRoll[1];
                            CutRoll[1] = 0;
                        }
                        break;
                    default:
                    Count[forCount+8] = Count[forCount+8] + (second & 1);
                }
                second = second >> 1;
            }
            WatchdogResetFlag = 1;
        }
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        //usleep(USLEEPTIMEUNITVERYSHORT);
        sleep(0);
    }
}
void * ZHI2cReaderFunction2(void *argument)
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
            printf("ans2:%d %d\n", x,y);
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
        //usleep(USLEEPTIMEUNITVERYSHORT);
        sleep(0);
    }
}
void * ZHI2cReaderFunction3(void *argument)
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
            printf("ans3:%d\n", x);
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
        //usleep(USLEEPTIMEUNITVERYSHORT);
        sleep(0);
    }
}

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
        TotalBadCount = ExCount[7] + ExCount[17] + ExCount[18] + ExCount[16]; 
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
