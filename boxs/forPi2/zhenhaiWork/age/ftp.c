#include "ftp.h"


size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int GetRemoteDataFunction()
{
    CURL *curl_handle;
    CURLcode res;
    long codep = 0;
    
    struct MemoryStruct chunk;
    char resumeDataAddress[INPUTLENGTH] = "http://192.168.3.2/api/machineStatus/";

    strcat(resumeDataAddress, MachineNo);
    strcat(resumeDataAddress, ".txt");

    printf("the web address is %s\n", resumeDataAddress);

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    //curl_easy_setopt(curl_handle, CURLOPT_URL, "http://www.example.com/");
    //curl_easy_setopt(curl_handle, CURLOPT_URL, "http://brianhsu.moe/A01.txt");
    curl_easy_setopt(curl_handle, CURLOPT_URL, resumeDataAddress);

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if(res != CURLE_OK) 
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file.
     *
     * Do something nice with it!
     */
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &codep);
        //printf("%lu bytes retrieved and return code is %ld\n", (long)chunk.size, codep);

        if(codep== 200)
        {
            printf("%lu bytes retrieved\n%s\n%ld\n", (long)chunk.size, chunk.memory, codep);

            char * htmlPtr;
            long htmlSize = (long)chunk.size;
            int forCount = 0;           
            int index = 1; 

            htmlPtr = chunk.memory;
            memset(ZHList->GoodNo, 0, sizeof(char)*INPUTLENGTH);

            for(forCount = 0; forCount < htmlSize; forCount++)
            {
                if(*htmlPtr == '\n')
                {
                    //create new node
                    while(1)
                    {
                        ZHNode = NULL;
                        ZHNode = (InputNode *)malloc(sizeof(InputNode));
                        if(ZHNode == NULL)
                        {
                            printf("alloc fail\n");
                            sleep(1);
                            continue;
                        }
                        memset(ZHNode->ISNo, 0, sizeof(char)*INPUTLENGTH);
                        memset(ZHNode->ManagerCard, 0, sizeof(char)*INPUTLENGTH);
                        memset(ZHNode->UserNo, 0, sizeof(char)*INPUTLENGTH);
                        memset(ZHNode->CountNo, 0, sizeof(char)*INPUTLENGTH);
                        memset(ZHNode->UploadFilePath, 0, sizeof(char)*INPUTLENGTH);
                        memset(ZHNode->GoodNo, 0, sizeof(char)*INPUTLENGTH); 
                        ZHNode->link = NULL;
                        break;
                    }
                    index = 1;
                    printf("|\n");
                    InputNode * tempNode = ZHList;
                    while(tempNode->link != NULL)
                    {
                        tempNode = tempNode->link;
                    }
                    tempNode->link = ZHNode;
                    OrderInBox++; 
                }
                else if(*htmlPtr != ' ')
                {
                    printf("%c", *htmlPtr);
                    switch(index)
                    {
                        case 1:
                            strncat(ZHNode->ISNo, htmlPtr, 1); 
                        break;
                        case 2:
                            strncat(ZHNode->ManagerCard, htmlPtr, 1); 
                        break;
                        case 3:
                            strncat(ZHNode->CountNo, htmlPtr, 1); 
                        break;
                        case 4:
                            strncat(ZHNode->GoodNo, htmlPtr, 1); 
                        break;
                        default:
                            printf("in default\n");;
                    }
                }
                else
                {
                    printf(" ");
                    index = index + 1;
                }
                htmlPtr++;
            }
            /* cleanup curl stuff */
            curl_easy_cleanup(curl_handle);
            free(chunk.memory);

            /* we're done with libcurl, so clean it up */
            curl_global_cleanup();
            printf("\n");
            return 0;
        }
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    free(chunk.memory);

    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();

    return 1;
}
