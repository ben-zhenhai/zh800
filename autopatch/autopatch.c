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
#define VersionNumberLength 8
#define FileNameSize 30
#define ExecuteFilePathLength 80
#define MaxRetryCount 5

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
        while(ep = readdir(dp))
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
                        getversion[forcount] =*(target+forcount);
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
        //printf("ready to write file\n");
        /* open file for writing */
        out->stream=fopen(out->filename, "wb");
        if(!out->stream)
        return -1; /* failure, can't open file to write */
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
    CURLcode res1, res2, res3;
    struct MemoryStruct chunk;
    char newestFile[FileNameSize];
    char currentFile[FileNameSize];
    char ftpFileName[FileNameSize];
    int needUpdate = 0;

    memset(newestFile, 0, sizeof(char)*FileNameSize);
    memset(currentFile, 0, sizeof(char)*FileNameSize);
    memset(ftpFileName, 0, sizeof(char)*FileNameSize);
 
    chunk.memory = malloc(1);  
    chunk.size = 0;   
    sleep(10);


    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    //curl_easy_setopt(curl_handle, CURLOPT_URL, "ftp://192.168.2.223:8888/");
    curl_easy_setopt(curl_handle, CURLOPT_URL, "ftp://192.168.20.100:21/upload/uploadFile/");
    //curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "raspberry:1234");
    curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "pi:raspberry");

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
                    printf(" %lu %s", newVersion, newversion);
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

    if(needUpdate){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_handle = curl_easy_init();
    
        struct FtpFile ftpfile, ftpfile2;
        if(curl_handle)
        {
            //char RemoteURL[80] = "ftp://192.168.2.223:8888/";
            char RemoteURL[80] = "ftp://192.168.20.100:21/upload/uploadFile/";
            strcat(RemoteURL, newestFile);
            //curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "raspberry:1234");
            curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "pi:raspberry");
       
            curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL);
            ftpfile.stream = NULL;
            ftpfile.filename = newestFile;  
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
        printf("current file %s %s\n", currentFile, newestFile);
        unlink(currentFile);

        FILE *pfile;
        char *buffer, *charPosition;
        pfile = fopen(newestFile, "r");
        fseek(pfile, 0, SEEK_END);
        int filesize = ftell(pfile);
        printf("filesize is %d\n", filesize);
        rewind(pfile);
        buffer = (char *)malloc(sizeof(char)*filesize);
        fread(buffer, 1, filesize, pfile);
        fclose(pfile);
        char downloadFileName[FileNameSize];
        char crcCheckValue[FileNameSize];
        int arrayPosition = 0;
        charPosition = buffer;
        short fileNameEndFlag = 0;

        memset(downloadFileName, 0, sizeof(char)*FileNameSize);
        memset(crcCheckValue, 0, sizeof(char)*FileNameSize);

        printf("ready to compare\n");

        while(filesize > 0)
        {   
            if(*charPosition != '\n' && *charPosition != 0x0d) 
            {
                if(*charPosition == ' ')
                {
                    fileNameEndFlag = 1;
                    arrayPosition  = 0;
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
            else if( *charPosition == '\n')
            {
                if(strcmp(downloadFileName, argv[1]) == 0)
                {
                    int retryCount = MaxRetryCount;
                    while(retryCount > 0)
                    {                  
                        curl_global_init(CURL_GLOBAL_DEFAULT);
                        curl_handle = curl_easy_init();
                        char road[ExecuteFilePathLength];
                        
                        memset(road, 0, sizeof(char)*ExecuteFilePathLength);
                        strcpy(road, argv[2]);
                        printf("%s %s %s\n", argv[1], argv[2], road);
                        //ftpfile3.filename = road;
                        //strcat(ftpfile3.filename, downloadFileName);
                        //printf("33 %s\n", ftpfile3.filename);
                        if(curl_handle)
                        {
                            //char RemoteURL2[80] = "ftp://192.168.2.223:8888/";
                            char RemoteURL2[80] = "ftp://192.168.20.100:21/upload/uploadFile/";
                            strcat(RemoteURL2, downloadFileName);
                            //curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "raspberry:1234");
                            curl_easy_setopt(curl_handle, CURLOPT_USERPWD, "pi:raspberry");
       
                            curl_easy_setopt(curl_handle, CURLOPT_URL, RemoteURL2);
                            ftpfile2.stream = NULL;
                            ftpfile2.filename = road;
                            strcat(ftpfile2.filename, downloadFileName);
                            printf("%s\n",ftpfile2.filename);
                            //ftpfile2.filename = downloadFileName;  
                            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_fwrite);
                            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &ftpfile2);

                            res3 = curl_easy_perform(curl_handle);
                            curl_easy_cleanup(curl_handle);
                            if(CURLE_OK != res3) {
                                fprintf(stderr, "curl told us %d\n", res3);
                            }
                        }

                        if(ftpfile2.stream)
                            fclose(ftpfile2.stream); /* close the local file */

                        curl_global_cleanup();
                        if(res3 == CURLE_OK)
                        {
                            //check crc
                            //pfile =  fopen(downloadFileName, "r");
                            pfile = fopen(ftpfile2.filename, "r");
                            unsigned char *crcBuffer;
                            fseek(pfile, 0, SEEK_END);
                            int fileSize = ftell(pfile);
                            rewind(pfile);
                            crcBuffer = (unsigned char *)malloc(sizeof(unsigned char)*fileSize);
                            fread(crcBuffer, 1, fileSize, pfile);
                            fclose(pfile);
                       
                            unsigned short crcResult = crcSlow(crcBuffer, fileSize);
                            free(crcBuffer);
                            if(crcResult == atoi(crcCheckValue))
                            {
                                printf("crc result:%d %d\n", crcResult, atoi(crcCheckValue));
                                break;
                            }
                        }
                        else
                        {
                            retryCount = retryCount -1;
                            sleep(5);
                                
                        }
                    }
                    break;
                }
                memset(downloadFileName, 0, sizeof(char)*FileNameSize);
                memset(crcCheckValue, 0, sizeof(char)*FileNameSize);
                arrayPosition = 0;
                fileNameEndFlag = 0;
            }else;
            ++charPosition;
            --filesize;
        }
        free(buffer);
        charPosition = NULL;

        if(res3 == CURLE_OK)
        {
            pid_t proc = fork();

            if(proc < 0)
            {
                printf("fork fail\n");
            
            }else if(proc == 0)
            {
                execlp("chmod", "chmod", "744", ftpfile2.filename, (char *) 0);
            }else
            {
                int result = -1;
                wait(&result);
                printf("ready to reboot...\n");
                execlp("reboot", "reboot", (char *)0);
            }
        }else
        {
            unlink(newestFile);
        }
    }else
    {
        printf("no need to update\n");
    }
    return 0;
}
