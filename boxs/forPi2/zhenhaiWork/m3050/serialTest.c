#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>
#include <termios.h>


int main ()
{
  int fd ;
  int count ;
  unsigned int nextTime ;
  struct termios options;
 
  if (wiringPiSetup () == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }
  if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }
  tcgetattr(fd, &options);
  options.c_cflag |= PARENB;
  options.c_cflag &= ~PARODD;
  options.c_cflag |= CSTOPB;
  //options.c_cflag |= PARENB;
  //options.c_cflag &= ~PARODD;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS7;
  tcsetattr(fd, TCSANOW, &options); 
  //tcsetattr(fd, &options); 

  for (;;)
  {
    unsigned char aaa[50];
    memset(aaa, 0, sizeof(char)*50);
    
    aaa[0] = 0x40;
    aaa[1] = 0x31;
    aaa[2] = 0x31;
    aaa[3] = 0x52;
    aaa[4] = 0x44;
    aaa[5] = 0x34;
    aaa[6] = 0x39;
    aaa[7] = 0x30;
    aaa[8] = 0x30;
    aaa[9] = 0x30;
    aaa[10] = 0x30;
    aaa[11] = 0x30;
    aaa[12] = 0x34;
    aaa[13] = 0x35;
    aaa[14] = 0x66;
    aaa[15] = 0x2a;
    aaa[16] = 0x0d;
    aaa[17] = 0x0a;
    
    int forCount = 0;
    for(forCount = 0; forCount < 18; ++forCount)
    {
        serialPutchar(fd, aaa[forCount]);
        printf("%x ", aaa[forCount]);
    }
    printf("\n");
    sleep(1);

    while(serialDataAvail(fd))
    {
        char xxx2 = serialGetchar(fd);
        printf ("%02c", xxx2);
    }
    printf("\n");
    sleep(1);
  }

  return 0 ;
}
