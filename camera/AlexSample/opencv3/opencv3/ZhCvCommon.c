//
//  ZhCvCommon.c
//  opencv3
//
//  Created by Alex on 2014/10/14.
//  Copyright (c) 2014年 myAlex. All rights reserved.
//

#include "ZhCvCommon.h"


IplImage *zhResize(IplImage *src,double scale,int reszieMethod)
{
    CvSize dst_cvsize;
    IplImage *dst=0;
    dst_cvsize.width=(int)(src->width*scale);
    dst_cvsize.height=(int)(src->height*scale);
    dst=cvCreateImage(dst_cvsize,src->depth,src->nChannels);
    cvResize(src,dst,reszieMethod);
    return dst;
    
}

void zhReverse(IplImage *src)
{
    cvXorS(src, cvScalarAll(255), src,0);
}
void zhImageToGray(IplImage *srcImage ,IplImage *dstImage)
{
    cvCvtColor(srcImage,dstImage,CV_RGB2GRAY);
}
void zhBinaryfromGray(IplImage *srcImage ,IplImage *dstImage)
{
    cvThreshold(srcImage,dstImage,120,255,CV_THRESH_BINARY);
    
    
}
void zhEdge(IplImage *srcImage ,IplImage *dstImage)
{
        cvCanny(srcImage, dstImage, 100, 100 * 3, 3);
}
IplImage* zhCreateImage( IplImage *refImage)
{
    IplImage *dstImage;
    dstImage = cvCreateImage(cvGetSize(refImage),IPL_DEPTH_8U,1);
    return dstImage;
}
IplImage *zhBinaryfromColor(IplImage *srcImage ,IplImage *ImageBinary )
{
    IplImage *ImageGray=0;

    ImageGray=cvCreateImage( cvGetSize(srcImage),IPL_DEPTH_8U,1);
    cvCvtColor(srcImage,ImageGray,CV_RGB2GRAY);
    cvThreshold(ImageGray,ImageBinary,120,255,CV_THRESH_BINARY);
    cvReleaseImage(&ImageGray);
    return ImageBinary;
}
void zhAxisPaint(IplImage *Image)
{
    IplImage* paintx=cvCreateImage( cvGetSize(Image),IPL_DEPTH_8U, 1 );
    IplImage* painty=cvCreateImage( cvGetSize(Image),IPL_DEPTH_8U, 1 );
    cvZero(paintx);
    cvZero(painty);
    int* v=malloc(Image->width);
    int* h=malloc(Image->height);
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
