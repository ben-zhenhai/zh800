#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringPi.h>
#include <wiringSerial.h>

int main ()
{
  int fd ;
  int count ;
  unsigned int nextTime ;

    char aaa[69] = { 0x01, 0x04, 0x44, 0xAC, 0x02, 0x03, 0xff, 0xff, 0x00, 0x03,
                     0x03, 0x03, 0x03, 0x00, 0x03, 0xff, 0x86, 0x02, 0xC8, 0x15,
                     0xff, 0xff, 0xff, 0xff, 0x28, 0x00, 0x05, 0x00, 0x06, 0x00,
                     0x73, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x15, 0x00, 0xc7, 0x05, 0x8e, 0x05, 0x1b, 0x1a, 
                     0x9a, 0x01, 0xff, 0xff, 0xff, 0xff, 0x10, 0x03, 0x00, 0x00,
                     0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
  {
    fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
    return 1 ;
  }

  if (wiringPiSetup () == -1)
  {
    fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
    return 1 ;
  }

  nextTime = millis () + 500 ;

  for (count = 0 ; count < 256 ; )
  {
    if (millis () > nextTime)
    {
      int forCount = 0;
      //printf ("\nOut: %3d: ", count) ;
      fflush (stdout) ;
      for(forCount = 0; forCount < 69; forCount++)
      {
        //printf("%x ", aaa[forCount]);
        serialPutchar (fd, aaa[forCount]) ;
      }
      //printf("\n");
      nextTime += 500 ;
      //++count ;
    }

  }

  printf ("\n") ;
  return 0 ;
}
