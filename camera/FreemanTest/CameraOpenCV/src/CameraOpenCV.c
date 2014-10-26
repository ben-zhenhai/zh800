/*
 * CameraOpenCV.c
 *
 *  Created on: Oct 24, 2014
 *      Author: freeman
 */

#include "ZhCvCommon.h"
#include <unistd.h>
#include <pthread.h>

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

int main(void)
{
    IplImage* pFrameImg = NULL;
    IplImage* pGrayImg = NULL;
    IplImage* pForegroundImg = NULL;
    IplImage* pBackgroundImg_8U = NULL;
    IplImage* pBackgroundImg_32F = NULL;
    CvMemStorage *pcvMStorage = cvCreateMemStorage(0);
    CvSeq *pcvSeq = NULL;
    CvCapture* pCapture = cvCaptureFromCAM(0);
    int nFrmNum = 0;
    int imgNameCount = 0;
    while( pFrameImg = cvQueryFrame( pCapture ) )
    {
        nFrmNum++;
        if(nFrmNum == 1)
        {
            pBackgroundImg_8U = cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_8U, 1);
            pBackgroundImg_32F = cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_32F, 1);
            pForegroundImg = cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_8U,1);
            pGrayImg = cvCreateImage(cvSize(pFrameImg->width, pFrameImg->height), IPL_DEPTH_8U,1);
            cvCvtColor(pFrameImg, pBackgroundImg_8U, CV_BGR2GRAY);
            cvCvtColor(pFrameImg, pForegroundImg, CV_BGR2GRAY);
            cvConvertScale(pBackgroundImg_8U, pBackgroundImg_32F, 1.0, 0.0);
        }
        else
        {
            cvCvtColor(pFrameImg, pGrayImg, CV_BGR2GRAY);
            cvAbsDiff(pGrayImg, pBackgroundImg_8U, pForegroundImg);
            cvThreshold(pForegroundImg, pForegroundImg, 120, 255.0, CV_THRESH_BINARY);

            int contours_num=cvFindContours(pForegroundImg, pcvMStorage, &pcvSeq, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));
            printf("%d\n", contours_num);
            cvRunningAvg(pGrayImg, pBackgroundImg_32F, 1, 0);
            cvConvertScale(pBackgroundImg_32F, pBackgroundImg_8U , 1.0, 0.0);
            if (contours_num > 10 && nFrmNum > 3) {
                pFrameImg = cvQueryFrame(pCapture);
                cvShowImage("Camera", pFrameImg);

                // START - will save image to file
                char fileName[100];
                sprintf(fileName, "/home/freeman//Picture %d.jpg", imgNameCount++);
                cvSaveImage(fileName, pFrameImg, 0);
                // END   - will save image to file
            }
            cvShowImage("webcam", pFrameImg);
            cvShowImage("background", pBackgroundImg_8U);
            cvShowImage("foreground", pForegroundImg);
            cvWaitKey(30);
        }
    }
    cvReleaseImage(&pFrameImg);
    cvReleaseImage(&pForegroundImg);
    cvReleaseImage(&pBackgroundImg_8U);
    cvReleaseImage(&pBackgroundImg_32F);
    cvReleaseImage(&pGrayImg);
    cvReleaseCapture(&pCapture);
    return 0;
}
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
