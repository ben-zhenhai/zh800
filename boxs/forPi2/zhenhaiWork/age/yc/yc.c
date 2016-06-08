#include "yc.h"

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

void * ZHSerialFunction(void * argument)
{
    int fd;
    char tempOutput[RS232Length];
    int stringCount = 0;
    //int getString = 0;
    int readyToStart = 0;

    if((fd = serialOpen("/dev/ttyAMA0", 9600)) < 0)
    {
        printf("Unable to open serial device: %s\n", strerror(errno));
        pthread_exit((void*)"fail");
    }
    
    while(SerialFunctionFlag)
    {
        while(serialDataAvail(fd))
        {
            char tempChar = serialGetchar(fd);
            printf("%x\n", tempChar);
            if(stringCount == 68)
            {
                pthread_mutex_lock(&MutexSerial);
                memset(FileOutput, 0, sizeof(char)*RS232Length);
                memcpy(FileOutput, tempOutput, stringCount);
                
                UpdateFlag = 1;
                pthread_mutex_unlock(&MutexSerial);
                memset(tempOutput, 0, sizeof(char)*RS232Length);
                readyToStart = 0;
                stringCount = 0;
            }else if(tempChar == 0x01 && readyToStart == 0 && stringCount == 0)
            {
                tempOutput[stringCount] = tempChar;
                readyToStart = 1;
                stringCount = 1;
            }else if(readyToStart == 1 && stringCount == 2)
            {
                if(tempChar == 0x44)
                { 
                    tempOutput[stringCount] = tempChar;
                    stringCount++;
                }else
                {
                    memset(tempOutput, 0, sizeof(char)*RS232Length);
                    readyToStart = 0;
                    stringCount = 0;
                }
            }else if(readyToStart == 1 && (stringCount >= 1)&& (stringCount <68))
            {
                tempOutput[stringCount] = tempChar;
                stringCount++;
            }else
            {
                memset(tempOutput, 0, sizeof(char)*RS232Length);
                readyToStart = 0;
                stringCount = 0;
            }
        }
    }
    if(fd >= 0) serialClose(fd);
}

void * WatchdogFunction(void *argument)
{
    //struct timeval now;
    //struct timespec outtime;
    ReceivePack receivePack;
    char tempOutput[RS232Length];
    int checkResult = 0;
    int forCount = 0;

    while(WatchdogThreadFlag)
    {
        memset(&receivePack, 0, sizeof(ReceivePack));
        memset(tempOutput, 0, sizeof(char)*RS232Length);

        if(UpdateFlag == 1)
        {
            pthread_mutex_lock(&MutexSerial);
            memcpy(tempOutput, FileOutput, RS232Length);
            //printf("ready to printf\n");
            int vers = 0;
            for(vers = 0; vers < 68; vers++)
            {
                printf("%x ", tempOutput[vers]);
            }
            printf("\n");
            if(CheckSum() == 1)checkResult = 1;
            UpdateFlag = 0;
            pthread_mutex_unlock(&MutexSerial);

            if(checkResult)
            {
                receivePack.addr = tempOutput[0];
                receivePack.cmd = tempOutput[1];
                receivePack.length = tempOutput[2];
                receivePack.checksum = tempOutput[3];
                receivePack.capHiDecimal = tempOutput[4];
                receivePack.capLowDecimal = tempOutput[5];
                receivePack.capHiDecimal_2 = tempOutput[6];
                receivePack.capLowDecimal_2 = tempOutput[7];
                receivePack.currHiDecimal = tempOutput[8];
                receivePack.currLowDecimal = tempOutput[9];
                receivePack.dHiDecimal = tempOutput[10];
                receivePack.currDecimal_1 = tempOutput[11];
                receivePack.currDecimal_2 = tempOutput[12];
                receivePack.fetState = tempOutput[13];
                receivePack.capDecimal = tempOutput[14];
                receivePack.dDecimal = tempOutput[15];
                receivePack.capHi =(int)(tempOutput[16] + 256 * tempOutput[17]);
                receivePack.capLow = (int)(tempOutput[18] + 256 * tempOutput[19]);
                receivePack.capHi_2 = (int)(tempOutput[20] + 256 * tempOutput[21]);
                receivePack.capLow_2 = (int)(tempOutput[22] + 256 * tempOutput[23]);
                receivePack.currHi = (int)(tempOutput[24] + 256 * tempOutput[25]);
                receivePack.currLow = (int)(tempOutput[26] + 256 * tempOutput[27]);
                receivePack.dHi = (int)(tempOutput[28] + 256 * tempOutput[29]);
                receivePack.capOver = (int)(tempOutput[30] + 256 * tempOutput[31]);
                receivePack.capLess = (int)(tempOutput[32] + 256 * tempOutput[33]);
                receivePack.damageOver = (int)(tempOutput[34] + 256 * tempOutput[35]);
                receivePack.leakage = (int)(tempOutput[36] + 256 * tempOutput[37]);
                receivePack.noLoad = (int)(tempOutput[38] + 256 * tempOutput[39]);
                receivePack.shortCircuit = (int)(tempOutput[40] + 256 * tempOutput[41]);
                receivePack.contrary = (int)(tempOutput[42] + 256 * tempOutput[43]);
                receivePack.adValue1 = (int)(tempOutput[44] + 256 * tempOutput[45]);
                receivePack.adValue2 = (int)(tempOutput[46] + 256 * tempOutput[47]);
                receivePack.capValue = (int)(tempOutput[48] + 256 * tempOutput[49]);
                receivePack.dValue = (int)(tempOutput[50] + 256 * tempOutput[51]);
                receivePack.noTest = (int)(tempOutput[52] + 256 * tempOutput[53]);
                receivePack.noUser = (int)(tempOutput[54] + 256 * tempOutput[55]);
                receivePack.totalCount = (long)(tempOutput[56] + 256 * tempOutput[57] + (256^2) * tempOutput[58] + (256^3) * tempOutput[59]);
                receivePack.capPass = (long) (tempOutput[60] + 256 * tempOutput[61] + (256^2) * tempOutput[62] + (256^3) * tempOutput[63]);
                receivePack.capPass_2 = (long) (tempOutput[64] + 256 * tempOutput[65] + (256^2) * tempOutput[66] + (256^3) * tempOutput[67]); 
           
                float capValue = ShiftDecimalFunction(receivePack.capValue, (int)receivePack.capDecimal);
                float adValue1 = ShiftDecimalFunction(receivePack.adValue1, (int)receivePack.currDecimal_1);
                float adValue2 = ShiftDecimalFunction(receivePack.adValue2, (int)receivePack.currDecimal_2);
                float df = ShiftDFDecimalFunction(receivePack.dValue, (int)receivePack.dDecimal);
                int frequency = SetFrequencyFunction(receivePack.fetState);
                float capMax = ShiftDecimalFunction(receivePack.capHi, (int)receivePack.capHiDecimal);
                float capMin = ShiftDecimalFunction(receivePack.capLow, (int)receivePack.capLowDecimal);
                float dfMax = ShiftDecimalFunction(receivePack.dHi, (int)receivePack.dHiDecimal);
                float lcMax = ShiftDecimalFunction(receivePack.currHi, (int)receivePack.currHiDecimal);
                float lcMin = ShiftDecimalFunction(receivePack.currLow, (int)receivePack.currLowDecimal);

                //if(capValue > capMax) Count[HIGHCAPCOUNT]++;
                //else if(capValue < capMin) Count[LOWCAPCOUNT]++;
                //else if(df > dfMax) Count[DXCOUNT]++;
                //else Count[GOODCOUNT]++;
                
                Count[HIGHCAPCOUNT] = receivePack.capOver;
                Count[LOWCAPCOUNT] = receivePack.capLess;
                Count[DXCOUNT] = receivePack.damageOver;
                Count[LEAKCOUNT] = receivePack.leakage;
                Count[NOLOADCOUNT] = receivePack.noLoad;
                Count[SHORTCOUNT] = receivePack.shortCircuit;
                Count[CONTRARYCOUNT] = receivePack.contrary;
                //Count[NOAGECOUNT] = receivePack.noTest;
                Count[TOTALCOUNT] = receivePack.totalCount;
                Count[GOODCOUNT] = receivePack.capPass; 
            }
            pthread_mutex_lock(&MutexFile);
            WriteFile(MachRUNNING);
            pthread_mutex_unlock(&MutexFile);

            memcpy(ExCount, Count, sizeof(unsigned long)*EVENTSIZE);
            TotalBadCount =  ExCount[HIGHCAPCOUNT] + ExCount[LOWCAPCOUNT] + ExCount[DXCOUNT] + ExCount[LEAKCOUNT] + ExCount[NOLOADCOUNT]
                                                    + ExCount[SHORTCOUNT] + ExCount[CONTRARYCOUNT]; //+ ExCount[NOAGECOUNT]; 


            for(forCount = 0; forCount < EVENTSIZE; forCount++)
            {
                printf("%d:%ld\n" , forCount, ExCount[forCount]);
            }
            printf("bad: %ld\n", TotalBadCount);
            if(ScreenIndex == 1 && WatchdogThreadFlag != 0)
            {
                pthread_mutex_lock(&MutexScreen);
                UpdateScreenFunction(1, 0);  
                pthread_mutex_unlock(&MutexScreen);
            } 


        }
    }
}

int CheckSum()
{
    char Sum = 0x00;
    int forCount = 0;
    char crcResult = FileOutput[3];
    FileOutput[3] = 0;
    for(forCount = 0; forCount < 70; forCount++)
    {
        Sum += FileOutput[forCount];
    }
    Sum = 0xff - Sum;
    printf("%s %x %x\n", __func__, Sum, crcResult);
    if(Sum == crcResult) return 1;
    else return 0;
}

float ShiftDecimalFunction(int value, int decimal)
{
    int forCount = 0;
    float returnValue = 0;
    returnValue = (float)value;
    for(forCount = 0; forCount < decimal; forCount++)
    {
        returnValue /= 10;
    }
    return returnValue;
}

int SetFrequencyFunction(char value)
{
    if(value == 0) return 120;
    else return 1000;
}

float ShiftDFDecimalFunction(int value, int decimal)
{
    float returnValue = 0;
    int forCount = 0;
    int localDecimal = 0;
    
    if(decimal == 0 || decimal > 5) localDecimal = 5;
    else localDecimal = decimal;
    returnValue = (float)value;

    for(forCount = 0; forCount < localDecimal; forCount++)
    {
        returnValue /= 10;
    }
    return returnValue;
}
