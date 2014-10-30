/*
 * image_verifier.c
 *
 *  Created on: Oct 29, 2014
 *      Author: freeman
 */

//
//  main.cpp
//  opencvTest
//
//  Created by Alex on 2014/4/12.
//  Copyright (c) 2014Š~ myAlex. All rights reserved.
//
//

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <wiringPi.h>

#define column 320
#define row 240
#define Threshold 150
#define Error1XThreshold 10
#define Error1YThreshold 10
#define Error2YThreshold column/8
//#define Error2XThreshold 30

#define Detail // freeman
#define showXY // freeman

IplImage *dst;
IplImage *showSumX;
IplImage *showSumY;

CvCapture *capture;

char pointXY[column][row];
short SumX1[column];

short SumX[column];
short SumY[row];


int localmaxY = 0;
int valueY = 0;

int YUpperBoundvalue = 0;
int YUpperBoundPos = 0;
int YLowerBoundvalue = 0;
int YLowerBoundPos = 0;

int XIntegralmax = 0;
int TempXIntegralmaxLeft = 0;
int TempXIntegralmaxRight = 0;

int localmaxX = 0;
int valueX = 0;

void detectAndDisplay(IplImage *frame);
void Error2();
void Error1();


void Error1()
{
    int ForCount1 = 0;
    int ForCount2 = 0;

    memset(SumX1, 0, sizeof(short)*column);
/*    for(ForCount1 = 0; ForCount1 < column; ForCount1++)
    {
        for(ForCount2 = YUpperBoundPos -1; ForCount2 >= (YUpperBoundPos/2); ForCount2--)
        {
            SumX1[ForCount1] = SumX1[ForCount1] + pointXY[ForCount1][ForCount1];
        }
        if(ForCount1 > 3 || SumX1[ForCount1] > 4 && SumX1[ForCount1-1] > 4 && SumX1[ForCount1-2] > 4 && SumX1[ForCount1-3] >4 )
        {
            for(ForCount2 = YUpperBoundPos -1; ForCount2 >= (YUpperBoundPos/2); ForCount2--)
            {
                if(pointXY[ForCount1][ForCount2] == 1)
                {
                    CvScalar s = cvGet2D(dst,ForCount2,ForCount1);
                    s.val[0] = 150;
                    cvSet2D(dst, ForCount2, ForCount1, s);

                }
                if(pointXY[ForCount1-1][ForCount2] == 1)
                {
                    CvScalar s = cvGet2D(dst,ForCount2,ForCount1-1);
                    s.val[0] = 150;
                    cvSet2D(dst, ForCount2, ForCount1-1, s);

                }
                if(pointXY[ForCount1-2][ForCount2] == 1)
                {
                    CvScalar s = cvGet2D(dst,ForCount2,ForCount1-2);
                    s.val[0] = 150;
                    cvSet2D(dst, ForCount2, ForCount1-2, s);

                }
                if(pointXY[ForCount1-3][ForCount2] == 1)
                {
                    CvScalar s = cvGet2D(dst,ForCount2,ForCount1-3);
                    s.val[0] = 150;
                    cvSet2D(dst, ForCount2, ForCount1-3, s);

                }

            }
            printf("Error1\n");
            break;
        }
    }
*/
    for(ForCount1 = YUpperBoundPos-1; ForCount1 >= (YUpperBoundPos/2); ForCount1--)
    {

        if(SumY[ForCount1] > Error1YThreshold && SumY[ForCount1-1] > Error1YThreshold && SumY[ForCount1-2] > Error1YThreshold )
        {
            for(ForCount2 = 0; ForCount2 < column; ForCount2++)
            {
                if(pointXY[ForCount2][ForCount1] == 1)
                {
                    CvScalar s = cvGet2D(dst,ForCount1,ForCount2);
                    s.val[0] = 150;
                    cvSet2D(dst, ForCount1, ForCount2, s);
                    cvSet2D(dst, ForCount1-1, ForCount2, s);
                    cvSet2D(dst, ForCount1-2, ForCount2, s);
                }
            }
            printf("Error1\n\n");
            break;
        }
    }
}

void Error2()
{
    int ForCount1;
    int ForCount2;
    ForCount1 = ForCount2 = 0;
    int ValueCount = 0;
    int pass = 0;

    memset(SumX1, 0, sizeof(short)*column);

    for(ForCount1 = 0; ForCount1 < column; ForCount1++)
    {
        //printf("%d", SumX1[ForCount1]);
        for(ForCount2 = YLowerBoundPos+1; ForCount2 < row; ForCount2++)
        {
            SumX1[ForCount1] = SumX1[ForCount1] + (short)pointXY[ForCount1][ForCount2];
        }
        //printf("%d %d:%d\n", YLowerBoundPos,ForCount1, SumX1[ForCount1]);
        if(SumX1[ForCount1] > Error2YThreshold)
        //if(SumX1[ForCount1] > 1)
        {
            //printf("%d\n", SumX1[ForCount1]);
            pass++;
            //ValueCount++;
            /*for(ForCount2 = YLowerBoundPos+1; ForCount2 < row; ForCount2++)
            {
                if(pointXY[ForCount1][ForCount2] == 1)
                {
                    CvScalar s = cvGet2D(dst,ForCount2,ForCount1);
                    s.val[0] = 70;
                    cvSet2D(dst, ForCount2, ForCount1, s);
                }
            }*/
        }
    }
    //if(ValueCount > Error2XThreshold) printf("Error2\n");
    if(pass == 0)
    {
        for(ForCount2 = YLowerBoundPos+1; ForCount2 < row; ForCount2++)
        {
            for(ForCount1 = 0; ForCount1 < column; ForCount1++)
            {
                if(pointXY[ForCount1][ForCount2] == 1)
                {
                    CvScalar s = cvGet2D(dst,ForCount2,ForCount1);
                    s.val[0] = 70;
                    cvSet2D(dst, ForCount2, ForCount1, s);
                }
            }
        }
    }
}

void Integral(int limit)
{
    struct timeval time1, time2;
    int ForCount =0;
    int ForCount2 = 0;
    char Flag = 0;

    //YUpperBoundPos = YUpperBoundvalue = YLowerBoundPos = YLowerBoundvalue = 0;

    gettimeofday(&time1, NULL);
    if(limit == column)
    {
        for(ForCount = 0; ForCount < column; ForCount++)
        {
            for(ForCount2 = 0; ForCount2 < row; ForCount2++)
            {
                SumX[ForCount] = SumX[ForCount] + (short)pointXY[ForCount][ForCount2];
            }
        }
    }
    else
    {
        localmaxY = valueY = 0;

        for(ForCount = 0; ForCount < row; ForCount++)
        {
            for(ForCount2 = 0; ForCount2 < column; ForCount2++)
            {
                SumY[ForCount] = SumY[ForCount]+(short)pointXY[ForCount2][ForCount];
            }
            if(valueY == 0 && SumY[ForCount] > (column/3))
            {
                YUpperBoundPos = ForCount;
                valueY = SumY[ForCount];
                YUpperBoundvalue = valueY;
            }
            else if(valueY != 0 && SumY[ForCount] > (column/3))
            {
                YLowerBoundPos = ForCount;
                YLowerBoundvalue = SumY[ForCount];
                valueY = SumY[ForCount];
            }else;
        }
        gettimeofday(&time2, NULL);
        for(ForCount = 0; ForCount < column; ForCount++)
        {
            CvScalar s = cvGet2D(dst,YLowerBoundPos,ForCount);
            s.val[0] = 200;
            cvSet2D(dst, YLowerBoundPos, ForCount, s);
        }
        //printf("LowerBoundPos:%d LowerBoundvalue:%d \nUpperBoundPos:%d UpperBoundValue:%d\n", YLowerBoundPos, YLowerBoundvalue,
        //                                                                                      YUpperBoundPos, YUpperBoundvalue);
    }

}

int main()
{
    //capture =cvCaptureFromCAM(0) ;
	//capture = cvCaptureFromFile("frames/noLight/%00008d.jpg");
//    if(capture)
	if(1)
    {
        IplImage *frame1, *frame2;
//        cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH,column);
//        cvSetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT,row);
        for(;;)
        {
//            frame1 = cvQueryFrame(capture);
            frame1 = cvLoadImage("test.jpg", 1);
            if(!frame1)
            {
                printf("No captured frame ;\n");
                break;
            }
            else
            {
                memset(SumX, 0, sizeof(short)*column);
                memset(SumY, 0, sizeof(short)*row);

                YUpperBoundPos = YUpperBoundvalue = YLowerBoundPos = YLowerBoundvalue = 0;
                frame2 = cvCreateImage( cvSize(column, row),frame1->depth, frame1->nChannels);
                cvResize(frame1, frame2, CV_INTER_LINEAR);
                detectAndDisplay(frame2);
                //printf("%d %d", frame1->width, frame1->height);
                sleep(0);
                //break;
            }

            char c = cvWaitKey(100);
            if((char)c == 'c' || (char)c == 'C')
            {
                break;
            }
        }
    }
}

void detectAndDisplay(IplImage *frame)
{
    IplImage *frame_gray;
    int ForCount1, ForCount2;
    ForCount1 = ForCount2 = 0;

    memset(pointXY, 0, sizeof(char)*column*row);

    frame_gray = cvCreateImage( cvGetSize(frame),IPL_DEPTH_8U,1);
    //frame_gray = cvCreateImage( cvSize(column, row),IPL_DEPTH_8U,1);
    dst = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
    //dst = cvCreateImage(cvSize(column,row),IPL_DEPTH_8U,1);

#ifdef showXY
    showSumX = cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U,1);
    showSumY = cvCreateImage(cvSize(row,column),IPL_DEPTH_8U,1);
#endif

    cvCvtColor(frame, frame_gray, CV_BGR2GRAY);
    //cvCanny(frame_gray, dst, 40, 40*3, 3);
    cvThreshold(frame_gray, dst, Threshold, 255, CV_THRESH_BINARY);

    for(ForCount1 = 0; ForCount1 < column; ForCount1++)
    {
        for(ForCount2 = 0; ForCount2 < row; ForCount2++)
        {
            CvScalar s = cvGet2D(dst,ForCount2,ForCount1);
            //char s = ((uchar *)(dst->imageData + ForCount1*dst->widthStep))[ForCount2];
#ifdef Detail
            printf("%3d %3d %f\n",ForCount1,ForCount2, s.val[0]);
#endif
            /*if( s.val[0] <= Threshold)
            {
                pointXY[ForCount1][ForCount2] = 1;
            }*/
            if(s.val[0] <= Threshold)
            {
                pointXY[ForCount1][ForCount2] = 0;
            }
            else
            {
                pointXY[ForCount1][ForCount2] = 1;
            }
        }
    }

    Integral(row);
    Integral(column);
    Error1();
    Error2();

    cvShowImage("Webcam",dst);
    cvShowImage("Webcam1",frame_gray);

#ifdef Detail
    for(ForCount1 = 0; ForCount1 < column; ForCount1++)
    {
        printf("x[%3d]:%d\n", ForCount1, SumX[ForCount1]);
    }
    printf("\n");
    for(ForCount1 = 0; ForCount1 < row; ForCount1++)
    {
        printf("y[%3d]:%d\n", ForCount1, SumY[ForCount1]);
    }
    printf("\n");
#endif

#ifdef showXY
    for(ForCount1 = 0; ForCount1 < column; ForCount1++)
    {
        for(ForCount2 = 0; ForCount2 < (int)SumX[ForCount1]; ForCount2++)
        {
            CvScalar s = cvGet2D(showSumX,ForCount2,ForCount1);
            s.val[0] = 255;
            cvSet2D(showSumX, ForCount2, ForCount1, s);
        }
    }
    cvShowImage("SumX", showSumX);

    for(ForCount1 = 0; ForCount1 < row; ForCount1++)
    {
        for(ForCount2 = 0; ForCount2 < (int)SumY[ForCount1]; ForCount2++)
        {
            CvScalar s = cvGet2D(showSumY,ForCount2,ForCount1);
            s.val[0] = 255;
            cvSet2D(showSumY, ForCount2, ForCount1, s);

        }
    }

#endif

    cvShowImage("SumY", showSumY);
    cvReleaseImage( &dst );
    cvReleaseImage( &frame_gray );
#ifdef showXY
    cvReleaseImage( &showSumX );
    cvReleaseImage( &showSumY );
#endif
}
