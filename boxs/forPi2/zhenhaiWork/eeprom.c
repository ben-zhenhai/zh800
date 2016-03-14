#include "eeprom.h"

int ZHEarseRepairStage()
{
    int fd, r;
    unsigned char eraseArray[32];

    memset(eraseArray, 0, sizeof(char)*32);

    fd = open(I2CDEVICEADDR, O_RDWR);
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
    if(r < 0)
    {
        perror("Selecting i2c device");
        return 1;
    }
   
    //i2c_smbus_write_byte_data(fd, 0x40, 0x00);

    printf("start earse eeprom\n");
 
    //eraseArray[0] = 0x00;
    //i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    //nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    
    eraseArray[0] = 0x00;
    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        

    eraseArray[0] = 0x20;
    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    close(fd);

    printf("finish earse repair stage\n");
    return 0;
}

int WriteRepairStageEEPROMDATA()
{
    int fd, r, forCount;
    int retryMax = 3;
    unsigned char tempRepairNo[30];
    unsigned char arraySize;

    char eepromRepairNo[INPUTLENGTH];
    char eepromRepairStage[INPUTLENGTH];
    char *ptr;

    memset(eepromRepairNo, 0, sizeof(char)*INPUTLENGTH);
    memset(eepromRepairStage, 0, sizeof(char)*INPUTLENGTH);

    ptr = &eepromRepairNo[1];
    eepromRepairNo[0] = 0x00;
    //eepromManagerCard[0] = 0x00;
    memcpy(ptr, RepairNo, sizeof(char)*strlen(RepairNo));
    
    ptr = &eepromRepairStage[1];
    eepromRepairStage[0] = 0x20;
    memcpy(ptr, IntoRepairModeTimeStamp, sizeof(char)*strlen(IntoRepairModeTimeStamp));

#ifdef Debug
    printf("[%s %d]1: %s %s %d\n", __func__, __LINE__, ptr, RepairNo, strlen(RepairNo)+1);
#endif
    while(retryMax > 0)
    {

        fd = open(I2CDEVICEADDR, O_RDWR);
        if(fd < 0)
        {
            perror("Open Fail");
            return 1;
        }
        r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
        if(r < 0)
        {
            perror("Selecting i2c device");
            return 1;
        }

        i2c_smbus_write_i2c_block_data(fd, 0x00, strlen(RepairNo)+1, &eepromRepairNo[0]);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
        i2c_smbus_write_i2c_block_data(fd, 0x00, strlen(IntoRepairModeTimeStamp)+1, &eepromRepairStage[0]);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
        //i2c_smbus_write_byte_data(fd, 0x30, 0x11);
        close(fd);

        printf("Write Repair mode to eeprom done....\n"); 

        //check eeprom
        memset(tempRepairNo, 0, sizeof(char)*30);

        usleep(USLEEPTIMEUNITLONG);
        fd = open(I2CDEVICEADDR, O_RDWR);
        if(fd < 0)
        {
            perror("Open Fail");
            return 1;
        }
        r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
        if(r < 0)
        {
            perror("Selecting i2c device");
            return 1;
        }

        //arraySize = 29;
        arraySize = 4;
        i2c_smbus_write_byte_data(fd, 0x00, 0x00);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempRepairNo[forCount] = i2cChar;
            }else
            {
                break;   
            }
        }

        //if(forCount < 29 || strncmp(tempRepairNo, RepairNo, strlen(RepairNo)) != 0)
        if(forCount < 4 || strncmp(tempRepairNo, RepairNo, strlen(RepairNo)) != 0)
        {
            printf("Repair no error\n");
            //ZHEarseEEPROMData();
            close(fd);
            retryMax--;
            sleep(1);
            continue;
        }
        //tempRepairNo[29] = 0x00;
        tempRepairNo[4] = 0x00;
    
        close(fd);
        return 0;
    }
    return 1;
}

int IsResumeFromRepair()
{
    int fd, r, forCount;
    char repairStage[10]; //REPAIRING
    char arraySize;
    char tempRepairNo[INPUTLENGTH];

    memset(tempRepairNo, 0, sizeof(char)*INPUTLENGTH);
    memset(repairStage, 0, sizeof(char)*10);
    fd = open(I2CDEVICEADDR, O_RDWR);
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
    if(r < 0)
    {
        perror("Selecting i2c device");
        return 1;
    }

    arraySize = 10;
    i2c_smbus_write_byte_data(fd, 0x00, 0x20);
    for(forCount = 0; forCount < arraySize; forCount++)
    {
        char i2cChar = i2c_smbus_read_byte(fd);
        if(i2cChar != 0x00 && i2cChar != 0xff)
        {
            repairStage[forCount] = i2cChar;
        }else
        {
            break;
        }
    }
    if(forCount < 10)
    {
        printf("not Resume from REPAIR\n");
        close(fd);
        return 1;
    }
    repairStage[11] = 0x00;

    //arraySize = 29;
    arraySize = 4;
    i2c_smbus_write_byte_data(fd, 0x00, 0x00);
    for(forCount = 0; forCount < arraySize; forCount++)
    {
        char i2cChar = i2c_smbus_read_byte(fd);
        if(i2cChar != 0x00 && i2cChar != 0xff)
        {
            tempRepairNo[forCount] = i2cChar;
        }else
        {
            break;
        }
    }
    //if(forCount < 29)
    if(forCount < 4)
    {
        printf("eeprom read lot no error\n");
        close(fd);
        return 1;
    }
    //tempRepairNo[29] = 0x00;
    tempRepairNo[4] = 0x00;

    close(fd);
    memset(RepairNo, 0, sizeof(char)*INPUTLENGTH);
    memcpy(RepairNo, tempRepairNo, sizeof(char)*30);
#ifdef DEBUG
    printf("[%s|%d]exit\n", __func__, __LINE__);
#endif 
    return 0;
}

int ReadLastFinishLotNo()
{
    int fd, r, forCount;
    char tempLotNo[30];
    char arraySize;

    memset(tempLotNo, 0, sizeof(char)*30);
    fd = open(I2CDEVICEADDR, O_RDWR);
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
    if(r < 0)
    {
        perror("Selecting i2c device");
        return 1;
    }

    arraySize = 14;
    i2c_smbus_write_byte_data(fd, 0x00, 0x30);
    for(forCount = 0; forCount < arraySize; forCount++)
    {
        char i2cChar = i2c_smbus_read_byte(fd);
        if(i2cChar != 0x00 && i2cChar != 0xff)
        {
            tempLotNo[forCount] = i2cChar;
        }else
        {
            break;
        }
    }
    if(forCount < 13)
    {
        printf("eeprom read lot no error\n");
        close(fd);
        return 1;
    }
    tempLotNo[14] = 0x00;

    close(fd);

    //copy to correct array
    memset(LastISNo, 0, sizeof(char)*INPUTLENGTH);
    memcpy(LastISNo, tempLotNo, sizeof(char)*30);
   
#ifdef DEBUG
    printf("[%s|%d]", __func__, __LINE__);
#endif 
    printf("%s\n", LastISNo);
    return 0;
}

int WriteLastFinishLotNo()
{
    int fd, r, forCount;
    int retryMax = 3;
    unsigned char tempLotNo[30];
    unsigned char arraySize;

    char eepromISNo[INPUTLENGTH];
    char *ptr;

    memset(eepromISNo, 0, sizeof(char)*INPUTLENGTH);

    ptr = &eepromISNo[1];
    eepromISNo[0] = 0x30;
    memcpy(ptr, ISNo, sizeof(char)*strlen(ISNo));
    printf("[%s %d]2: %s %s %d\n", __func__, __LINE__, ptr,ISNo, strlen(ISNo)+1);

    while(retryMax > 0)
    {
        fd = open(I2CDEVICEADDR, O_RDWR);
        if(fd < 0)
        {
            perror("Open Fail");
            return 1;
        }
        r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
        if(r < 0)
        {
            perror("Selecting i2c device");
            return 1;
        }

        i2c_smbus_write_i2c_block_data(fd, 0x00, strlen(ISNo)+1, &eepromISNo[0]);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
        close(fd);

#ifdef DEBUG
        //check data by log
        for(forCount = 0; forCount < strlen(ISNo) + 1; forCount++)
        {
            if(forCount == 0)
            {
                printf("%x ", eepromISNo[forCount]);
            }else
            {
                printf("%c", eepromISNo[forCount]);
            }
        }
        printf("\n");
#endif

        printf("Write eeprom done....\n"); 

        //check eeprom
        memset(tempLotNo, 0, sizeof(char)*30);

        usleep(USLEEPTIMEUNITLONG);
        fd = open(I2CDEVICEADDR, O_RDWR);
        if(fd < 0)
        {
            perror("Open Fail");
            return 1;
        }
        r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
        if(r < 0)
        {
            perror("Selecting i2c device");
            return 1;
        }
        arraySize = 14;
        i2c_smbus_write_byte_data(fd, 0x00, 0x30);
        //i2c_smbus_write_byte_data(fd, 0x00, 0x20);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempLotNo[forCount] = i2cChar;
            }else
            {
                break;
            }
        }
        if(forCount < 13 || strncmp(tempLotNo, ISNo, strlen(ISNo)) != 0)
        {
            printf("lot no error\n");
            close(fd);
            retryMax--;
            sleep(1);
            continue;
        }
        tempLotNo[14] = 0x00;

        close(fd);
        return 0;
    }
    return 1;
}

int ZHEarseEEPROMData()
{
    int fd, r;
    unsigned char eraseArray[32];

    memset(eraseArray, 0, sizeof(char)*32);

    fd = open(I2CDEVICEADDR, O_RDWR);
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
    if(r < 0)
    {
        perror("Selecting i2c device");
        return 1;
    }
   
    //i2c_smbus_write_byte_data(fd, 0x40, 0x00);

    printf("start earse eeprom\n");
 
    //eraseArray[0] = 0x00;
    //i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    //nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    
    eraseArray[0] = 0x40;
    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        

    eraseArray[0] = 0x60;
    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        

    eraseArray[0] = 0x80;
    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    
    eraseArray[0] = 0xa0;
    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    
    eraseArray[0] = 0xc0;
    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    close(fd);

    printf("finish earse eeprom\n");
    return 0;
}

int ReadEEPROMData()
{
    int fd, r, forCount;
    char tempLotNo[30];
    char tempManagerCard[30];
    char tempCountNo[30];
    char tempGoodNo[30];
    char arraySize;

    memset(tempLotNo, 0, sizeof(char)*30);
    memset(tempManagerCard, 0, sizeof(char)*30);
    memset(tempCountNo, 0, sizeof(char)*30);
    memset(tempGoodNo, 0, sizeof(char)*30);

    fd = open(I2CDEVICEADDR, O_RDWR);
    if(fd < 0)
    {
        perror("Open Fail");
        return 1;
    }
    r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
    if(r < 0)
    {
        perror("Selecting i2c device");
        return 1;
    }

    arraySize = 24;
    //i2c_smbus_write_byte_data(fd, 0x00, 0x00);
    i2c_smbus_write_byte_data(fd, 0x00, 0x40);
    for(forCount = 0; forCount < arraySize; forCount++)
    {
        char i2cChar = i2c_smbus_read_byte(fd);
        if(i2cChar != 0x00 && i2cChar != 0xff)
        {
            tempManagerCard[forCount] = i2cChar;
        }else
        {
            break;   
        }
    }
    if(forCount < 23)
    {
        printf("part no error\n");
        close(fd);
        return 1;
    }
    tempManagerCard[24] = 0x00;
    
    arraySize = 14;
    //i2c_smbus_write_byte_data(fd, 0x00, 0x20);
    i2c_smbus_write_byte_data(fd, 0x00, 0x60);
    for(forCount = 0; forCount < arraySize; forCount++)
    {
        char i2cChar = i2c_smbus_read_byte(fd);
        if(i2cChar != 0x00 && i2cChar != 0xff)
        {
            tempLotNo[forCount] = i2cChar;
        }else
        {
            break;
        }
    }
    if(forCount < 13)
    {
        printf("lot no error\n");
        close(fd);
        return 1;
    }
    tempLotNo[14] = 0x00;

    arraySize = 10;
    //i2c_smbus_write_byte_data(fd, 0x00, 0x30);
    i2c_smbus_write_byte_data(fd, 0x00, 0x70);
    for(forCount = 0; forCount < arraySize; forCount++)
    {
        char i2cChar = i2c_smbus_read_byte(fd);
        if(i2cChar != 0x00 && i2cChar != 0xff)
        {
            tempCountNo[forCount] = i2cChar;
        }else
        {
            tempCountNo[forCount] = 0x00;
            break;
        }
    }
    if(forCount == 0)
    {
        printf("Good Count error\n");
        close(fd);
        return 1;
    }
    
    //i2c_smbus_write_byte_data(fd, 0x00, 0x40);
    i2c_smbus_write_byte_data(fd, 0x00, 0x80);
    for(forCount = 0; forCount < arraySize; forCount++)
    {
        char i2cChar = i2c_smbus_read_byte(fd);
        if(i2cChar != 0x00 && i2cChar != 0xff)
        {
            tempGoodNo[forCount] = i2cChar;
        }else
        {
            tempGoodNo[forCount] = 0x00;
            break;
        }
    }
    close(fd);

    //copy to correct array
    memset(ISNo, 0, sizeof(char)*INPUTLENGTH);
    memset(ManagerCard, 0, sizeof(char)*INPUTLENGTH);
    memset(CountNo, 0, sizeof(char)*INPUTLENGTH);
    memset(GoodNo, 0, sizeof(char)*INPUTLENGTH);
    memcpy(ISNo, tempLotNo, sizeof(char)*30);
    memcpy(ManagerCard, tempManagerCard, sizeof(char)*30);
    memcpy(CountNo, tempCountNo, sizeof(char)*30);
    memcpy(GoodNo, tempGoodNo, sizeof(char)*30);
   
#ifdef DEBUG
    printf("[%s|%d]", __func__, __LINE__);
#endif 
    printf("%s %s %s %s \n", ISNo, ManagerCard, CountNo, GoodNo);
    return 0;
}

int WriteEEPROMData()
{
    int fd, r, forCount;
    int retryMax = 3;
    unsigned char tempLotNo[30];
    unsigned char tempManagerCard[30];
    unsigned char tempCountNo[30];
    unsigned char tempGoodNo[30];
    unsigned char arraySize;

    char eepromISNo[INPUTLENGTH];
    char eepromManagerCard[INPUTLENGTH];
    char eepromCountNo[INPUTLENGTH]; 
    char eepromGoodNo[INPUTLENGTH];
    char *ptr;

    memset(eepromISNo, 0, sizeof(char)*INPUTLENGTH);
    memset(eepromManagerCard, 0, sizeof(char)*INPUTLENGTH);
    memset(eepromCountNo, 0, sizeof(char)*INPUTLENGTH);
    memset(eepromGoodNo,0, sizeof(char)*INPUTLENGTH);

    //0x00 0x20 0x30 0x40 
    ptr = &eepromManagerCard[1];
    eepromManagerCard[0] = 0x40;
    //eepromManagerCard[0] = 0x00;
    memcpy(ptr, ManagerCard, sizeof(char)*strlen(ManagerCard));
#ifdef Debug
    printf("[%s %d]1: %s %s %d\n", __func__, __LINE__, ptr, ManagerCard, strlen(ManagerCard)+1);
#endif

    ptr = &eepromISNo[1];
    eepromISNo[0] = 0x60;
    //eepromISNo[0] = 0x20;
    memcpy(ptr, ISNo, sizeof(char)*strlen(ISNo));
#ifdef Debug
    printf("[%s %d]2: %s %s %d\n", __func__, __LINE__, ptr,ISNo, strlen(ISNo)+1);
#endif

    ptr = &eepromCountNo[1];
    eepromCountNo[0] = 0x70;
    //eepromCountNo[0] = 0x30;
    memcpy(ptr, CountNo, sizeof(char)*strlen(CountNo));
#ifdef Debug
    printf("[%s %d]3: %s %s %d\n", __func__, __LINE__, ptr, CountNo, strlen(CountNo)+1);
#endif

    ptr = &eepromGoodNo[1];
    eepromGoodNo[0] = 0x80;
    //eepromGoodNo[0] = 0x40;
    memcpy(ptr, GoodNo, sizeof(char)*strlen(GoodNo));
#ifdef Debug
    printf("[%s %d]4: %s %s %d\n", __func__, __LINE__, ptr, GoodNo, strlen(GoodNo)+1);
#endif

    while(retryMax > 0)
    {

        fd = open(I2CDEVICEADDR, O_RDWR);
        if(fd < 0)
        {
            perror("Open Fail");
            return 1;
        }
        r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
        if(r < 0)
        {
            perror("Selecting i2c device");
            return 1;
        }

        i2c_smbus_write_i2c_block_data(fd, 0x00, strlen(ManagerCard)+1, &eepromManagerCard[0]);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
        i2c_smbus_write_i2c_block_data(fd, 0x00, strlen(ISNo)+1, &eepromISNo[0]);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
        i2c_smbus_write_i2c_block_data(fd, 0x00, strlen(CountNo)+1, &eepromCountNo[0]);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
        i2c_smbus_write_i2c_block_data(fd, 0x00, strlen(GoodNo)+1, &eepromGoodNo[0]);
        nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
        //i2c_smbus_write_byte_data(fd, 0x30, 0x11);
        close(fd);

#ifdef DEBUG
        //check data by log
        for(forCount = 0; forCount < strlen(ISNo) + 1; forCount++)
        {
            if(forCount == 0)
            {
                printf("%x ", eepromISNo[forCount]);
            }else
            {
                printf("%c", eepromISNo[forCount]);
            }
        }
        printf("\n");

        for(forCount = 0; forCount < strlen(ManagerCard) + 1; forCount++)
        {
            if(forCount == 0)
            {
                printf("%x ", eepromManagerCard[forCount]);
            }else
            {
                printf("%c", eepromManagerCard[forCount]);
            }
        }
        printf("\n");

        for(forCount = 0; forCount < strlen(CountNo) + 1; forCount++)
        {
            if(forCount == 0)
            {
                printf("%x ", eepromCountNo[forCount]);
            }else
            {
                printf("%c", eepromCountNo[forCount]);
            }
        }
        printf("\n");

        for(forCount = 0; forCount < strlen(GoodNo) + 1; forCount++)
        {
            if(forCount == 0)
            {
                printf("%x ", eepromGoodNo[forCount]);
            }else
            {
                printf("%c ", eepromGoodNo[forCount]);
            }
        }
        printf("\n");
#endif
        printf("Write eeprom done....\n"); 

        //check eeprom
        memset(tempLotNo, 0, sizeof(char)*30);
        memset(tempManagerCard, 0, sizeof(char)*30);
        memset(tempCountNo, 0, sizeof(char)*30);
        memset(tempGoodNo, 0, sizeof(char)*30);

        usleep(USLEEPTIMEUNITLONG);
        fd = open(I2CDEVICEADDR, O_RDWR);
        if(fd < 0)
        {
            perror("Open Fail");
            return 1;
        }
        r = ioctl(fd, I2C_SLAVE, EEPROMADDRESS);
        if(r < 0)
        {
            perror("Selecting i2c device");
            return 1;
        }

        arraySize = 24;
        //i2c_smbus_write_byte_data(fd, 0x00, 0x00);
        i2c_smbus_write_byte_data(fd, 0x00, 0x40);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempManagerCard[forCount] = i2cChar;
            }else
            {
                break;   
            }
        }

        if(forCount < 23 || strncmp(tempManagerCard, ManagerCard, strlen(ManagerCard)) != 0)
        {
            printf("part no error\n");
            close(fd);
            retryMax--;
            sleep(1);
            continue;
        }
        tempManagerCard[24] = 0x00;
    
        arraySize = 14;
        i2c_smbus_write_byte_data(fd, 0x00, 0x60);
        //i2c_smbus_write_byte_data(fd, 0x00, 0x20);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempLotNo[forCount] = i2cChar;
            }else
            {
                break;
            }
        }
        if(forCount < 13 || strncmp(tempLotNo, ISNo, strlen(ISNo)) != 0)
        {
            printf("lot no error\n");
            //ZHEarseEEPROMData();
            close(fd);
            retryMax--;
            sleep(1);
            continue;
        }
        tempLotNo[14] = 0x00;

        arraySize = 10;
        //i2c_smbus_write_byte_data(fd, 0x00, 0x30);
        i2c_smbus_write_byte_data(fd, 0x00, 0x70);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempCountNo[forCount] = i2cChar;
            }else
            {
                tempCountNo[forCount] = 0x00;
                break;
            }
        }
        if(strncmp(tempCountNo, CountNo, strlen(CountNo)) != 0)   
        {
            printf("CountNo error\n");
            //ZHEarseEEPROMData();
            close(fd);
            retryMax--;
            sleep(1);
            continue;
        } 

        i2c_smbus_write_byte_data(fd, 0x00, 0x80);
        //i2c_smbus_write_byte_data(fd, 0x00, 0x40);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempGoodNo[forCount] = i2cChar;
            }else
            {
                tempGoodNo[forCount] = 0x00;
                break;
            }
        }
        if(strlen(GoodNo) > 0 && strncmp(tempGoodNo, GoodNo, strlen(GoodNo)) != 0)
        {
            printf("GoodNo error\n");
            //ZHEarseEEPROMData();
            close(fd);
            retryMax--;
            sleep(1);
            continue;
        }
        close(fd);
        return 0;
    }
    return 1;
}
