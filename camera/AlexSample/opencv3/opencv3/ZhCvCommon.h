//
//  ZhCvCommon.h
//  opencv3
//
//  Created by Alex on 2014/10/14.
//  Copyright (c) 2014年 myAlex. All rights reserved.
//

#ifndef opencv3_ZhCvCommon_h
#define opencv3_ZhCvCommon_h
#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <stdlib.h>

#define ZhResize(Src,Scale) zhResize(Src,Scale,3)

IplImage *zhResize(IplImage *src,double scale,int reszieMethod);
IplImage *zhCreateImage( IplImage *refImage);
void zhImageToGray(IplImage *srcImage ,IplImage *dstImage);
#endif
