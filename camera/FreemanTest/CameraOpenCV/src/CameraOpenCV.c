/*
 * CameraOpenCV.c
 *
 *  Created on: Oct 24, 2014
 *      Author: freeman
 */

#include "ZhCvCommon.h"
#include <unistd.h>
#include <pthread.h>

#define column 320
#define row 240
#define Threshold 150
#define Error1XThreshold 10
#define Error1YThreshold 10
#define Error2YThreshold column/8

IplImage *dst;
IplImage *showSumX;
IplImage *showSumY;

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

void detectAndDisplay(IplImage *frame);
void Error2();
void Error1();

int main(void)
{
    IplImage* pFrameImg = NULL;
    IplImage* pGrayImg = NULL;
    IplImage* pForegroundImg = NULL;
    IplImage* pBackgroundImg_8U = NULL;
    IplImage* pBackgroundImg_32F = NULL;
    CvMemStorage *pcvMStorage = cvCreateMemStorage(0);
    CvSeq *pcvSeq = NULL;
//    CvCapture* capture = cvCaptureFromCAM(1);
    CvCapture *capture = cvCaptureFromFile("frames/noLight/%00008d.jpg"); // freeman 2014.10.29
    int nFrmNum = 0;
    int imgNameCount = 0;

    int key;

    if(!capture)
    {
    	fprintf(stderr, "Cannot get Camera! \n");
    	return 1;
    }

    pFrameImg = cvQueryFrame(capture);
    pBackgroundImg_8U =  cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_8U, 1);
    pBackgroundImg_32F = cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_32F, 1);
    pForegroundImg =     cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_8U,1);
    pGrayImg =           cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_8U,1);
    cvCvtColor(pFrameImg, pBackgroundImg_8U, CV_BGR2GRAY);
    cvCvtColor(pFrameImg, pForegroundImg, CV_BGR2GRAY);
    cvConvertScale(pBackgroundImg_8U, pBackgroundImg_32F, 1.0, 0.0);

    int file_counter = 1;

    while(pFrameImg)
    {
    	//IplImage* test = zhResize(pFrameImg, 0.5f, CV_INTER_LINEAR);
        cvCvtColor(pFrameImg, pGrayImg, CV_BGR2GRAY); // convert source image(pFrameImg) to gray(pGramImg)
        cvAbsDiff(pGrayImg, pBackgroundImg_8U, pForegroundImg);
        cvThreshold(pForegroundImg, pForegroundImg, 110, 255.0, CV_THRESH_BINARY);

        int contours_num = cvFindContours(pForegroundImg, pcvMStorage, &pcvSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
        // 當 contours_num == 0, 表示畫面是靜止的，這時候才開始判斷導線棒是否正確。

        printf("counter: %d,  contours_num: %d\n", file_counter++, contours_num);
        cvRunningAvg(pGrayImg, pBackgroundImg_32F, 1, 0);
        cvConvertScale(pBackgroundImg_32F, pBackgroundImg_8U , 1.0, 0.0);
//        zhReverse(pFrameImg);
        cvShowImage("webcam", pFrameImg);
        cvShowImage("background", pBackgroundImg_8U);
        cvShowImage("foreground", pForegroundImg);
        //cvWaitKey(30);
        pFrameImg = cvQueryFrame(capture);
        key = cvWaitKey(10);
        if (key == 27)
        {
            break;
        }
    }
    cvReleaseImage(&pFrameImg);
    cvReleaseImage(&pForegroundImg);
    cvReleaseImage(&pBackgroundImg_8U);
    cvReleaseImage(&pBackgroundImg_32F);
    cvReleaseImage(&pGrayImg);
    cvReleaseCapture(&capture);
    return 0;
}

void Error1()
{
    int ForCount1 = 0;
    int ForCount2 = 0;

    memset(SumX1, 0, sizeof(short)*column);
    for(ForCount1 = YUpperBoundPos - 1; ForCount1 >= (YUpperBoundPos/2); ForCount1--)
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
        for(ForCount2 = YLowerBoundPos+1; ForCount2 < row; ForCount2++)
        {
            SumX1[ForCount1] = SumX1[ForCount1] + (short)pointXY[ForCount1][ForCount2];
        }
        if(SumX1[ForCount1] > Error2YThreshold)
        {
            pass++;
        }
    }
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
    }
}


//
//int main(void)
//{
//    double scale=0.1;
//    IplImage *picture;
//    CvSize dst_cvsize;
//    IplImage *src=0;
//    IplImage *binary=0;
//
//
//
//    picture=cvLoadImage("/Users/Alex/mynode/picture1.jpg", 0);
//
//
//    src=ZhResize(picture, 0.1);
//    binary=zhCreateImage(src);
//
//    //zhImageToGray(picture, binary);
//    //binary=cvCreateImage( cvGetSize(src),IPL_DEPTH_8U,1);
//    //binary=cvCreateImage( cvGetSize(picture),IPL_DEPTH_8U,1);
//
//   // cvCvtColor(picture,binary,CV_BGR2GRAY);
//    zhEdge(src,binary);
//    cvXorS(binary, cvScalarAll(255), binary,0);         //将拍到的图像反色（闪一下形成拍照效果）
// //   zhAxisPaint(binary);
//   // cvCanny(dst, binary, 100, 100 * 3, 3);
//    cvNamedWindow("dst",CV_WINDOW_NORMAL);
//    cvNamedWindow("orin",CV_WINDOW_NORMAL);
//    cvShowImage("orin",src);
//    cvResizeWindow("dst",300,400);
//    cvShowImage("dst",binary);
//    cvWaitKey(0);
//    cvReleaseImage(&picture);
//    cvReleaseImage(&src);
//    cvDestroyWindow("dst");
//
//
//}


/* Sub-pixel interpolation methods */
//enum
//{
//    CV_INTER_NN        =0,
//    CV_INTER_LINEAR    =1,
//    CV_INTER_CUBIC     =2,
//    CV_INTER_AREA      =3,
//    CV_INTER_LANCZOS4  =4
//};



//縮小
//int main(void)
//{
//    double scale=0.1;
//    IplImage *picture;
//    CvSize dst_cvsize;
//    IplImage *dst=0;
//
//    picture=cvLoadImage("/Users/Alex/mynode/picture1.jpg", 0);
//    dst_cvsize.width=(int)(picture->width*scale);
//    dst_cvsize.height=(int)(picture->height*scale);
//    dst=cvCreateImage(dst_cvsize,picture->depth,picture->nChannels);
//    cvResize(picture,dst,CV_INTER_AREA);
//
//    cvNamedWindow("scr",CV_WINDOW_AUTOSIZE);
//    cvNamedWindow("dst",CV_WINDOW_AUTOSIZE);
//    cvShowImage("scr",picture);
//    cvShowImage("dst",dst);
//    cvWaitKey(0);
//    cvReleaseImage(&picture);
//    cvReleaseImage(&dst);
//    cvDestroyWindow("scr");
//    cvDestroyWindow("dst");
//
//
//}

// ----------------------------------------------------------------------------------------


//
//int main(int argc, char** argv)
//{
//
//
////
//	CvCapture* capture;
//    IplImage* greyImage;
//    IplImage *pImg = NULL;
//    IplImage* difference;
//    IplImage* temp;
//    IplImage* movingAverage;
//    CvSize imgSize;
//    CvCapture *cap = cvCaptureFromCAM(0);
//    cvSetCaptureProperty(cap,CV_CAP_PROP_FRAME_WIDTH,480);
//    cvSetCaptureProperty(cap,CV_CAP_PROP_FRAME_HEIGHT,320);
//    int first=1;
//    char fileName[100];
//    char key;
//    int count = 0;
//    while (1) {
//        pImg = cvQueryFrame(cap);
//        imgSize= cvGetSize(pImg);
//        greyImage= cvCreateImage( imgSize, IPL_DEPTH_8U, 1);
//        movingAverage= cvCreateImage( imgSize, IPL_DEPTH_32F, 3);
//        if(first==1)
//		{
//			difference = cvCloneImage(pImg);
//			temp = cvCloneImage(pImg);
//			cvConvertScale(pImg, movingAverage, 1.0, 0.0);
//			first = 0;
//		}
//        else
//		{
//			cvRunningAvg(pImg, movingAverage, 0.020, NULL);
//		}
//        cvConvertScale(movingAverage,temp, 1.0, 0.0);
//        cvAbsDiff(pImg,temp,difference);
//        cvCvtColor(difference,greyImage,CV_RGB2GRAY);
//
//        cvFlip(pImg, NULL, 1);
//        // key = cvWaitKey(1);
//        // if(key == 27) break;
//
//
////        if(key == 'c')
//        {
//            sprintf(fileName, "/Users/Alex/Picture %d.jpg", ++count);
//           // int a=cvSaveImage(fileName, pImg,0);
//
//          //  printf("%d",count);
//          //  cvXorS(pImg, cvScalarAll(255), pImg,0);
//           // cvShowImage("Camera",pImg);
//            //cvWaitKey(200);
//        }
//        cvShowImage("Camera",difference);
//        cvShowImage("Camera1",temp);
//        cvShowImage("Camera2",greyImage);
//    }
//    cvReleaseCapture(&cap);
//    return 0;
//
//
//
//}
//


//int main(int argc, char** argv)
//{
//	cvNamedWindow("vedio",0);
//	CvCapture* capture;
//    capture = cvCreateCameraCapture(0);
//    IplImage *pImg = NULL;
//    CvCapture *cap = cvCaptureFromCAM(0);
//    cvSetCaptureProperty(cap,CV_CAP_PROP_FRAME_WIDTH,480);
//    cvSetCaptureProperty(cap,CV_CAP_PROP_FRAME_HEIGHT,320);
//    char fileName[100];
//    char key;
//    int count = 0;
//    while(1)
//    {
//        pImg = cvQueryFrame(cap);
//
//        cvFlip(pImg, NULL, 1);
//       // key = cvWaitKey(1);
//       // if(key == 27) break;
//
//
//        if(key == 'c')
//        {
//            sprintf(fileName, "/Users/Alex/Picture %d.jpg", ++count);
//            int a=cvSaveImage(fileName, pImg,0);
//
//            printf("%d",count);
//            cvXorS(pImg, cvScalarAll(255), pImg,0);
//            cvShowImage("Camera",pImg);
//            cvWaitKey(200);
//        }
//        cvShowImage("Camera",pImg);
//    }
//    cvReleaseCapture(&cap);
//    return 0;
//
//
//
//}
//


//pthread





// fork .
//
//int main(void)
//{
//    pid_t pid;
//    pid=fork();
//    if(pid < 0)
//    {
//        printf("fail to fork \n");
//        exit(1);
//    }else if(pid==0)
//        printf("this is child, pid : %u \n",getpid());
//    else{
//        printf("this is parent , pid : %u , child pid : %u \n",getpid(),pid);
//    }
//    return 0;  b
//}
