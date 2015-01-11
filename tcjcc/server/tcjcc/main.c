#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CONFIG_STRING_LENGTH 50
#define CONFIG_SETTING_NUMBER 5

#define PRINT_LOG

#define TOTAL_MACHINE_NUMBER 23

enum{
    COMPANYNO = 0,
    TARGETFOLDER,
    SOURCEFOLDER,
    ERRORMACHFOLDER,
    CONTROLNO
};

void FuncSettingMachineName(char **a)
{
    a[0]="T01-01";
    a[1]="T01-02";
    a[2]="T01-03";
    a[3]="T01-04";
    a[4]="T01-05";
    a[5]="T01-06";
    a[6]="T01-07";
    a[7]="T01-08";
    a[8]="T01-09";
    a[9]="T01-10";
    a[10]="T01-11";
    a[11]="T01-12";
    a[12]="T01-13";
    a[13]="T01-14";
    a[14]="T01-15";
    a[15]="T01-16";
    a[16]="T01-17";
    a[17]="T01-18";
    a[18]="T01-19";
    a[19]="T01-20";
    a[20]="D01";
    a[21]="WM01";
    a[22]="WM02";
}

int main()
{
    FILE *pFileSource, *pFileTarget, *pFileError;
    FILE *pFileConfig;
    int fileSize = 0;
    char *buffer, *charPosition;
    char *machineName[TOTAL_MACHINE_NUMBER];

    char configString[80];
    char companyNo[CONFIG_STRING_LENGTH];
    char targetFolderPath[CONFIG_STRING_LENGTH];
    char sourceFolderPath[CONFIG_STRING_LENGTH];
    char tempSourceFolderPath[CONFIG_STRING_LENGTH];
    char errorFolderPath[CONFIG_STRING_LENGTH];
    char controlNo[CONFIG_STRING_LENGTH];
    char currentTime[CONFIG_STRING_LENGTH];
    char currentYear[CONFIG_STRING_LENGTH];
    short configFlag[CONFIG_SETTING_NUMBER];
    time_t mytime;
    struct tm *localTime;
    unsigned char firstTimeFlag = 0;

    memset(configString, 0, sizeof(char)*80);
    memset(companyNo, 0, sizeof(char)*CONFIG_STRING_LENGTH);
    memset(targetFolderPath, 0, sizeof(char)*CONFIG_STRING_LENGTH);
    memset(sourceFolderPath, 0, sizeof(char)*CONFIG_STRING_LENGTH);
    memset(tempSourceFolderPath, 0, sizeof(char)*CONFIG_STRING_LENGTH);
    memset(errorFolderPath, 0, sizeof(char)*CONFIG_STRING_LENGTH);
    memset(controlNo, 0, sizeof(char)*CONFIG_STRING_LENGTH);
    memset(currentTime, 0, sizeof(char)*CONFIG_STRING_LENGTH);
    memset(currentYear, 0, sizeof(char)*CONFIG_STRING_LENGTH);

    memset(configFlag, 0, sizeof(short)*CONFIG_SETTING_NUMBER);

    FuncSettingMachineName(machineName);

    mytime = time(NULL);
    localTime = localtime(&mytime);
    strftime(currentTime, sizeof(currentTime), "%m%d%H%M", localTime);
    strftime(currentYear, sizeof(currentYear), "%Y", localTime);

    pFileConfig = fopen("d:\\tcjcc\\config.txt", "r");
    if(pFileConfig != NULL)
    {
        int arrayIndex = 0;
        fseek(pFileConfig, 0, SEEK_END);
        fileSize = ftell(pFileConfig);
        rewind(pFileConfig);
        buffer = (char *)malloc(sizeof(char)*fileSize);
        fread(buffer, 1, fileSize, pFileConfig);
        charPosition = buffer;
        fclose(pFileConfig);

        while(fileSize > 0)
        {
            if(*charPosition == '\n')
            {
                if(strncmp(configString, "#CompanyNo:", 11) == 0)
                {
                    configFlag[COMPANYNO] = 1;
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 11; ++forCount)
                    {
                        companyNo[forCount] = configString[forCount+11];
                    }
                    #ifdef PRINT_LOG
                    printf("%s|\n", companyNo);
                    #endif
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "#SourceFolder:", 14) == 0)
                {
                    configFlag[SOURCEFOLDER] = 1;
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 14; ++forCount)
                    {
                        sourceFolderPath[forCount] = configString[forCount+14];
                    }
                    #ifdef PRINT_LOG
                    printf("%s|\n", sourceFolderPath);
                    #endif
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "#TargetFolder:", 14) == 0)
                {
                    configFlag[TARGETFOLDER] = 1;
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 14; ++forCount)
                    {
                        targetFolderPath[forCount] = configString[forCount + 14];
                    }
                    #ifdef PRINT_LOG
                    printf("%s|\n", targetFolderPath);
                    #endif
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "#ErrorFolder:", 13) == 0)
                {
                    configFlag[ERRORMACHFOLDER] = 1;
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 13; ++forCount)
                    {
                        errorFolderPath[forCount] = configString[forCount + 13];
                    }
                    #ifdef PRINT_LOG
                    printf("%s|\n", errorFolderPath);
                    #endif
                    memset(configString, 0, sizeof(char)*80);
                }else if(strncmp(configString, "#CompanyControlNo:", 18) == 0)
                {
                    configFlag[CONTROLNO] = 1;
                    int forCount = 0;
                    for(forCount = 0; forCount < arrayIndex - 18; ++forCount)
                    {
                        controlNo[forCount] = configString[forCount + 18];
                    }
                    #ifdef PRINT_LOG
                    printf("%s|\n", controlNo);
                    #endif // PRINT_LOG
                    memset(configString, 0, sizeof(char)*80);
                }
                else;
                arrayIndex = 0;
            }else
            {
                configString[arrayIndex] = *charPosition;
                ++arrayIndex;
            }
            fileSize--;
            charPosition++;
        }

        if(buffer != NULL)
        {
            free(buffer);
            charPosition = NULL;
        }
    }
    else
    {
        printf("Can not open config file!\n");
        goto Exit;
    }
    //check read config file correct
    int forCount = 0;
    for(forCount = 0; forCount < CONFIG_SETTING_NUMBER; ++forCount)
    {
        if(configFlag[forCount] == 0)
        {
            printf("initial error %d!\n", forCount);
            goto Exit;
        }
    }

    strcat(targetFolderPath, currentTime);
    strcat(targetFolderPath, ".");
    strcat(targetFolderPath, controlNo);
    strcat(errorFolderPath, "errorMachine.txt");
    pFileTarget = fopen(targetFolderPath,"w");
    pFileError = fopen(errorFolderPath, "w");

    if(pFileTarget != NULL)
    {
        fprintf(pFileTarget ,"100%sWAR",companyNo);
        for(forCount = 0; forCount <TOTAL_MACHINE_NUMBER; forCount++)
        {
            memset(tempSourceFolderPath, 0, sizeof(char)*CONFIG_STRING_LENGTH);
            strcpy(tempSourceFolderPath, sourceFolderPath);
            strcat(tempSourceFolderPath, machineName[forCount]);
            printf("%s\n", tempSourceFolderPath);
            if(machineName[forCount] == "D01" || machineName[forCount] =="WM01" || machineName[forCount] == "WM02")
            {
                if(machineName[forCount] == "D01")
                {
                    pFileSource = fopen(tempSourceFolderPath,"r");
                    if(pFileSource != NULL)
                    {
                        char rawData[40];
                        int arrayIndex = 0;
                        memset(rawData, 0, sizeof(char)*40);
                        fseek(pFileSource, 0, SEEK_END);
                        fileSize = ftell(pFileSource);
                        rewind(pFileSource);
                        buffer = (char *)malloc(sizeof(char)*fileSize);
                        fread(buffer, 1, fileSize, pFileSource);
                        fclose(pFileSource);
                        charPosition = buffer;

                        while(fileSize > 0)
                        {
                            if(*charPosition == '\n')
                            {
                                if(rawData[arrayIndex-1] == '2' && rawData[arrayIndex-2] == '3')
                                {
                                    if(firstTimeFlag == 0)
                                    {
                                        fprintf(pFileError,"%s", machineName[forCount]);
                                        firstTimeFlag = 1;
                                    }
                                    else
                                    {
                                        fprintf(pFileError,"\n%s", machineName[forCount]);
                                    }
                                }
                                fprintf(pFileTarget,"\n%s", rawData);
                                arrayIndex = 0;
                                memset(rawData, 0, sizeof(char)*40);
                            }else
                            {
                                rawData[arrayIndex] = *charPosition;
                                ++arrayIndex;
                            }
                            fileSize--;
                            charPosition++;
                        }
                        if(buffer != NULL)
                        {
                            free(buffer);
                            charPosition = NULL;
                        }
                        unlink(tempSourceFolderPath);
                    }
                    else
                    {
                        printf("Open file fail %s\n", tempSourceFolderPath);
                        fprintf(pFileTarget,"\n248%-6s%d%s%-10d00", machineName[forCount], atoi(currentYear)-1911, currentTime, 0);
                        if(firstTimeFlag == 0)
                        {
                            fprintf(pFileError,"%s", machineName[forCount]);
                            firstTimeFlag = 1;
                        }
                        else
                        {
                            fprintf(pFileError,"\n%s", machineName[forCount]);
                        }
                    }
                }
                else
                {
                                        pFileSource = fopen(tempSourceFolderPath,"r");
                    if(pFileSource != NULL)
                    {
                        char rawData[40];
                        int arrayIndex = 0;
                        memset(rawData, 0, sizeof(char)*40);
                        fseek(pFileSource, 0, SEEK_END);
                        fileSize = ftell(pFileSource);
                        rewind(pFileSource);
                        buffer = (char *)malloc(sizeof(char)*fileSize);
                        fread(buffer, 1, fileSize, pFileSource);
                        fclose(pFileSource);
                        charPosition = buffer;

                        while(fileSize > 0)
                        {
                            if(*charPosition == '\n')
                            {
                                if(rawData[arrayIndex-1] == '2' && rawData[arrayIndex-2] == '3')
                                {
                                    if(firstTimeFlag == 0)
                                    {
                                        fprintf(pFileError,"%s", machineName[forCount]);
                                        firstTimeFlag = 1;
                                    }
                                    else
                                    {
                                        fprintf(pFileError,"\n%s", machineName[forCount]);
                                    }
                                }
                                //fprintf(pFileTarget,"\n%s", rawData);
                                arrayIndex = 0;
                                memset(rawData, 0, sizeof(char)*40);
                            }else
                            {
                                rawData[arrayIndex] = *charPosition;
                                ++arrayIndex;
                            }
                            fileSize--;
                            charPosition++;
                        }
                        if(buffer != NULL)
                        {
                            free(buffer);
                            charPosition = NULL;
                        }
                        unlink(tempSourceFolderPath);
                    }
                    else
                    {
                        printf("Open file fail %s\n", tempSourceFolderPath);
                        //fprintf(pFileTarget,"\n248%-6s%d%s%-10d00", machineName[forCount], atoi(currentYear)-1911, currentTime, 0);
                        if(firstTimeFlag == 0)
                        {
                            fprintf(pFileError,"%s", machineName[forCount]);
                            firstTimeFlag = 1;
                        }
                        else
                        {
                            fprintf(pFileError,"\n%s", machineName[forCount]);
                        }
                    }
                }
            }
            else
            {
                pFileSource = fopen(tempSourceFolderPath,"r");
                if(pFileSource != NULL)
                {
                    char rawData[40];
                    int arrayIndex = 0;

                    memset(rawData, 0, sizeof(char)*40);
                    fseek(pFileSource, 0, SEEK_END);
                    fileSize = ftell(pFileSource);
                    rewind(pFileSource);
                    buffer = (char *)malloc(sizeof(char)*fileSize);
                    fread(buffer, 1, fileSize, pFileSource);
                    fclose(pFileSource);
                    charPosition = buffer;

                    while(fileSize > 0)
                    {
                        if(*charPosition == '\n')
                        {
                            if(rawData[arrayIndex-1] == '2' && rawData[arrayIndex-2] == '3')
                            {
                                if(firstTimeFlag == 0)
                                {
                                    fprintf(pFileError,"%s", machineName[forCount]);
                                    firstTimeFlag = 1;
                                }
                                else
                                {
                                    fprintf(pFileError,"\n%s", machineName[forCount]);
                                }
                            }
                            fprintf(pFileTarget,"\n%s", rawData);
                            arrayIndex = 0;
                            memset(rawData, 0, sizeof(char)*40);
                        }else
                        {
                            rawData[arrayIndex] = *charPosition;
                            ++arrayIndex;
                        }
                        fileSize--;
                        charPosition++;
                    }
                    if(buffer != NULL)
                    {
                        free(buffer);
                        charPosition = NULL;
                    }
                    unlink(tempSourceFolderPath);
                }
                else
                {
                    printf("Open file fail %s\n", tempSourceFolderPath);
                    fprintf(pFileTarget,"\n246%-6s%d%s%-10d00", machineName[forCount], atoi(currentYear)-1911, currentTime, 0);
                    fprintf(pFileTarget,"\n247%-6s%d%s%-10d00", machineName[forCount], atoi(currentYear)-1911, currentTime, 0);
                    fprintf(pFileTarget,"\n259%-6s%d%s%-10d00", machineName[forCount], atoi(currentYear)-1911, currentTime, 0);
                    if(firstTimeFlag == 0)
                    {
                        fprintf(pFileError,"%s", machineName[forCount]);
                        firstTimeFlag = 1;
                    }
                    else
                    {
                        fprintf(pFileError,"\n%s", machineName[forCount]);
                    }
                }
            }
        }
        fclose(pFileTarget);
    }else
    {
        printf("Open file fail\n");
    }
    if(pFileError != NULL)
    {
        fclose(pFileError);
    }
Exit:
    //while(1);

    return 0;
}
