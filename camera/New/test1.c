//
//  main.cpp
//  opencvTest
//
//  Created by Alex on 2014/4/12.
//  Copyright (c) 2014¦~ myAlex. All rights reserved.
//
//

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

//#include <stdlib.h>
void * WindowHandle1;
char FileName[]="/Users/Alex/Desktop/mywork/opencvTest/opencvTest/IMG_0265.JPG";

char TrackbarName[]="Threshold";

int TrackbarInitValue=180;
int TrackbarStopValue=255;
CvCapture *capture;
IplImage *frame;
IplImage *Image1;
IplImage *Image2;
CvSize Image2Size;
IplImage *Image3;
CvSize Image3Size;

CvMat *Matrix1;
int depth;
void onTrackbar(int position);

int main()
{

    

    
    IplImage *frame;
    capture =cvCaptureFromCAM(0) ;
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,480);
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,320);

    
    cvNamedWindow("Webcam",0);

    
        depth=cvGetTrackbarPos(TrackbarName,"Webcam");

        cvCreateTrackbar(TrackbarName,"Webcam",&TrackbarInitValue,TrackbarStopValue,onTrackbar);

cvWaitKey(0);

    

}
void onTrackbar(int position)
{
    frame = cvQueryFrame(capture);
    Image2=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
    Image3 = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
    cvCvtColor(frame,Image2,CV_RGB2GRAY);
    cvThreshold(Image2,Image3,position,255,CV_THRESH_BINARY);
    printf("pos=%d \n",position);
    cvShowImage("Webcam",Image3);
    //printf("matrix1=%s \n",Matrix1);
    //cvNot(Image2, Image3);

}