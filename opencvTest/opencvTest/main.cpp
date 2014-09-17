//
//  main.cpp
//  opencvTest
//
//  Created by Alex on 2014/4/12.
//  Copyright (c) 2014年 myAlex. All rights reserved.
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
IplImage *ImageGray;
IplImage *ImageBinary;
IplImage *ImageBinary2;
CvSize Image2Size;
IplImage *Image3;
CvSize Image3Size;

CvMat *Matrix1;
int depth;
void onTrackbar(int position);

int main()
{
    
    
//    IplImage *frame;
    capture =cvCaptureFromCAM(0) ;
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,480);
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,320);

    while (1) {
        frame = cvQueryFrame(capture);
        ImageGray=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        ImageBinary = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        Image2=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        Image3=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        ImageBinary2=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        cvCvtColor(frame,ImageGray,CV_RGB2GRAY);
        //cvCvtColor(frame,Image3,CV_RGB2GRAY);
        //cvCvtColor(Image2,ImageGray,CV_RGB2GRAY);
        cvCanny(ImageGray, Image3, 100, 100 * 3, 3);
        cvThreshold(ImageGray,ImageBinary,120,255,CV_THRESH_BINARY);
        //cvThreshold(Image3,ImageBinary2,104,255,CV_THRESH_BINARY);
  
        CvMemStorage *pcvMStorage = cvCreateMemStorage();
        CvSeq *pcvSeq = NULL;
        cvFindContours(ImageBinary, pcvMStorage, &pcvSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
        IplImage *pOutlineImage = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
        int nLevels = 5;
        cvRectangle(pOutlineImage, cvPoint(0, 0), cvPoint(pOutlineImage->width, pOutlineImage->height), CV_RGB(255, 255, 255), CV_FILLED);
        cvDrawContours(pOutlineImage, pcvSeq, CV_RGB(255,0,0), CV_RGB(0,255,0), nLevels, 2);
        // 显示轮廓图
//        cvNamedWindow(pstrWindowsOutLineTitle, CV_WINDOW_AUTOSIZE);
        cvShowImage("DrawCount", pOutlineImage);
        
        CvScalar s,t;
        
        cvShowImage("WebcamB1",ImageBinary);
        //cvShowImage("WebcamB2",ImageBinary2);
        cvShowImage("WebcamGray",ImageGray);
        //cvShowImage("Webcam2",Image2);
        cvShowImage("Webcam3",Image3);
        cvNamedWindow("Webcam",0);
    }




    

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


