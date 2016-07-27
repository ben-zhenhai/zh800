#include "ftp.h"

//用來抓去server 資料的 method
//主要是改 cURL 的 sample code 

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

//目前撈data的網頁位置 192.168.3.2/api/machineStatus/xxx.txt, xxx表示機台號碼 ex A2600-01.txt

int GetRemoteDataFunction()
{
    CURL *curl_handle;
    CURLcode res;
    long codep = 0;
    
    struct MemoryStruct chunk;
    char resumeDataAddress[INPUTLENGTH] = "http://192.168.3.2/api/machineStatus/";
    //char resumeDataAddress[INPUTLENGTH] = "http://192.168.0.141:8081/api/machineStatus/";

    strcat(resumeDataAddress, MachineNo);
    strcat(resumeDataAddress, ".txt");

    printf("the web address is %s\n", resumeDataAddress);

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
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
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
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
        printf("%lu bytes retrieved and return code is %ld\n", (long)chunk.size, codep);

        if(codep== 200)
        {
            char * htmlPtr;
            long htmlSize = (long)chunk.size;
            int forCount = 0;           
            int index = 1; 

            htmlPtr = chunk.memory;
            memset(GoodNo, 0, sizeof(char)*INPUTLENGTH);

            for(forCount = 0; forCount < htmlSize; forCount++)
            {
                if(*htmlPtr != ' ')
                {
                    printf("%c", *htmlPtr);
                    switch(index)
                    {
                        case 1:
                            strncat(ISNo, htmlPtr, 1); 
                        break;
                        case 2:
                            strncat(ManagerCard, htmlPtr, 1); 
                        break;
                        case 3:
                            strncat(CountNo, htmlPtr, 1); 
                        break;
                        case 4:
                            strncat(GoodNo, htmlPtr, 1); 
                        break;
                        default:
                            ;
                    }
                }
                else
                {
                    printf("\n");
                    index = index + 1;
                }
                htmlPtr++;
            }
            printf("\n");
            /* cleanup curl stuff */
            curl_easy_cleanup(curl_handle);
            free(chunk.memory);

            /* we're done with libcurl, so clean it up */
            curl_global_cleanup();
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
