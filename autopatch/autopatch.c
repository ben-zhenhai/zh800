#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sys/wait.h>
#include <time.h>

#include "crc.h"
//使用於工廠數據收集上
#define Version "taicon_ver"
#define FrontNameLength 10
//使用於污水偵測數據收集上
//#define Version "tcjcc_ver"
//#define FrontNameLength 9
#define VersionNumberLength 8
#define FileNameSize 256
#define ExecuteFilePathLength 256
#define MaxRetryCount 5
#define updatelist "updatelist"
#define TempFilepath "/home/pi/zhlog/"
#define LogFilePath "/home/pi/zhlog/patchlog"

char FtpServer[FileNameSize];
char AccPw[FileNameSize];
char Machine[FileNameSize];
char NewestFileName[FileNameSize];

struct FtpFile
{
    char *filename;
    FILE *stream;
};

struct MemoryStruct {
    char *memory;
    size_t size;    
};

//file name and crc value 是否跟最新版本一致, 一致的話就不需要拉新檔案
int CheckLocalFileCRCValueFunction(char *dlFilePath, int crcValue)
{
    FILE *filePtr;
    int fileSize = 0;   
    unsigned char *buffer;

    printf("[%s]%s %d\n", __func__, dlFilePath, crcValue);

    filePtr = fopen(dlFilePath, "r");
    if(filePtr != NULL)
    {
        fseek(filePtr, 0, SEEK_END);
        fileSize = ftell(filePtr);
        rewind(filePtr);
        buffer = (unsigned char *)malloc(sizeof(unsigned char)*fileSize);
        fread(buffer, 1, fileSize, filePtr);
        fclose(filePtr);
        unsigned short crcResult = crcSlow(buffer, fileSize);
        printf("check crc Result: %d\n",crcResult);

        if(buffer != NULL) free(buffer);

        if(crcResult != crcValue) return 1;
        else return 0;
    }
    return 1;
}

void GetNewFileName(char *a)
{
    DIR *dp;
    struct dirent *ep;
    unsigned long currentVersion = 0;
    char getversion[VersionNumberLength+1];
    memset(getversion, 0, sizeof(char)*(VersionNumberLength+1));

    dp = opendir(".");

    if(dp != NULL)
    {
        while((ep = readdir(dp)))
        {
            char *target = ep->d_name;
            
            unsigned long getVersion = 0;
            target = target + FrontNameLength;
            if(strncmp(ep->d_name, Version, FrontNameLength) == 0)
            {
                int forcount = 0;
                for(forcount = 0; forcount <=VersionNumberLength; forcount++)
                {
                    if(forcount != VersionNumberLength)
                    {
                        getversion[forcount] = *(target+forcount);
                    }
                    else
                    {
                        getversion[forcount] = '\0';
                    }                
                }
                getVersion = atoi(getversion);
                printf("version: %ld, file name:%s\n", getVersion, ep->d_name);
                if(currentVersion < getVersion)
                {
                    currentVersion = getVersion;
                    strcpy(a, ep->d_name);
                }
            }
        }
    }
}

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpFile *out=(struct FtpFile *)stream;
    if(out && !out->stream) {
        out->stream=fopen(out->filename, "wb");
        if(!out->stream)
        return -1; 
    }
    return fwrite(buffer, size, nmemb, out->stream);
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

//確認local 端的 版本文件與server相比是否是最新版本, 是的話 retrurn 0, 否則 return 1執行update
//比較方式則是透過filename 上面的日期數字相比... yyyymmddxx,分別為年 月 日 編號,若是一天內有兩次patch 則透過編號+1區別
//ex: 2016070501 跟 2016070502 
int CheckNewestVersionFunction()
{
    CURL *curl_handle;
    CURLcode result;
    struct MemoryStruct chunk;
    char dlFileName[FileNameSize];
    char dlAddress[FileNameSize];
    char currentFile[FileNameSize];
    int needUpdate = 0;
    FILE *filePtr;
    time_t now;
    struct tm  ts;
    char buf[80];

    memset(dlAddress, 0, sizeof(char)*FileNameSize);
    memset(dlFileName, 0, sizeof(char)*FileNameSize);
    memset(currentFile, 0, sizeof(char)*FileNameSize);
    memset(buf, 0, sizeof(char)*80);

    chunk.memory = malloc(1);  
    chunk.size = 0;   

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    strcpy(dlAddress, FtpServer);
    strcat(dlAddress, Machine);
    printf("%s\n", dlAddress);
    curl_easy_setopt(curl_handle, CURLOPT_URL, dlAddress);
    curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    result = curl_easy_perform(curl_handle);
    // Get current time
    time(&now);
    // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);
    if(result != CURLE_OK) 
    {
        fprintf(stderr, "%s curl_easy_perform() failed: %s\n", __func__, curl_easy_strerror(result));

        curl_easy_cleanup(curl_handle);
        if(chunk.memory)
        {
            free(chunk.memory);
        }
        curl_global_cleanup();

        filePtr = fopen(LogFilePath, "a");
        if(filePtr != NULL)
        {
            fprintf(filePtr,"%s access server fail\n", buf);
            fclose(filePtr);    
        }
        return 0;
    }else 
    {
        printf("%lu bytes retrieved\n", (long)chunk.size);
        unsigned stringLength = strlen(chunk.memory)+1;
        int forCount; 
        char *a;
        a = chunk.memory;
        unsigned long currentVersion = 0;
 
        GetNewFileName(NewestFileName);
        strcpy(currentFile, NewestFileName);
        char *target_1 = NewestFileName;
        char currentVersionArray[VersionNumberLength+1];
    
        target_1 = target_1 + FrontNameLength;
        for(forCount = 0; forCount < VersionNumberLength; forCount++)
        {
            if(forCount != VersionNumberLength)
            {
                currentVersionArray[forCount] = *(target_1 + forCount);
            }
            else
            {
                currentVersionArray[forCount] = '\0';
            }
        }
        currentVersion = atol(currentVersionArray);

        printf("%ld The new File is: %s\n", currentVersion, NewestFileName);

        for(forCount = 0; forCount< stringLength; ++forCount)
        {
            if(*a == '\n')
            {
                if(strncmp(dlFileName, Version, FrontNameLength) == 0)
                {
                    char *target = dlFileName;
                    char newversion[VersionNumberLength+1];
                    int forCount_2 = 0;
                
                    memset(newversion, 0, sizeof(char)*(VersionNumberLength+1));
                    target = target + FrontNameLength;
                    for(forCount_2 = 0; forCount_2 < VersionNumberLength; forCount_2++)
                    {
                        if(forCount_2 != VersionNumberLength)
                        {
                            newversion[forCount_2] = *(target+forCount_2);
                        }
                        else
                        {
                            //newversion[forCount_2] = '\0';
                            ;
                        }
                    }
                    unsigned long newVersion = atol(newversion);
                    if (newVersion > currentVersion)
                    {
                        memset(NewestFileName, 0, sizeof(char)*FileNameSize);
                        strcpy(NewestFileName, dlFileName);
                        needUpdate = 1;
                    }
                    memset(dlFileName, 0, sizeof(char)*FileNameSize);
                }
            }else if(*a == ' ')
            {
                memset(dlFileName, 0, sizeof(char)*FileNameSize);
            }
            else
            {
                strncat(dlFileName, a, sizeof(char)*1);
            }
            printf("%c", *a);
            a++;
        }
    }
    curl_easy_cleanup(curl_handle);
    if(chunk.memory)
    {
        free(chunk.memory);
    }
    curl_global_cleanup();
    //delete file
    unlink(currentFile);
    return needUpdate; 
}

int main(int argc,char* argv[])
{
    CURL *curl_handle;
    CURLcode res2, res3 ;
    char configString[ExecuteFilePathLength];
    char updatelistFilePath[ExecuteFilePathLength];
    char tempNewestFile[ExecuteFilePathLength];
    FILE *pfile;
    char *buffer, *charPosition;
    int fileSize = 0;
    int arrayIndex = 0;
    time_t     now;
    struct tm  ts;
    char       buf[80];

    memset(NewestFileName, 0, sizeof(char)*FileNameSize);
    memset(Machine, 0, sizeof(char)*FileNameSize);
    memset(configString, 0, sizeof(char)*ExecuteFilePathLength);
    memset(tempNewestFile, 0, sizeof(char)*ExecuteFilePathLength);
    memset(FtpServer, 0, sizeof(char)*ExecuteFilePathLength);    
    memset(AccPw, 0, sizeof(char)*ExecuteFilePathLength);
    memset(updatelistFilePath, 0, sizeof(char)*ExecuteFilePathLength);
    memset(buf, 0, sizeof(char)*80);

    pfile = fopen("config", "r");
    fseek(pfile, 0, SEEK_END);
    fileSize = ftell(pfile);
    rewind(pfile);   
    buffer = (char *)malloc(sizeof(char)*fileSize);
    fread(buffer, 1, fileSize, pfile);
    fclose(pfile);
    charPosition = buffer;

    while(fileSize > 0)
    {
        if(*charPosition == '\n')
        {
            if(strncmp(configString, "Server:", 7) == 0)
            {
                int forCount = 0;
                for(forCount = 0; forCount < arrayIndex-7; ++forCount)
                {
                    FtpServer[forCount] =  configString[forCount+7];
                }
                printf("%s|\n", FtpServer);
                memset(configString, 0, sizeof(char)*ExecuteFilePathLength);
            }
            else if(strncmp(configString, "AccPw:", 6) == 0)
            {
                int forCount = 0;
                for(forCount = 0; forCount < arrayIndex-6; ++forCount)
                {
                    AccPw[forCount] = configString[forCount+6];
                }
                memset(configString, 0, sizeof(char)*ExecuteFilePathLength);
            }
            else if(strncmp(configString, "MachineType:", 12) == 0)
            {
                int forCount = 0;
                for(forCount = 0; forCount < arrayIndex-12; ++forCount)
                {
                    Machine[forCount] = configString[forCount+12];    
                }
                printf("%s|\n", Machine);
                memset(configString, 0, sizeof(char)*ExecuteFilePathLength);
            }
            else;
            arrayIndex = 0;
        }else
        {
            configString[arrayIndex] = *charPosition;
            ++arrayIndex;
        }
        //printf("%c", *charPosition);
        ++charPosition;
        --fileSize;
    }
    if(buffer != NULL)
    {
        free(buffer);
        charPosition = NULL;
    }
    sleep(5);
    //ready to update
    if(CheckNewestVersionFunction())
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle = curl_easy_init();
    
        struct FtpFile ftpfile, ftpfile2;
        int updateSuccess = 0;
        char downloadFileName[FileNameSize]; //in updatelist
        char crcCheckValue[FileNameSize];    //in updatelist
        int arrayPosition = 0;
        short fileNameEndFlag = 0;

        int fileSize2 = 0;
        char *bufferForupdatelist, *charPositionForupdatelist;
        char *bufferTempNewestFile, *charPositionTempNewestFile;
        char targetName[FileNameSize];
        char targetPath[ExecuteFilePathLength];
        char remoteFolder[FileNameSize];

        //update 最新版本的files crc
        if(curl_handle)
        {
            char RemoteURL[ExecuteFilePathLength];

            memset(RemoteURL, 0, sizeof(char)*ExecuteFilePathLength);
            strcpy(RemoteURL, FtpServer);
            strcat(RemoteURL, Machine);
            strcat(RemoteURL, NewestFileName);
            curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);
            curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL);
            strcpy(tempNewestFile,TempFilepath);
            strcat(tempNewestFile, NewestFileName);
            ftpfile.stream = NULL;
            ftpfile.filename = tempNewestFile;
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_fwrite);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &ftpfile);

            res2 = curl_easy_perform(curl_handle);
            curl_easy_cleanup(curl_handle);
            if(CURLE_OK != res2) 
            {
                fprintf(stderr, "curl told us %d\n", res2);
                if(ftpfile.stream)
                    fclose(ftpfile.stream); /* close the local file */

                curl_global_cleanup();
                printf("no need to update\n");

                time(&now);
                ts = *localtime(&now);
                memset(buf, 0, sizeof(char)*80);
                strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

                pfile = fopen(LogFilePath, "a");
                if(pfile != NULL)
                {
                    fprintf(pfile,"%s download newest file list fail\n", buf);
                    fclose(pfile);
                }
                return 0;
            }
        }
        if(ftpfile.stream)
            fclose(ftpfile.stream); /* close the local file */

        curl_global_cleanup();

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle = curl_easy_init();
    
        //update updatelist 
        //files need to update and remote file path and local file path
        if(curl_handle)
        {
            char RemoteURL[ExecuteFilePathLength];

            memset(RemoteURL, 0, sizeof(char)*ExecuteFilePathLength);
            strcpy(RemoteURL, FtpServer);
            strcat(RemoteURL, Machine);
            strcat(RemoteURL, updatelist);
            curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL);
            curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);
            strcpy(updatelistFilePath,TempFilepath);
            strcat(updatelistFilePath, updatelist);
            ftpfile.stream = NULL;
            ftpfile.filename = updatelistFilePath;
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_fwrite);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &ftpfile);

            res2 = curl_easy_perform(curl_handle);
            curl_easy_cleanup(curl_handle);
            if(CURLE_OK != res2) {
                fprintf(stderr, "curl told us %d\n", res2);
                if(ftpfile.stream)
                    fclose(ftpfile.stream); /* close the local file */

                curl_global_cleanup();
                curl_global_cleanup();

                time(&now);
                ts = *localtime(&now);
                memset(buf, 0, sizeof(char)*80);
                strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

                pfile = fopen(LogFilePath, "a");
                if(pfile != NULL)
                {
                    fprintf(pfile,"%s download updatelist fail\n", buf);
                    fclose(pfile);
                }
                printf("no need to update\n");
                return 0;
            }
        }

        if(ftpfile.stream)
            fclose(ftpfile.stream); /* close the local file */

        curl_global_cleanup();

        pfile = fopen(tempNewestFile, "r");
        fseek(pfile, 0, SEEK_END);
        fileSize = ftell(pfile);
        rewind(pfile);
        bufferTempNewestFile = (char *)malloc(sizeof(char)*fileSize);
        fread(bufferTempNewestFile, 1, fileSize, pfile);
        fclose(pfile);
        charPositionTempNewestFile = bufferTempNewestFile;
        memset(downloadFileName, 0, sizeof(char)*FileNameSize);
        memset(crcCheckValue, 0, sizeof(char)*FileNameSize);

        printf("ready to compare %d\n", fileSize);

        memset(targetName, 0, sizeof(char)*FileNameSize);
        memset(remoteFolder, 0, sizeof(char)*FileNameSize);
        memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);

        pfile = fopen(updatelistFilePath, "r");
        fseek(pfile, 0, SEEK_END);
        fileSize2 = ftell(pfile);
        rewind(pfile);
        bufferForupdatelist = (char *)malloc(sizeof(char)*fileSize2);
        fread(bufferForupdatelist, 1, fileSize2, pfile);
        fclose(pfile);

        //使用兩個loop 去做比對... 先開taicon_xxxxxxx parse 完一行後在parse 全部的 updatelist 進行比多
        while(fileSize > 0)
        {   
            if(*charPositionTempNewestFile != '\n' && *charPositionTempNewestFile != 0x0d) 
            {
                if(*charPositionTempNewestFile == ' ')
                {
                    fileNameEndFlag = 1;
                    arrayPosition  = 0;
                    charPositionTempNewestFile++;
                    fileSize--;
                }
                if(fileNameEndFlag == 0)
                {
                    downloadFileName[arrayPosition] = *charPositionTempNewestFile;
                }else
                {
                    crcCheckValue[arrayPosition] = *charPositionTempNewestFile;
                }
                ++arrayPosition;
            }else if(*charPositionTempNewestFile == '\n')
            {
                short fileNameEndFlag2 = 0;
                int arrayPosition2 = 0;
                int fileSize3 = fileSize2;
                memset(targetName, 0, sizeof(char)*FileNameSize);
                memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);
                memset(remoteFolder, 0, sizeof(char)*FileNameSize);
                charPositionForupdatelist = bufferForupdatelist;
                
                while(fileSize3 > 0)
                {
                    if(*charPositionForupdatelist != '\n' && *charPositionForupdatelist != 0x0d) 
                    {
                        if(*charPositionForupdatelist == ' ')
                        {
                            fileNameEndFlag2 = fileNameEndFlag2 + 1;
                            arrayPosition2 = 0;
                            charPositionForupdatelist++;
                            fileSize3--;
                        }
                        if(fileNameEndFlag2 == 2)
                        {
                            remoteFolder[arrayPosition2] = *charPositionForupdatelist;
                        }
                        else if(fileNameEndFlag2 == 1)
                        {
                            targetPath[arrayPosition2] = *charPositionForupdatelist;
                        }else
                        {
                            targetName[arrayPosition2] = *charPositionForupdatelist;
                        }
                        ++arrayPosition2;
                    }
                    else if( *charPositionForupdatelist == '\n')
                    {
                        printf("%s|%s|%s|%s|\n", targetName, targetPath, remoteFolder, downloadFileName);
                        // implement function for check local file first
                         
                        if(strcmp(downloadFileName, targetName) == 0)
                        {
                            int retryCount = MaxRetryCount;
                            char actualFileName[ExecuteFilePathLength];
                            memset(actualFileName, 0, sizeof(char)*ExecuteFilePathLength);
                            strcpy(actualFileName, targetPath);
                            strcat(actualFileName, targetName);
 
                            if(CheckLocalFileCRCValueFunction(&actualFileName[0], atoi(crcCheckValue)))
                            {
                                while(retryCount > 0)
                                {                  
                                    curl_global_init(CURL_GLOBAL_DEFAULT);
                                    curl_handle = curl_easy_init();
                                    char road[ExecuteFilePathLength];
 
                                    memset(road, 0, sizeof(char)*ExecuteFilePathLength);
                                    printf("%s|%s|\n", targetName, targetPath);
                                    if(curl_handle)
                                    {
                                        char RemoteURL2[ExecuteFilePathLength];
                                        memset(RemoteURL2, 0, sizeof(char)*ExecuteFilePathLength);
                                        strcpy(RemoteURL2, FtpServer);
                                        strcat(RemoteURL2, remoteFolder);
                                        strcat(RemoteURL2, targetName);
                                        curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);
       
                                        curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL2);
                                        strcpy(road, targetPath);
                                        strcat(road, targetName);
                                        strcat(road, ".temp");
                                        ftpfile2.filename = road;
                                        ftpfile2.stream = NULL;
                                        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_fwrite);
                                        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &ftpfile2);

                                        res3 = curl_easy_perform(curl_handle);
                                        curl_easy_cleanup(curl_handle);
                                        if(CURLE_OK != res3) 
                                        {
                                            fprintf(stderr, "curl told us %d\n", res3);
                                        }
                                    }
                                    if(ftpfile2.stream)
                                        fclose(ftpfile2.stream); /* close the local file */
                                    curl_global_cleanup();

                                    if(res3 == CURLE_OK)
                                    {
                                        //成功下載新版的file 後進行 crc 比對, 確認upload 成功
                                        //check crc
                                        pfile = fopen(road, "r");
                                        if(pfile != NULL)
                                        {
                                            unsigned char *crcBuffer;
                                            fseek(pfile, 0, SEEK_END);
                                            int fileSize = ftell(pfile);
                                            rewind(pfile);
                                            crcBuffer = (unsigned char *)malloc(sizeof(unsigned char)*fileSize);
                                            fread(crcBuffer, 1, fileSize, pfile);
                                            fclose(pfile);
                                    
                                            unsigned short crcResult = crcSlow(crcBuffer, fileSize);
                                            printf("crc result:%d %d\n", crcResult, atoi(crcCheckValue));
                                            if(crcBuffer != NULL)
                                            {
                                                printf("ready to free crcBuffer\n");
                                                free(crcBuffer);
                                            }
                                    
                                            if(crcResult == atol(crcCheckValue))
                                            {
                                                updateSuccess = updateSuccess + 1;            
                                                int i = 0, j = 0;
                                                char c;
                                                char *checkFilename;
                                                checkFilename = (char *)malloc(sizeof(char)*strlen(road));
                                                strcpy(checkFilename, road);
                                                int compareSuccess = 0;
 
                                                for(i = 0, j = strlen(road); i < j; i++, j--)
                                                {
                                                    c = checkFilename[i];
                                                    checkFilename[i] = checkFilename[j];
                                                    checkFilename[j] = c;
                                                }
                                                if(strncmp(checkFilename, "pmet.zg.rat", 11) == 0)
                                                {
                                                    compareSuccess = 1;
                                                }
                                                if(checkFilename != NULL)
                                                {
                                                    printf("%s ready to free checkFilename\n", checkFilename);
                                                    free(checkFilename);
                                                }
                                                if(compareSuccess)
                                                {
                                                    pid_t proc = fork();
                                                    if(proc < 0)
                                                    {
                                                        printf("fork fail\n");
                                                        return -1;
                                                    }else if(proc == 0)
                                                    {
                                                        unlink(actualFileName);

                                                        execlp("mv", "mv", road, actualFileName, (char *) 0);
                                                        exit(0);
                                                    }else
                                                    {
                                                        int  result = -1;
                                                        wait(&result);
                                                    }
                                                    if(strcmp(actualFileName, "/home/pi/new_patch/mongodb.tar.gz") == 0)
                                                    {
                                                        printf("ready to remove mongo folder\n");
                                                        pid_t proc2 = fork();
                                                        if(proc2 < 0)
                                                        {
                                                            printf("fork fail\n");
                                                            return -1;
                                                        }else if(proc2 == 0)
                                                        {
                                                            execlp("rm" , "rm", "-rvf" , "/home/pi/mongodb", (char *) 0); 
                                                            exit(0);
                                                        }else
                                                        {
                                                            int result = -1;
                                                            wait(&result);
                                                        }
                                                    }
                                                    pid_t proc3 = fork();
                                                    if(proc3 < 0)
                                                    {
                                                        printf("fork fail\n");
                                                        return -1;
                                                    }else if(proc3 == 0)
                                                    {   
                                                        execlp("tar", "tar", "zxvf", actualFileName, "-C","/home/pi/", (char *) 0);
                                                        exit(0);
                                                    }else
                                                    {
                                                        int result = -1;
                                                        wait(&result);
                                                    }
                                                }else
                                                {
                                                    char actualFileName[ExecuteFilePathLength];
                                                    memset(actualFileName, 0, sizeof(char)*ExecuteFilePathLength);
                                                    strcpy(actualFileName, targetPath);
                                                    strcat(actualFileName, targetName);
 
                                                    pid_t proc = fork();
                                                    if(proc < 0)
                                                    {
                                                        printf("fork fail\n");
                                                        return -1;
                                                    }else if(proc == 0)
                                                    {
                                                        execlp("chmod", "chmod", "744", road,(char *) 0);
                                                        exit(0);
                                                    }else
                                                    { 
                                                        int result = -1;
                                                        wait(&result);
                                                    }
                                                    pid_t proc_2 = fork();
                                                    if(proc_2 < 0)
                                                    {
                                                        printf("fork fail\n");
                                                        return -1;
                                                    }else if(proc_2 == 0)
                                                    {
                                                        unlink(actualFileName);
                                                        execlp("mv", "mv", road, actualFileName, (char *) 0);
                                                        exit(0);
                                                    }else
                                                    { 
                                                        int result = -1;
                                                        wait(&result);
                                                    }
                                                }
                                                break;
                                            }
                                        }
                                    }
                                    retryCount = retryCount -1;
                                    sleep(5);
                                }
                            }else
                            {
                                updateSuccess = updateSuccess + 1;
                                time(&now);
                                ts = *localtime(&now);
                                memset(buf, 0, sizeof(char)*80);
                                strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

                                printf("This file no need to upgrade\n");
                                pfile = fopen(LogFilePath, "a");
                                if(pfile != NULL)
                                {
                                    fprintf(pfile,"%s %s not need to update(crc correct)\n", buf, targetName);
                                    fclose(pfile);
                                }
                            }
                            if(retryCount <= 0)
                            {
                                time(&now);
                                ts = *localtime(&now);
                                memset(buf, 0, sizeof(char)*80);
                                strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

                                pfile = fopen(LogFilePath, "a");
                                if(pfile != NULL)
                                {
                                    fprintf(pfile,"%s %s fail to update(Max retry)\n", buf, targetName);
                                    fclose(pfile);
                                }
                                printf("Download Fail: MAX retry\n");
                                unlink(tempNewestFile);
                            }
                            memset(targetName, 0, sizeof(char)*FileNameSize);
                            memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);
                            memset(remoteFolder, 0, sizeof(char)*FileNameSize);
                            arrayPosition2 = 0;
                            fileNameEndFlag2 = 0;
                            break;
                        }
                        else
                        {
                            memset(targetName, 0, sizeof(char)*FileNameSize);
                            memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);
                            memset(remoteFolder, 0, sizeof(char)*FileNameSize);
                            arrayPosition2 = 0;
                            fileNameEndFlag2 = 0;
                        }
                    }
                    else;
                    ++charPositionForupdatelist;
                    --fileSize3;
                }
                memset(downloadFileName, 0, sizeof(char)*FileNameSize);
                memset(crcCheckValue, 0, sizeof(char)*FileNameSize);
                arrayPosition = 0;
                fileNameEndFlag = 0;

            }else;
            ++charPositionTempNewestFile;
            --fileSize;
        }
        if(bufferTempNewestFile != NULL)
        {
            printf("ready to free bufferTempNewestFile\n"); 
            free(bufferTempNewestFile);
            charPositionTempNewestFile = NULL;
            printf("free bufferTempNewestFile done\n"); 
        }

        if(bufferForupdatelist != NULL)
        {
            printf("ready to free bufferForupdatelist\n"); 
            free(bufferForupdatelist);
            charPositionForupdatelist = NULL;
            printf("free bufferForupdatelist done\n"); 
        }
        if(updateSuccess > 0)
        {
            printf("%d file update success\n", updateSuccess);
            //execlp("reboot", "reboot", (char *)0);
            pid_t proc_3 = fork();
            if(proc_3 < 0)
            {
                printf("fork fail\n");
                return -1;
            }else if(proc_3 == 0)
            {
                execlp("mv", "mv", tempNewestFile, NewestFileName, (char *)0);
                return 0;
            }else
            { 
                int result = -1;
                wait(&result);
            }
            time(&now);
            ts = *localtime(&now);
            strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

            //pfile = fopen(LogFilePath, "a");
            //if(pfile != NULL)
            //{
            //    fprintf(pfile,"%s update success\n", buf);
            //    fclose(pfile);
            //}
        }
    }else
    {
        printf("no need to update\n");
        time(&now);
        ts = *localtime(&now);
        strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

        pfile = fopen(LogFilePath, "a");
        if(pfile != NULL)
        {
            fprintf(pfile,"%s no need to update\n", buf);
            fclose(pfile);
        }
    }
    return 0;
}
