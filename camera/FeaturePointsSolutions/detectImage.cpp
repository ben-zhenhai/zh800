#include "detection.hpp"

#include <sys/time.h>


int main(int argc, char ** argv) {

    if (argc != 2) {
      printf("usage: %s imageFile.jpg\n", argv[0]);
      exit(0);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    long startSec = tv.tv_sec;
    long startUsec = tv.tv_usec;
 
    IplImage * frame = cvLoadImage(argv[1], 1);
    IplImage * resized = resizeTo(frame, 320, 240);
    CvMat * grayImage = toGrayImage(resized);
    CvMat * edgeImage = toEdgeImage(grayImage);


    detectError(resized, grayImage, edgeImage, 100, 800, 0);

    gettimeofday(&tv, NULL);

    printf("use %ld seconds and %ld microseconds\n", tv.tv_sec - startSec, tv.tv_usec - startUsec);
 
    while (1) {
        cvShowImage("Image", resized);
        cvWaitKey(30);
    }
}
