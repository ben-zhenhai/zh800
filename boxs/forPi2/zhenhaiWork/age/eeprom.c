#include "eeprom.h"

void CleanListFunction()
{
    InputNode *nodePtr = ZHList;

    while(nodePtr->link != NULL)
    {
        ZHList = ZHList->link;
        free(nodePtr);
        nodePtr = ZHList; 
    }
}

int EarseEEPROMData()
{
    int fd, r, returnValue;
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
    printf("[%s|%d]", __func__, __LINE__);
    i2c_smbus_write_byte_data(fd, 0x00, 0x00);
    eraseArray[0] = 0x00; 
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
    eraseArray[0] = 0x20; 
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    eraseArray[0] = 0x40;
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    eraseArray[0] = 0x60;
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
        
    eraseArray[0] = 0x80;
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    eraseArray[0] = 0xa0;
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    eraseArray[0] = 0xc0;
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x00, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    
    eraseArray[0] = 0x00; 
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x01, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    eraseArray[0] = 0x20;
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x01, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    eraseArray[0] = 0x40; 
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x01, 0x20, &eraseArray[0]);
    printf("%d ", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    eraseArray[0] = 0x60; 
    returnValue = i2c_smbus_write_i2c_block_data(fd, 0x01, 0x20, &eraseArray[0]);
    printf("%d\n", returnValue);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        

    close(fd);
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
    int queueSize = 0;
    int highAddressIncreaceCount = 0;

    char lowAddress, highAddress; 

    lowAddress = highAddress = 0x00;

    InputNode *nodePtr = ZHList;

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

    i2c_smbus_write_byte_data(fd, 0x00, 0x00);
    arraySize = i2c_smbus_read_byte(fd);
    if(arraySize == 0xff)
    {
        printf("queue size error\n");
        close(fd);
        return 1;
    }
    queueSize = (int) arraySize;
    printf("queue size %d\n", queueSize);

    memset(tempLotNo, 0, sizeof(char)*30);
    memset(tempManagerCard, 0, sizeof(char)*30);
    memset(tempCountNo, 0, sizeof(char)*30);
    memset(tempGoodNo, 0, sizeof(char)*30);

    arraySize = 24;
    i2c_smbus_write_byte_data(fd, highAddress, lowAddress + 0x20);
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
    if(forCount < 23)
    {
        printf("part no error\n");
        close(fd);
        return 1;
    }
    tempManagerCard[24] = 0x00;
    
    arraySize = 14;
    i2c_smbus_write_byte_data(fd, highAddress, lowAddress + 0x40);
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
    if(forCount < 13)
    {
        printf("lot no error\n");
        close(fd);
        return 1;
    }
    tempLotNo[14] = 0x00;

    arraySize = 10;
    i2c_smbus_write_byte_data(fd, highAddress, lowAddress + 0x50);
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
    lowAddress = lowAddress + 0x60;
    i2c_smbus_write_byte_data(fd, highAddress, lowAddress);
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

    //copy to correct array
    memset(nodePtr->ISNo, 0, sizeof(char)*INPUTLENGTH);
    memset(nodePtr->ManagerCard, 0, sizeof(char)*INPUTLENGTH);
    memset(nodePtr->CountNo, 0, sizeof(char)*INPUTLENGTH);
    memset(nodePtr->GoodNo, 0, sizeof(char)*INPUTLENGTH);

    printf("[%s|%d]%s %s %s %s\n", __func__, __LINE__, tempLotNo, tempManagerCard, tempCountNo, tempGoodNo);

    memcpy(nodePtr->ISNo, tempLotNo, sizeof(char)*30);
    memcpy(nodePtr->ManagerCard, tempManagerCard, sizeof(char)*30);
    memcpy(nodePtr->CountNo, tempCountNo, sizeof(char)*30);
    memcpy(nodePtr->GoodNo, tempGoodNo, sizeof(char)*30);

    --queueSize;
    ++OrderInBox;

    while(queueSize > 0)
    {
        if(highAddressIncreaceCount == 2)
        {
            highAddressIncreaceCount = 0;
            highAddress = highAddress + 0x01;
        }

        ZHNode = (InputNode *)malloc(sizeof(InputNode));
        if(ZHNode == NULL)
        {
            printf("alloc new node fail\n");
            close(fd);        
            CleanListFunction(); 
            OrderInBox = 0;
            return 1;
        }
        memset(ZHNode->ISNo, 0, sizeof(char)*INPUTLENGTH);
        memset(ZHNode->ManagerCard, 0, sizeof(char)*INPUTLENGTH);
        memset(ZHNode->UserNo, 0, sizeof(char)*INPUTLENGTH);
        memset(ZHNode->CountNo, 0, sizeof(char)*INPUTLENGTH);
        memset(ZHNode->UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
        memset(ZHNode->GoodNo, 0, sizeof(char)*INPUTLENGTH);
        ZHNode->link = NULL;

        memset(tempLotNo, 0, sizeof(char)*30);
        memset(tempManagerCard, 0, sizeof(char)*30);
        memset(tempCountNo, 0, sizeof(char)*30);
        memset(tempGoodNo, 0, sizeof(char)*30);

        arraySize = 24;
        i2c_smbus_write_byte_data(fd, highAddress, lowAddress + 0x20);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempManagerCard[forCount] = i2cChar;
                printf("%c ", i2cChar);
            }
            else
            {
                break;   
            }
        }
        if(forCount < 23)
        {
            printf("part no error\n");
            close(fd);
            CleanListFunction(); 
            OrderInBox = 0;
            if(ZHNode != NULL) free(ZHNode);
            return 1;
        }
        tempManagerCard[24] = 0x00;
        printf("\n");
    
        arraySize = 14;
        i2c_smbus_write_byte_data(fd, highAddress, lowAddress + 0x40);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempLotNo[forCount] = i2cChar;
                printf("%c ", i2cChar);
            }else
            {
                break;
            }
        }
        if(forCount < 13)
        {
            printf("lot no error\n");
            close(fd);
            CleanListFunction(); 
            OrderInBox = 0;
            if(ZHNode != NULL) free(ZHNode);
            return 1;
        }
        tempLotNo[14] = 0x00;
        printf("\n");

        arraySize = 10;
        i2c_smbus_write_byte_data(fd, highAddress, lowAddress + 0x50);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempCountNo[forCount] = i2cChar;
                printf("%c ", i2cChar);
            }else
            {
                tempCountNo[forCount] = 0x00;
                break;
            }
        }
        printf("\n");
        lowAddress = lowAddress + 0x60;
        i2c_smbus_write_byte_data(fd, highAddress, lowAddress);
        for(forCount = 0; forCount < arraySize; forCount++)
        {
            char i2cChar = i2c_smbus_read_byte(fd);
            if(i2cChar != 0x00)
            {
                tempGoodNo[forCount] = i2cChar;
                printf("%c", i2cChar);
            }else
            {
                tempGoodNo[forCount] = 0x00;
                break;
            }
        }
        printf("\n");

        //copy to correct array
        memcpy(ZHNode->ISNo, tempLotNo, sizeof(char)*30);
        memcpy(ZHNode->ManagerCard, tempManagerCard, sizeof(char)*30);
        memcpy(ZHNode->CountNo, tempCountNo, sizeof(char)*30);
        memcpy(ZHNode->GoodNo, tempGoodNo, sizeof(char)*30);

        nodePtr = ZHList;
        while(nodePtr->link != NULL)
        {
            nodePtr = nodePtr->link;
        }
        nodePtr->link = ZHNode;
        --queueSize;
        ++OrderInBox;
        ++highAddressIncreaceCount;
    }
    close(fd);
    return 0;
}

int WriteEEPROMData()
{
    int fd, r, forCount;
    
    char tempLotNo[30];
    char tempManagerCard[30];
    char tempCountNo[30];
    char tempGoodNo[30];
    char arraySize;   

    int queueSize = 0;
    int highAddressIncreaceCount = 0;

    char queueArray[2];
    char eepromISNo[INPUTLENGTH];
    char eepromManagerCard[INPUTLENGTH];
    char eepromCountNo[INPUTLENGTH];
    char eepromGoodNo[INPUTLENGTH];
    char *ptr;
    
    char highAddress, lowAddress;
    highAddress = lowAddress = 0x00;    

    InputNode *nodePtr = ZHList;

    if(nodePtr != NULL)
    {
        ++queueSize;
        while(nodePtr->link != NULL)
        {
            nodePtr = nodePtr->link;
            ++queueSize;
        } 
        printf("queue size is: %d\n", queueSize);
    }
    
    memset(queueArray, 0, sizeof(char)*2);

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
    queueArray[1] = (char)queueSize;

    i2c_smbus_write_i2c_block_data(fd, 0x00, 0x02, &queueArray[0]);
    nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
    close(fd);

    nodePtr = ZHList;
    while(nodePtr != NULL)
    {
        int retryMax = 3;
        if(highAddressIncreaceCount == 2)
        {
            highAddressIncreaceCount = 0;
            highAddress = highAddress + 0x01;
            lowAddress = 0x00;
        }

        memset(eepromISNo, 0, sizeof(char)*INPUTLENGTH);
        memset(eepromManagerCard, 0, sizeof(char)*INPUTLENGTH);
        memset(eepromCountNo, 0, sizeof(char)*INPUTLENGTH);
        memset(eepromGoodNo, 0, sizeof(char)*INPUTLENGTH);

        //0x20 0x40 0x50 0x60 0x80 0xa0 0xb0 0xc0 
        ptr = &eepromManagerCard[1];
        lowAddress = lowAddress + 0x20;
        eepromManagerCard[0] = lowAddress;
        memcpy(ptr, nodePtr->ManagerCard, sizeof(char)*strlen(nodePtr->ManagerCard));
        printf("1: %s %s %d\n", ptr, nodePtr->ManagerCard, strlen(nodePtr->ManagerCard)+1);

        ptr = &eepromISNo[1];
        lowAddress = lowAddress + 0x20;
        eepromISNo[0] = lowAddress;
        memcpy(ptr, nodePtr->ISNo, sizeof(char)*strlen(nodePtr->ISNo));
        printf("2: %s %s %d\n", ptr,nodePtr->ISNo, strlen(nodePtr->ISNo)+1);

        ptr = &eepromCountNo[1];
        lowAddress = lowAddress + 0x10;
        eepromCountNo[0] = lowAddress;
        memcpy(ptr, nodePtr->CountNo, sizeof(char)*strlen(nodePtr->CountNo));
        printf("3: %s %s %d\n", ptr, nodePtr->CountNo, strlen(nodePtr->CountNo)+1);

        ptr = &eepromGoodNo[1];
        lowAddress = lowAddress + 0x10;
        eepromGoodNo[0] = lowAddress;
        memcpy(ptr, nodePtr->GoodNo, sizeof(char)*strlen(nodePtr->GoodNo));
        printf("4: %s %s %d\n", ptr, nodePtr->GoodNo, strlen(nodePtr->GoodNo)+1);

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

            i2c_smbus_write_i2c_block_data(fd, highAddress, strlen(nodePtr->ManagerCard)+1, &eepromManagerCard[0]);
            nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);        
            i2c_smbus_write_i2c_block_data(fd, highAddress, strlen(nodePtr->ISNo)+1, &eepromISNo[0]);
            nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
            i2c_smbus_write_i2c_block_data(fd, highAddress, strlen(nodePtr->CountNo)+1, &eepromCountNo[0]);
            nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);
            i2c_smbus_write_i2c_block_data(fd, highAddress, strlen(nodePtr->GoodNo)+1, &eepromGoodNo[0]);
            nanosleep((const struct timespec[]){{0, 5000000L}}, NULL);

            close(fd);
#ifdef DEBUG
            //check data by log
            for(forCount = 0; forCount < strlen(nodePtr->ISNo) + 1; forCount++)
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

            for(forCount = 0; forCount < strlen(nodePtr->ManagerCard) + 1; forCount++)
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

            for(forCount = 0; forCount < strlen(nodePtr->CountNo) + 1; forCount++)
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

            for(forCount = 0; forCount < strlen(nodePtr->GoodNo) + 1; forCount++)
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
            i2c_smbus_write_byte_data(fd, highAddress, lowAddress - 0x40);
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

            if(forCount < 23 || strncmp(tempManagerCard, nodePtr->ManagerCard, strlen(nodePtr->ManagerCard)) != 0)
            {
                printf("part no error\n");
                close(fd);
                retryMax--;
                sleep(1);
                continue;
            }
            tempManagerCard[24] = 0x00;
    
            arraySize = 14;
            i2c_smbus_write_byte_data(fd, highAddress, lowAddress - 0x20);
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
            if(forCount < 13 || strncmp(tempLotNo, nodePtr->ISNo, strlen(nodePtr->ISNo)) != 0)
            {
                printf("lot no error\n");
                close(fd);
                retryMax--;
                sleep(1);
                continue;
            }
            tempLotNo[14] = 0x00;

            arraySize = 10;
            i2c_smbus_write_byte_data(fd, highAddress, lowAddress - 0x10);
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
            if(strncmp(tempCountNo, nodePtr->CountNo, strlen(nodePtr->CountNo)) != 0)   
            {
                printf("CountNo error\n");
                close(fd);
                retryMax--;
                sleep(1);
                continue;
            } 

            i2c_smbus_write_byte_data(fd, highAddress, lowAddress);
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
            if(strncmp(tempGoodNo, nodePtr->GoodNo, strlen(nodePtr->GoodNo)) != 0)
            {
                printf("GoodNo error\n");
                close(fd);
                retryMax--;
                sleep(1);
                continue;
            }
            close(fd);
            //should be success!!
            break;
        }
        if(retryMax > 0)
        {
            nodePtr = nodePtr->link;
            printf("next ~!!\n");
        }else
        {
            printf("write data to EEPROM fail!!\n");
            return 1;
        }
    }
    return 0;
}


