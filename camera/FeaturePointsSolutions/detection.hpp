#ifndef __DECT_H__
#define __DECT_H__

#include <stdio.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/legacy/compat.hpp>
#include <gsl/gsl_fit.h>
#include <math.h>

CvMat * toBWImage(CvMat * src);
CvMat * toEdgeImage(CvMat * src);
CvMat * toGrayImage(IplImage * src);
IplImage * resizeTo(IplImage * src, int width, int height);
int isDot(int row, int column, CvMat * image);
int hasContinuePoint(int row, int column, CvMat * image);
int findTopBottom(CvMat * edgeImage, int * topLineY, int * bottomLineY);
int findLeftRight(CvSeq *imageKeypoints, int * leftLineX, int * rightLineX);
int isProturding(IplImage * image, 
                 int topLineY, int bottomLineY, int leftLineX, int rightLineX, 
                 CvSeq *imageKeypoints, int keyPointSizeThreshold, 
                 float outsideRatioThreshold, int distanceThreshold);

void drawAllFeaturePoints(IplImage * image, CvSeq * imageKeypoints);
 
int isRotate(IplImage * image, 
             int topLineY, int leftLineX, int rightLineX, CvSeq *imageKeypoints, 
             int keyPointSizeThreshold, int distanceThreshold,
             double absSlopeThreshold, double * outSlope);

void detectError(IplImage * image, CvMat * grayImage, CvMat * edgeImage, 
               int keyFrameFeature,
               int featureThreshold, int counter);

void detectErrorInBounds(
    IplImage * image, CvMat * grayImage, CvMat * edgeImage, 
    int keyFrameFeature,
    int featureThreshold, int counter, 
    int topLineY, int bottomLineY, int leftLineX, int rightLineX);


int getFeatureCount(CvMat * grayImage, int featureThreshold);

#endif

