#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <curl/curl.h>

#include "crc.h"

#define Version "taicon_ver"
#define FrontNameLength 10
//#define Version "tcjcc_ver"
//#define FrontNameLength 9
#define VersionNumberLength 8
#define FileNameSize 40
#define ExecuteFilePathLength 80
#define MaxRetryCount 5
#define updatelist "updatelist"
#define TempFilepath "/home/pi/zhlog/"

struct FtpFile
{
    char *filename;
    FILE *stream;
};

struct MemoryStruct {
    char *memory;
    size_t size;
    
};

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
    /*a = contents;
    while(a != NULL)
    {
        printf("%c",*a);
        a++;
    }*/
    //printf("%s",contents);
    //printf("\n string length: %d\n", strlen(contents)+1);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main(int argc,char* argv[])
{
    CURL *curl_handle;
    CURLcode res1, res2, res3 ;
    struct MemoryStruct chunk;
    char newestFile[FileNameSize];
    char currentFile[FileNameSize];
    char ftpFileName[FileNameSize];
    char machine[FileNameSize];
    char configString[ExecuteFilePathLength];
    char updatelistFilePath[ExecuteFilePathLength];
    char tempNewestFile[ExecuteFilePathLength];
    short needUpdate = 0;
    FILE *pfile;
    char *buffer, *charPosition;
    int filesize = 0;
    int arrayIndex = 0;
    char ftpServer[ExecuteFilePathLength];
    char ftpServer_2[ExecuteFilePathLength];
    char AccPw[ExecuteFilePathLength]; 

    memset(newestFile, 0, sizeof(char)*FileNameSize);
    memset(currentFile, 0, sizeof(char)*FileNameSize);
    memset(ftpFileName, 0, sizeof(char)*FileNameSize);
    memset(machine, 0, sizeof(char)*FileNameSize);
    memset(configString, 0, sizeof(char)*ExecuteFilePathLength);
    memset(tempNewestFile, 0, sizeof(char)*ExecuteFilePathLength);
    memset(ftpServer, 0, sizeof(char)*ExecuteFilePathLength);    
    memset(ftpServer_2, 0, sizeof(char)*ExecuteFilePathLength);    
    memset(AccPw, 0, sizeof(char)*ExecuteFilePathLength);
    memset(updatelistFilePath, 0, sizeof(char)*ExecuteFilePathLength);

    pfile = fopen("config", "r");
    fseek(pfile, 0, SEEK_END);
    filesize = ftell(pfile);
    rewind(pfile);   
    buffer = (char *)malloc(sizeof(char)*filesize);
    fread(buffer, 1, filesize, pfile);
    fclose(pfile);
    charPosition = buffer;

    while(filesize > 0)
    {
        if(*charPosition == '\n')
        {
            if(strncmp(configString, "Server:", 7) == 0)
            {
                int forCount = 0;
                for(forCount = 0; forCount < arrayIndex-7; ++forCount)
                {
                    ftpServer[forCount] =  configString[forCount+7];
                }
                printf("%s|\n", ftpServer);
                memset(configString, 0, sizeof(char)*ExecuteFilePathLength);
            }
            else if(strncmp(configString, "AccPw:", 6) == 0)
            {
                int forCount = 0;
                for(forCount = 0; forCount < arrayIndex-6; ++forCount)
                {
                    AccPw[forCount] = configString[forCount+6];
                }
                //printf("%s|\n", AccPw);
                memset(configString, 0, sizeof(char)*ExecuteFilePathLength);
            }
            else if(strncmp(configString, "MachineType:", 12) == 0)
            {
                int forCount = 0;
                for(forCount = 0; forCount < arrayIndex-12; ++forCount)
                {
                    machine[forCount] = configString[forCount+12];    
                }
                printf("%s|\n", machine);
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
        --filesize;
    }
    if(buffer != NULL)
    {
        free(buffer);
        charPosition = NULL;
    }
    chunk.memory = malloc(1);  
    chunk.size = 0;   
    sleep(10);

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    strcpy(ftpServer_2, ftpServer);
    strcat(ftpServer_2, machine);
    curl_easy_setopt(curl_handle, CURLOPT_URL, ftpServer_2);
    curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    res1 = curl_easy_perform(curl_handle);

    if(res1 != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res1));
    }
    else {
        printf("%lu bytes retrieved\n", (long)chunk.size);
        unsigned stringLength = strlen(chunk.memory)+1;
        unsigned int forCount = 0;
        char *a;
        a = chunk.memory;
        unsigned long currentVersion = 0;
        int forcount = 0;
 
        GetNewFileName(newestFile);
        strcpy(currentFile, newestFile);
        char *target_1 = newestFile;
        char currentversion[VersionNumberLength+1];
    
        target_1 = target_1 + FrontNameLength;
        for(forcount = 0; forcount < VersionNumberLength; forcount++)
        {
            if(forcount != VersionNumberLength)
            {
                currentversion[forcount] = *(target_1 + forcount);
            }
            else
            {
                currentversion[forcount] = '\0';
            }
        }
        currentVersion = atoi(currentversion);

        printf("%ld The new File is: %s\n", currentVersion,newestFile);

        for(forCount = 0; forCount< stringLength; ++forCount)
        {
            if(*a == '\n')
            {
                if(strncmp(ftpFileName, Version, FrontNameLength) == 0)
                {
                    char *target = ftpFileName;
                    char newversion[VersionNumberLength+1];
                    unsigned long newVersion = 0;
                
                    memset(newversion, 0, sizeof(char)*(VersionNumberLength+1));
                    target = target + FrontNameLength;
                    for(forcount = 0; forcount < VersionNumberLength; forcount++)
                    {
                        if(forcount != VersionNumberLength)
                        {
                            newversion[forcount] = *(target+forcount);
                        }
                        else
                        {
                            //newversion[forcount] = '\0';
                        }
                    }
                    newVersion = atoi(newversion);
                    //printf(" %lu %s", newVersion, newversion);
                    if (newVersion > currentVersion)
                    {
                        needUpdate = 1;
                        memset(newestFile, 0, sizeof(char)*FileNameSize);
                        strcpy(newestFile, ftpFileName);
                    }
                    memset(ftpFileName, 0, sizeof(char)*FileNameSize);
                }
            }else if(*a == ' ')
            {
               memset(ftpFileName, 0, sizeof(char)*FileNameSize);
            }
            else
            {
                strncat(ftpFileName, a, sizeof(char)*1);
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


    //ready to update
    if(needUpdate){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle = curl_easy_init();
    
        struct FtpFile ftpfile, ftpfile2;
        int updateSuccess = 0;
        char downloadFileName[FileNameSize];
        char crcCheckValue[FileNameSize];
        int arrayPosition = 0;
        short fileNameEndFlag = 0;

        int filesize2;
        char *buffer2, *charPosition2;
        char targetName[FileNameSize];
        char targetPath[ExecuteFilePathLength];
        char machineType[FileNameSize];

        //update version list
        if(curl_handle)
        {
            char RemoteURL[ExecuteFilePathLength];

            memset(RemoteURL, 0, sizeof(char)*ExecuteFilePathLength);
            strcpy(RemoteURL, ftpServer);
            strcat(RemoteURL, machine);
            strcat(RemoteURL, newestFile);
            curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);
            curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL);
            strcpy(tempNewestFile,TempFilepath);
            strcat(tempNewestFile, newestFile);
            ftpfile.stream = NULL;
            ftpfile.filename = tempNewestFile;
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_fwrite);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &ftpfile);

            res2 = curl_easy_perform(curl_handle);
            curl_easy_cleanup(curl_handle);
            if(CURLE_OK != res2) {
                fprintf(stderr, "curl told us %d\n", res2);
                if(ftpfile.stream)
                    fclose(ftpfile.stream); /* close the local file */

                curl_global_cleanup();
                printf("no need to update\n");
                return 0;
            }
        }

        if(ftpfile.stream)
            fclose(ftpfile.stream); /* close the local file */

        curl_global_cleanup();
        //delete file
        printf("current file %s | %s\n", currentFile, tempNewestFile);
        unlink(currentFile);

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle = curl_easy_init();
    
        //update updatelist 
        //files need to update and  remote file path and local file path
        if(curl_handle)
        {
            char RemoteURL[ExecuteFilePathLength];

            memset(RemoteURL, 0, sizeof(char)*ExecuteFilePathLength);
            strcpy(RemoteURL, ftpServer);
            strcat(RemoteURL, machine);
            strcat(RemoteURL, updatelist);
            curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);
            curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL);
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
                printf("no need to update\n");
                return 0;
            }
        }

        if(ftpfile.stream)
            fclose(ftpfile.stream); /* close the local file */

        curl_global_cleanup();

        pfile = fopen(tempNewestFile, "r");
        fseek(pfile, 0, SEEK_END);
        filesize = ftell(pfile);
        printf("filesize is %d\n", filesize);
        rewind(pfile);
        buffer = (char *)malloc(sizeof(char)*filesize);
        fread(buffer, 1, filesize, pfile);
        fclose(pfile);
        charPosition = buffer;
        memset(downloadFileName, 0, sizeof(char)*FileNameSize);
        memset(crcCheckValue, 0, sizeof(char)*FileNameSize);

        printf("ready to compare %d\n", filesize);

        memset(targetName, 0, sizeof(char)*FileNameSize);
        memset(machineType, 0, sizeof(char)*FileNameSize);
        memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);

        pfile = fopen(updatelistFilePath, "r");
        fseek(pfile, 0, SEEK_END);
        filesize2 = ftell(pfile);
        printf("filesize is %d\n", filesize2);
        rewind(pfile);
        buffer2 = (char *)malloc(sizeof(char)*filesize2);
        fread(buffer2, 1, filesize2, pfile);
        fclose(pfile);

        while(filesize > 0)
        {   
            if(*charPosition != '\n' && *charPosition != 0x0d) 
            {
                if(*charPosition == ' ')
                {
                    fileNameEndFlag = 1;
                    arrayPosition  = 0;
                    charPosition++;
                    filesize--;
                }
                if(fileNameEndFlag == 0)
                {
                    downloadFileName[arrayPosition] = *charPosition;
                }else
                {
                    crcCheckValue[arrayPosition] = *charPosition;
                }
                ++arrayPosition;
            }
            else if(*charPosition == '\n')
            {
                short fileNameEndFlag2 = 0;
                int arrayPosition2 = 0;
                int filesize3 = 0;
                memset(targetName, 0, sizeof(char)*FileNameSize);
                memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);
                memset(machineType, 0, sizeof(char)*FileNameSize);
                filesize3 = filesize2;
                charPosition2 = buffer2;
                
                while(filesize3 > 0)
                {
                    if(*charPosition2 != '\n' && *charPosition2 != 0x0d) 
                    {
                        if(*charPosition2 == ' ')
                        {
                            fileNameEndFlag2 = fileNameEndFlag2 + 1;
                            arrayPosition2 = 0;
                            charPosition2++;
                            filesize3--;
                        }
                        if(fileNameEndFlag2 == 2)
                        {
                            machineType[arrayPosition2] = *charPosition2;
                        }
                        else if(fileNameEndFlag2 == 1)
                        {
                            targetPath[arrayPosition2] = *charPosition2;
                        }else
                        {
                            targetName[arrayPosition2] = *charPosition2;
                        }
                        ++arrayPosition2;
                    }
                    else if( *charPosition2 == '\n')
                    {
                        //printf("%s|%s|%s\n", targetName, targetPath, machineType);
                        if(strcmp(downloadFileName, targetName) == 0)
                        {
                            int retryCount = MaxRetryCount;
                            while(retryCount > 0)
                            {                  
                                curl_global_init(CURL_GLOBAL_DEFAULT);
                                curl_handle = curl_easy_init();
                                char road[ExecuteFilePathLength];
                                char actualFileName[ExecuteFilePathLength];                       
 
                                memset(road, 0, sizeof(char)*ExecuteFilePathLength);
                                memset(actualFileName, 0, sizeof(char)*ExecuteFilePathLength);
                                strcpy(road, targetPath);
                                printf("%s|%s|%s|\n", targetName, targetPath, road);
                                if(curl_handle)
                                {
                                    char RemoteURL2[ExecuteFilePathLength];
                                    memset(RemoteURL2, 0, sizeof(char)*ExecuteFilePathLength);
                                    strcpy(RemoteURL2, ftpServer);
                                    strcat(RemoteURL2, machineType);
                                    strcat(RemoteURL2, downloadFileName);
                                    curl_easy_setopt(curl_handle, CURLOPT_USERPWD, AccPw);
       
                                    curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL2);
                                    ftpfile2.stream = NULL;
                                    ftpfile2.filename = road;
                                    strcat(ftpfile2.filename, downloadFileName);
                                    strcpy(actualFileName, ftpfile2.filename);
                                    strcat(ftpfile2.filename, ".temp");
                                    printf("%s\n",ftpfile2.filename);
                                    //ftpfile2.filename = downloadFileName;  
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
                                    //check crc
                                    pfile = fopen(ftpfile2.filename, "r");
                                    unsigned char *crcBuffer;
                                    fseek(pfile, 0, SEEK_END);
                                    int fileSize = ftell(pfile);
                                    rewind(pfile);
                                    crcBuffer = (unsigned char *)malloc(sizeof(unsigned char)*fileSize);
                                    fread(crcBuffer, 1, fileSize, pfile);
                                    fclose(pfile);
                       
                                    unsigned short crcResult = crcSlow(crcBuffer, fileSize);
                                    printf("crc result:%d %d\n", crcResult, atoi(crcCheckValue));
                                    free(crcBuffer);
                                    
                                    if(crcResult == atoi(crcCheckValue))
                                    {
                                        updateSuccess = updateSuccess + 1;            
                                        //printf("crc result:%d %d\n", crcResult, atoi(crcCheckValue));
                                        char *checkFilename;
                                        int i = 0, j = 0;
                                        char c;
                                        checkFilename = (char *)malloc(sizeof(char)*strlen(ftpfile2.filename));
                                        strcpy(checkFilename, ftpfile2.filename);
                                        
                                        for(i = 0, j = strlen(checkFilename)-1; i < j; i++, j--)
                                        {
                                            c = checkFilename[i];
                                            checkFilename[i] = checkFilename[j];
                                            checkFilename[j] = c;
                                        }
                                        if(strncmp(checkFilename, "pmet.zg.rat", 11) == 0)
                                        {
                                            pid_t proc = fork();
                                            if(proc < 0)
                                            {
                                                printf("fork fail\n");
                                                return -1;
                                            }else if(proc == 0)
                                            {
                                                execlp("mv", "mv", ftpfile2.filename, actualFileName, (char *) 0);
                                                return 0;
                                            }else
                                            {
                                                int  result = -1;
                                                wait(&result);
                                            }
                                            printf("%s\n", actualFileName);
                                            pid_t proc2 = fork();
                                            if(proc2 < 0)
                                            {
                                                printf("fork fail\n");
                                                return -1;
                                            }else if(proc2 == 0)
                                            {
                                                execlp("rm" , "rm", "-rvf" , "/home/pi/mongodb", (char *) 0); 
                                            }else
                                            {
                                                int result = -1;
                                                wait(&result);
                                            }
                                            proc2 = fork();
                                            if(proc2 < 0)
                                            {
                                                printf("fork fail\n");
                                                return -1;
                                            }else if(proc2 == 0)
                                            {
                                                execlp("tar", "tar", "zxvf", actualFileName, "-C","/home/pi/", (char *) 0);
                                            }else
                                            {
                                                int result = -1;
                                                wait(&result);
                                            }

                                        }
                                        else
                                        {
                                            pid_t proc = fork();
                                            if(proc < 0)
                                            {
                                                printf("fork fail\n");
                                                return -1;
                                            }else if(proc == 0)
                                            {
                                                execlp("chmod", "chmod", "744", ftpfile2.filename, (char *) 0);
                                                return 0;
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
                                                execlp("mv", "mv", ftpfile2.filename, actualFileName, (char *) 0);
                                                return 0;
                                            }else
                                            { 
                                                int result = -1;
                                                wait(&result);
                                            }
                                        }
                                        free(checkFilename);
                                        break;
                                    }
                                }
                                retryCount = retryCount -1;
                                sleep(5);
                            }
                            if(retryCount <= 0)
                            {
                                printf("Download Fail: MAX retry\n");
                                unlink(tempNewestFile);
                            }
                            memset(targetName, 0, sizeof(char)*FileNameSize);
                            memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);
                            memset(machineType, 0, sizeof(char)*FileNameSize);
                            arrayPosition2 = 0;
                            fileNameEndFlag2 = 0;
                            break;
                        }
                        else
                        {
                            memset(targetName, 0, sizeof(char)*FileNameSize);
                            memset(targetPath, 0, sizeof(char)*ExecuteFilePathLength);
                            memset(machineType, 0, sizeof(char)*FileNameSize);
                            arrayPosition2 = 0;
                            fileNameEndFlag2 = 0;
                        }
                    }
                    else;
                    ++charPosition2;
                    --filesize3;
                }
                memset(downloadFileName, 0, sizeof(char)*FileNameSize);
                memset(crcCheckValue, 0, sizeof(char)*FileNameSize);
                arrayPosition = 0;
                fileNameEndFlag = 0;

            }else;
            ++charPosition;
            --filesize;
        }
        if(buffer != NULL)
        { 
            free(buffer);
            charPosition = NULL;
        }

        if(buffer2 != NULL)
        {
            free(buffer2);
            charPosition2 = NULL;      
        }
        if(updateSuccess > 0)
        {
            printf("%d file update success\nready to reboot...\n", updateSuccess);
            //execlp("reboot", "reboot", (char *)0);
            pid_t proc_3 = fork();
            if(proc_3 < 0)
            {
                printf("fork fail\n");
                return -1;
            }else if(proc_3 == 0)
            {
                execlp("mv", "mv", tempNewestFile, newestFile, (char *)0);
                return 0;
            }else
            { 
                int result = -1;
                wait(&result);
            }
            printf("oooooooooooo\n");
            pid_t proc_4 = fork();
            if(proc_4 < 0)
            {
                printf("fork fail\n");
                return -1;
            }else if(proc_4 == 0)
            {
                execlp("sync","sync",(char *)0);
                return 0;
            }else
            {
                int result = -1;
                wait(&result);
            }
        }
    }else
    {
        printf("no need to update\n");
    }
    return 0;
}
