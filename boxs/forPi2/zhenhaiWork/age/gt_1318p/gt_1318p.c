#include "gt_1318p.h"

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
    //static struct  timeval changeIntoRepairmodeTimeStemp;
    struct ifreq ifr;
    int fd;
    InputNode *tempNode = ZHList;   
 
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, ZHNETWORKTYPE, IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    
    filePtr = fopen(ZHList->UploadFilePath, "a");
    //printf("%s open file %s\n", __func__, ZHList->UploadFilePath);
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
                        if(abs(Count[forCount] - ExCount[forCount]) > ZHMAXOUTPUT)
                        {
                            fprintf(filePtr, "%s %s %s -1 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachRUNNING);
                        }else
                        {
                            fprintf(filePtr, "%s %s %s %ld %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, 
                                                                Count[forCount] - ExCount[forCount], (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachRUNNING);
                        }
                    }else
                    {
                        fprintf(filePtr, "%s %s %s 0 %ld %ld %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                Count[forCount] - ExCount[forCount],
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                forCount+1, MachineNo, ZHList->UserNo, MachRUNNING);
                    }
                }else;
            }
        break;
        case MachREPAIRING:
            gettimeofday(&changeIntoRepairmodeTimeStemp, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n", 
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, 
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
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachJobDone);
        break;
        case MachLOCK:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachLOCK);          
        break;
        case MachUNLOCK:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachUNLOCK); 
        break;
        case MachSTOPForce1:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachSTOPForce1);        
        break;
        case MachSTOPForce2:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachSTOPForce2);        
        break;
        case MachSTART:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachSTART);
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
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, MachRESUMEFROMPOWEROFF);
        break;
        case MachPOWEROFF:
            gettimeofday(&now, NULL);
            fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                ZHList->ISNo, ZHList->ManagerCard, ZHList->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, ZHList->UserNo, MachPOWEROFF);
            sleep(1);     
            while(tempNode->link != NULL)
            {
                tempNode = tempNode->link;
                gettimeofday(&now, NULL);
                fprintf(filePtr, "%s %s %s 0 %ld 0 %s %d %s %s 0 0 0 %02d\n",
                                                                tempNode->ISNo, tempNode->ManagerCard, tempNode->CountNo, (long)now.tv_sec,
                                                                inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
                                                                GOODCOUNT+1, MachineNo, tempNode->UserNo, MachPOWEROFF);
                sleep(1);
            }
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
    char tempOutput[RS232Length];
    int charCount = 0;
    int stringCount = 0;
    int getString = 0;
    int readyToStart = 0;

    if((fd = serialOpen("/dev/ttyAMA0", 19200)) < 0)
    {
       printf("Unable to open serial device: %s\n", strerror(errno));
       pthread_exit((void*)"fail");
    }
    while(SerialFunctionFlag)
    {
        while(serialDataAvail(fd))
        {    
            char tempChar1 = serialGetchar(fd);
            if (tempChar1 == 0x7D && getString == 0 && readyToStart == 1)
            {
               //count plus one and memset
               //Count[TOTAL]++;
               memset(tempOutput, 0, sizeof(char)*RS232Length);
               stringCount = 0;
            }
            else if (tempChar1 == 0x7B)
            {
                //default
                memset(tempOutput,0, sizeof(char)*RS232Length);
                getString = 1;
                Count[TOTAL]++;
                tempOutput[stringCount] = tempChar1;
                stringCount++;
                readyToStart = 1;
            }
            else if(tempChar1 == 0x7D && getString == 1)
            {
                //package
                if(stringCount == 48)
                {
                    pthread_mutex_lock(&MutexSerial);
                    
                    memset(FileOutput, 0, sizeof(char)*RS232Length);
                    memcpy(FileOutput, tempOutput, stringCount);
               
                    UpdateFlag = 1; 
                    pthread_mutex_unlock(&MutexSerial);
                }
                memset(tempOutput, 0, sizeof(char)*RS232Length);
                stringCount = 0;
                getString = 0;
            }
            else if( getString == 1 && stringCount < 48)
            {
                tempOutput[stringCount] = tempChar1;
                stringCount++;
            }
            else;
            fflush (stdout) ;
        }
    }
    if(fd >= 0)
    {
        serialClose(fd);
    }
    
}

void * WatchdogFunction(void *argument)
{
    struct timeval now;
    struct timespec outtime;
    struct ifreq ethreq;
    
    int fd2;
    int writeFileCount = 0;
    char tempFileOutput[RS232Length];
    char configParameter[18];
    unsigned char countArray[9];

    float lcCheck, dxUpperBond;
    double cxLowerBond, cxUpperBond, cxCheck;

    int forCount = 0;
    short errorCheckCount[3];
    char result[3]; //remeber last stage 

    lcCheck = dxUpperBond = 0;
    cxUpperBond = cxCheck = dxUpperBond = 0;

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
            char resetParameterFlag = 0;
            memset(tempFileOutput, 0, sizeof(char)*RS232Length);
            pthread_mutex_lock(&MutexSerial);
            for(forCount = 0; forCount < RS232Length; forCount++)
            {
                tempFileOutput[forCount] = FileOutput[forCount];
            }
            UpdateFlag = 0;
            pthread_mutex_unlock(&MutexSerial);

            for(forCount = 0; forCount < 18; forCount++)
            {
                if(configParameter[forCount] != tempFileOutput[forCount])
                {
                    resetParameterFlag = 1;
                    break;
                }
            }
            if(resetParameterFlag ==1)
            {
                memcpy(configParameter, tempFileOutput, 18);
                lcCheck = LCSetter(configParameter[1], configParameter[2], configParameter[3], configParameter[4]);
                cxCheck = CXSetter(configParameter[12], configParameter[13], configParameter[14]);
                cxUpperBond = cxCheck * CXUpBoundSetter(configParameter[9], configParameter[10], configParameter[11]);
                cxLowerBond = cxCheck * CXLowBoundSetter(configParameter[6], configParameter[7], configParameter[8]);
                dxUpperBond = DXSetter(configParameter[15], configParameter[16], configParameter[17]);
            }else;
            double cx = CXResult(tempFileOutput[25], tempFileOutput[26], tempFileOutput[27], tempFileOutput[28], 
                            tempFileOutput[29], tempFileOutput[30]);
            float dx = DXResult(tempFileOutput[31], tempFileOutput[32], tempFileOutput[33], tempFileOutput[34], tempFileOutput[35]);
            float lc = LCResult(tempFileOutput[36], tempFileOutput[37], tempFileOutput[38], tempFileOutput[39], tempFileOutput[40]);
#ifdef DEBUG 
            printf("CX: %f %f %f\n", cxLowerBond, cx, cxUpperBond);
            printf("DX: %f %f\n", dxUpperBond, dx);
            printf("LC: %f %f\n", lcCheck, lc);
#endif
            if(lc >= lcCheck || lc < 0)
            {
                Count[LCBAD] += 1;
            }else if(cx >= cxUpperBond || cx <= cxLowerBond || cx < 0)
            {
                Count[CXBAD] = Count[CXBAD] + 1;
            }else if(dx >= dxUpperBond)
            {
                Count[DXBAD] = Count[DXBAD] + 1;
            }else if(dx < 0 && dx != -3)
            {
                Count[DXBAD] = Count[DXBAD] + 1;
            }else
            {
                Count[GOODCOUNT] = Count[GOODCOUNT] + 1;
            }
        }
        writeFileCount = (writeFileCount + WATCHDOGPERIOD) % WRITEFILETIMER;   
        //printf("%d\n", writeFileCount);
        if(writeFileCount == 0 || WatchdogThreadFlag == 0)
        {   
            //[vers|2014.10.25|end] 
            //FILE I/O
            pthread_mutex_lock(&MutexFile);
            WriteFile(MachRUNNING);
            pthread_mutex_unlock(&MutexFile);

            memcpy(ExCount, Count, sizeof(unsigned long)*EVENTSIZE);
            TotalBadCount = ExCount[CXBAD] + ExCount[DXBAD] + ExCount[LCBAD];

            if(ScreenIndex == 1 && WatchdogThreadFlag != 0)
            {
                pthread_mutex_lock(&MutexScreen);
                UpdateScreenFunction(1, 0);  
                pthread_mutex_unlock(&MutexScreen);
            } 
            printf("%s %s %s %s %s|| %ld %ld %ld %ld %ld\n", 
                ZHList->ISNo, ZHList->ManagerCard, MachineNo, ZHList->UserNo, ZHList->CountNo, 
                                Count[GOODCOUNT], Count[CXBAD], Count[DXBAD], Count[LCBAD], Count[TOTAL]);

      
            //check network status
            /*
            fd2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            memset(&ethreq, 0, sizeof(ethreq));
            strncpy(ethreq.ifr_name, ZHNETWORKTYPE, IFNAMSIZ);
            ioctl(fd2,  SIOCGIFFLAGS, &ethreq);
        
            if(ethreq.ifr_flags & IFF_RUNNING)
            {
                //connect
            }else
            {
                //disconnect
            }
            close(fd2);
            */
        }       
    }
}

double CXResult(char CX1, char CX2, char CX3, char CX4, char CX5, char CX6)
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
        char a1, a2, a3, a4, a5;
        
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

float DXResult(char DS1, char DS2, char DS3, char DS4, char DS5)
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

float LCResult(char LC1, char LC2, char LC3, char LC4, char LC5)
{
    char a1,a2,a3,a4;    
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

float LCSetter(char LC1, char LC2, char LC3, char LC4) 
{
    float ans = 0;
    LC1 = (LC1 & 0x0f);
    LC2 = (LC2 & 0x0f);
    LC3 = (LC3 & 0x0f);
    LC4 = (LC4 & 0x0f);
    ans = ((float)LC1) * 100 + ((float)LC2) *10 + ((float)LC3) + ((float)LC4)* 0.1;
    return ans;
}

double CXSetter(char CX1, char CX2, char CX3)
{
    double ans = 0;
    CX1 = (CX1 & 0x0f);
    CX2 = (CX2 & 0x0f);
    CX3 = (CX3 & 0x0f);
    ans = (((double)CX1) * 10 + ((double)CX2)) * pow(10 ,(double)CX3);
    return ans;
}

double CXUpBoundSetter(char CX1, char CX2, char CX3)
{
    double ans = 0;
    CX1 = (CX1 & 0x0f);
    CX2 = (CX2 & 0x0f);
    CX3 = (CX3 & 0x0f);
    ans = 1 + ((double)CX1) * 0.1 + ((double)CX2) * 0.01 + ((double)CX3) * 0.001;
    return ans;
}

double CXLowBoundSetter(char CX1, char CX2, char CX3)
{
    double ans = 0;
    CX1 = (CX1 & 0x0f);
    CX2 = (CX2 & 0x0f);
    CX3 = (CX3 & 0x0f);
    ans = 1 - ((double)CX1)*0.1 - ((double)CX2) * 0.01 - ((double)CX3) * 0.001;
    return ans;
}

float DXSetter(char DX1, char DX2, char DX3)
{
    float ans = 0;
    DX1 = (DX1 & 0x0f);
    DX2 = (DX2 & 0x0f);
    DX3 = (DX3 & 0x0f);
    ans = ((float)DX1) *10 + ((float)DX2) + ((float)DX3)*0.1;
    return ans;
}
