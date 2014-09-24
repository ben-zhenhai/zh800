#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>
#include <pthread.h>
#include <curl/curl.h>

pthread_mutex_t mutex;

void *ErrorThread(void *argement)
{
    int fd;
    int flag = 0;

    while(1){
    pthread_mutex_lock(&mutex);
     if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
    //if ((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0)
    {
      fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
      pthread_exit(NULL);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    fflush(stdout);
    char array[27];

    array[0] = 0x02;
    array[1] = 0x37;
    array[2] = 0x46;
    array[3] = 0x30;
    array[4] = 0x30;
    array[5] = 0x48;
    array[6] = 0x57;
    array[7] = 0x4d;
    array[8] = 0x2c;
    array[9] = 0x31;
    array[10] = 0x2c;
    array[11] = 0x53;
    array[12] = 0x4d;
    array[13] = 0x39;
    array[14] = 0x31;
    array[15] = 0x36;
    array[16] = 0x2e;
    array[17] = 0x46;
    array[18] = 0x2c;
    array[19] = 0x30;
    array[20] = 0x31;
    array[21] = 0x2c;
    if(flag == 0){
        array[22] = 0x4e;
        flag = 1;
    }
    else
    {
        array[22] = 0x46;
        flag = 0;
    }
    array[23] = 0x03;
    array[24] = 0x0d;
    array[25] = 0x0a;
    array[26] = '\0';

    int x;
    /*for(x = 0; x < 27; x++)
    {
        printf("%x ", array[x]);
    }
    printf("\n");*/

    serialPuts(fd, array);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    close(fd);
    sleep(1);
    pthread_mutex_unlock(&mutex);
    sleep(1);
    }
}

void *CountThread(void *argument)
{
    int fd;
    int flag;
    char array[59];

    array[22] = 0x01;
    while(1){
    pthread_mutex_lock(&mutex);
     if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
    //if ((fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0)
    {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        pthread_exit(NULL);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    fflush(stdout);

    array[0] = 0x02;
    array[1] = 0x37;
    array[2] = 0x46;
    array[3] = 0x30;
    array[4] = 0x30;
    array[5] = 0x48;
    array[6] = 0x57;
    array[7] = 0x49;
    array[8] = 0x2c;
    array[9] = 0x31;
    array[10] = 0x2c;
    array[11] = 0x53;
    array[12] = 0x52;
    array[13] = 0x30;
    array[14] = 0x30;
    array[15] = 0x30;
    array[16] = 0x31;
    array[17] = 0x36;
    array[18] = 0x2c;
    array[19] = 0x31;
    array[20] = 0x36;
    array[21] = 0x2c;

    if(array[22] == 0x09)
    {
        array[22] = 0x01;
    }
    else array[22]++;
    array[23] = 0x26;
    array[24] = 0x01;
    array[25] = 0x01;

    array[26] = 0x67;
    array[27] = 0x27;
    array[28] = 0x01;
    array[29] = 0x01;

    array[30] = 0x79;
    array[31] = 0x29;
    array[32] = 0x01;
    array[33] = 0x01;

    array[34] = 0x19;
    array[35] = 0x14;
    array[36] = 0x01;
    array[37] = 0x01;

    array[38] = 0x58;
    array[39] = 0x01;
    array[40] = 0x01;
    array[41] = 0x01;

    array[42] = 0x0f;
    array[43] = 0x82;
    array[44] = 0x01;
    array[45] = 0x01;
    array[46] = 0x01;

    array[47] = 0x30;
    array[48] = 0x01;
    array[49] = 0x01;
    array[50] = 0x01;

    array[51] = 0x01;
    array[52] = 0x01;
    array[53] = 0x01;
    array[54] = 0x01;

    array[55] = 0x03;
    array[56] = 0x0d;
    array[57] = 0x0a;
    array[58] = '\0';

    int x;
    /*for(x = 0; x < 58; x++)
    {
        printf("%x ", array[x]);
    }
    printf("\n");*/
    serialPuts(fd, array);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    close(fd);
    sleep(1);
    pthread_mutex_unlock(&mutex);
    sleep(1);
    }
}

int main ()
{
    pthread_t errorThread, countThread;

    pthread_mutex_init(&mutex, NULL);

    if (wiringPiSetup () == -1)
    {
      fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
      return 1 ;
    }

    pthread_create(&errorThread, NULL, ErrorThread, NULL);
    pthread_create(&countThread, NULL, CountThread, NULL);
    while(1)
    {
        /*while(serialDataAvail(fd))
        {
            printf("%x ", serialGetchar(fd));
        }*/
        sleep(120);
        printf("while loop\n");
        break;
    }
    int s;
    s = pthread_cancel(errorThread);
    if( s != 0)
    {
        printf("s!=0 1\n");
    }
    printf("cancel 1\n");
    pthread_join(errorThread, NULL);
    s = pthread_cancel(countThread);
    if( s != 0)
    {
        printf("s!=0 2\n");
    }
    printf("cancel 2\n");
    pthread_join(errorThread, NULL);

    return 0 ;
}

