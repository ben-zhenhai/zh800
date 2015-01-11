#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <wiringPi.h>
#include <stdlib.h>

enum
{
    zhPH = 0,
    zhTEMP,
    zhEC,
};


void * SendSerialFunction(void * argument);
void * ReceiveSerialFunction(void * argument);
void * FtpFunction(void * argument);
void * RemoteFunction(void * argument);
unsigned int zhCRCCheck(unsigned char* data, int dataLength);
static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
void PerserFunction1(unsigned char* data);
void PerserFunction2(unsigned char* data, unsigned long** plong);

