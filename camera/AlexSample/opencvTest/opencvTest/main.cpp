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
//char FileName[]="/Users/Alex/Desktop/mywork/opencvTest/opencvTest/IMG_0265.JPG";
char FileName[]="/Users/Alex/mynode/picture1.jpg";
char TrackbarName[]="Threshold";

int TrackbarInitValue=180;
int TrackbarStopValue=255;
CvCapture *capture;
CvCapture *capture2;
IplImage *frame;
IplImage *frame2;
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
void axisPaint(IplImage *Image);
int main()
{

//    cvNamedWindow("Show Image",0);
//    
//    cvResizeWindow("Show Image",300,400);

//    IplImage *frame;
    capture =cvCaptureFromCAM(0) ;
   // capture2 =cvCaptureFromCAM(1) ;
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,480);
    cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,320);
 //   cvSetCaptureProperty(capture2,CV_CAP_PROP_FRAME_WIDTH,480);
  //  cvSetCaptureProperty(capture2,CV_CAP_PROP_FRAME_HEIGHT,320);

    while (1) {
        frame = cvQueryFrame(capture);
       // frame2 = cvQueryFrame(capture2);
        ImageGray=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        ImageBinary = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
        Image2=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        Image3=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        ImageBinary2=cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
        cvCvtColor(frame,ImageGray,CV_RGB2GRAY);
        //cvCvtColor(frame,Image3,CV_RGB2GRAY);
        //cvCvtColor(Image2,ImageGray,CV_RGB2GRAY);
        cvCanny(ImageGray, Image3, 100, 100 * 3, 3);
        cvThreshold(ImageGray,ImageBinary,170,255,CV_THRESH_BINARY);
        //cvXorS(ImageBinary, cvScalarAll(255), ImageBinary,0);
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
        axisPaint(ImageBinary);
        CvScalar s,t;
        
        cvShowImage("WebcamB1",ImageBinary);
        //   cvShowImage("WebcamB1",frame2);
        //cvShowImage("WebcamB2",ImageBinary2);
        cvShowImage("WebcamGray",ImageGray);
        //cvShowImage("Webcam2",Image2);
        //cvShowImage("Webcam3",Image3);
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

void axisPaint(IplImage *Image)
{
    IplImage* paintx=cvCreateImage( cvGetSize(Image),IPL_DEPTH_8U, 1 );
    IplImage* painty=cvCreateImage( cvGetSize(Image),IPL_DEPTH_8U, 1 );
    cvZero(paintx);
    cvZero(painty);
    int* v=new int[Image->width];
    int* h=new int[Image->height];
    memset(v,0,Image->width*4);
    memset(h,0,Image->height*4);
    
    
    int x,y;
    CvScalar s,t;
    for(x=0;x<Image->width;x++)
    {
        for(y=0;y<Image->height;y++)
        {
            s=cvGet2D(Image,y,x);
            if(s.val[0]==0)
                v[x]++;
        }
    }
    
    for(x=0;x<Image->width;x++)
    {
        for(y=0;y<v[x];y++)
        {
            t.val[0]=255;
            cvSet2D(paintx,y,x,t);
        }
    }
    
    for(y=0;y<Image->height;y++)
    {
        for(x=0;x<Image->width;x++)
        {
            s=cvGet2D(Image,y,x);
            if(s.val[0]==0)
                h[y]++;
        }
    }
    for(y=0;y<Image->height;y++)
    {
        for(x=0;x<h[y];x++)
        {
            t.val[0]=255;
            cvSet2D(painty,y,x,t);
        }
    }
    cvNamedWindow("bariny",1);
    cvNamedWindow("xpaint",1);
    cvNamedWindow("ypaint",1);
    cvShowImage("bariny",Image);
    cvShowImage("ypaint",paintx);
    cvShowImage("xpaint",painty);
   
   
}
