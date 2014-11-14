#include "detection.hpp"

CvMat * toBWImage(CvMat * src) {
    CvMat *dest = cvCreateMat(src->height, src->width, CV_8UC1);
    cvThreshold(src, dest, 120, 255, CV_THRESH_BINARY);
    return dest;
}

CvMat * toEdgeImage(CvMat * src) {
    CvMat *dest = cvCreateMat(src->height, src->width, CV_8UC1);
    cvCanny(src, dest, 50, 150, 3);
    return dest;
}

CvMat * toGrayImage(IplImage * src) {
    CvMat * dest = cvCreateMat(src->height, src->width, CV_8UC1);
    cvCvtColor(src, dest, CV_BGR2GRAY);
    return dest;
}

IplImage * resizeTo(IplImage * src, int width, int height) {
    IplImage * dest = NULL;
    CvSize size;

    size.width=width;
    size.height=height;

    dest = cvCreateImage(size,src->depth,src->nChannels);
    cvResize(src, dest, CV_INTER_LINEAR);
    return dest;
}

int isDot(int row, int column, CvMat * image) {
    CvScalar s = cvGet2D(image, row, column);
    return s.val[0] > 0 ? 1 : 0;
}

int hasContinuePoint(int row, int column, CvMat * image) {
    int minRow = row - 5 >= 0 ? row - 5 : 0;
    int maxRow = row + 5 < image->height ? row + 5 : image->height - 1;
    int maxColumn = (column + 1) < image->width ? (column + 1) : image->width - 1;

    for (int i = minRow; i < maxRow; i++) {
        if (isDot(i, maxColumn, image)) {
            return 1;
        }
    }
    return 0;
}

int findTopBottom(CvMat * edgeImage, int * topLineY, int * bottomLineY) {
    *topLineY = 65535;
    *bottomLineY = -1;

    for (int i = 0; i < edgeImage->height; i++) {
        
        int isLine = isDot(i, 0, edgeImage);

        for (int j = 0; j < edgeImage->width / 3; j++) {
            int isContinue = hasContinuePoint(i, j, edgeImage);
            isLine = isLine && isContinue;
        }

        if (isLine) {

            if (*topLineY > i) {
                *topLineY = i;
            }

            if (*bottomLineY < i) {
                *bottomLineY = i;
            }
        }
    }
}

int findLeftRight(CvSeq *imageKeypoints, int * leftLineX, int * rightLineX) {
    *leftLineX = 65535;
    *rightLineX = -1;

    for( int i = 0; i < imageKeypoints->total; i++ )
    {
        CvSURFPoint* r = (CvSURFPoint*)cvGetSeqElem( imageKeypoints, i );
        if (r->size >= 40 && r->size <= 80) {

          if (*leftLineX > r->pt.x) {
            *leftLineX = (int) r->pt.x;
          }

          if (*rightLineX <= r->pt.x) {
            *rightLineX = (int) r->pt.x;
          }
        }
    }
}

int isProturding(IplImage * image, 
                 int topLineY, int bottomLineY, int leftLineX, int rightLineX, 
                 CvSeq *imageKeypoints, int keyPointSizeThreshold, 
                 float outsideRatioThreshold, int distanceThreshold) {

    int isOverlapping = 0;
    int proturdingCount = 0;
    int insideRegionCount = 0;

    for(int i = 0; i < imageKeypoints->total; i++ )
    {
        CvSURFPoint* r = (CvSURFPoint*)cvGetSeqElem( imageKeypoints, i );
        CvPoint center;
        int radius;
        center.x = cvRound(r->pt.x);
        center.y = cvRound(r->pt.y);
        radius = cvRound(r->size*1.2/9.*2);

        if (r->size <= keyPointSizeThreshold) {
            if (r->pt.x >= leftLineX + 5 && r->pt.x <= rightLineX - 5) {
                insideRegionCount++;
                if (r->pt.y <= (topLineY + distanceThreshold)) {
                    proturdingCount++;
                }
            }
        }
    }

    if (insideRegionCount == 0) {
        return 0;
    } else {
        double ratio = (double) proturdingCount / insideRegionCount;
        return ratio >= outsideRatioThreshold;
    }
}

int isRotate(IplImage * image, 
             int topLineY, int leftLineX, int rightLineX, CvSeq *imageKeypoints, 
             int keyPointSizeThreshold, int distanceThreshold,
             double slopeAbsThreshold, double * outSlope) {

    int validPointCount = 0;

    for(int i = 0; i < imageKeypoints->total; i++ )
    {
        CvSURFPoint* r = (CvSURFPoint*)cvGetSeqElem( imageKeypoints, i );
        if (r->size <= keyPointSizeThreshold) {
            int insideLeftRight = r->pt.x >= leftLineX + 5 && r->pt.x <= rightLineX - 5;
            int insideTop = r->pt.y >= (topLineY + distanceThreshold);

            if (insideLeftRight && insideTop) {
                validPointCount++;
            }
        }
    }

    double * xOfPoints = (double *) malloc(sizeof(double) * validPointCount);
    double * yOfPoints = (double *) malloc(sizeof(double) * validPointCount);
    double * weights = (double *) malloc(sizeof(double) * validPointCount);

    for(int i = 0, counter = 0; i < imageKeypoints->total; i++ )
    {
        CvSURFPoint* r = (CvSURFPoint*)cvGetSeqElem( imageKeypoints, i );
        if (r->size <= keyPointSizeThreshold) {
            int insideLeftRight = r->pt.x >= leftLineX + 5 && r->pt.x <= rightLineX - 5;
            int insideTop = r->pt.y >= (topLineY + distanceThreshold);

            if (insideLeftRight && insideTop) {
                xOfPoints[counter] = r->pt.x;
                yOfPoints[counter] = r->pt.y;
                weights[counter] = 10.0 / r->size;
                counter++;
            }
        }
    }
    
    double c0, slope, cov00, cov01, cov11, sumsq;

    gsl_fit_wlinear (
        yOfPoints, 1, weights, 1, xOfPoints, 1, 
        validPointCount, &c0, &slope, &cov00, &cov01, &cov11, &sumsq
    );

    *outSlope = slope;

    double absSlope = slope < 0 ? slope * -1 : slope;
    return !isnan(slope) && absSlope > slopeAbsThreshold;
}

void drawSlopeInfo(IplImage * image, double slope) {

    int startPointX = 80;
    int startPointY = 120;
    int endPointX = 200;
    int endPointY = (int) (startPointY + (slope * (endPointX - startPointX)));

    if (!isnan(slope)) {
      cvLine(
          image, 
          cvPoint(startPointY, startPointX), 
          cvPoint(endPointY, endPointX), 
          CV_RGB(255,255,0), 2, CV_AA
      );
    }

    CvFont font = cvFont(0.8, 1);
 
    char * slopeMessage = (char *) malloc(sizeof(char) * 100);
    sprintf(slopeMessage, "slope: %lf", slope);
    cvPutText(image, slopeMessage, cvPoint(0,100), &font, CV_RGB(255, 0, 0));
    free(slopeMessage);
}

void drawBoundary(IplImage * image, int topLineY, int bottomLineY, 
                  int leftLineX, int rightLineX) {

    cvLine(
        image, cvPoint(0,topLineY), cvPoint(319, topLineY), 
        CV_RGB(255, 0, 0), 1, CV_AA, 0
    );

    cvLine(
        image, cvPoint(0,bottomLineY), cvPoint(319, bottomLineY), 
        CV_RGB(255, 0, 0), 1, CV_AA, 0
    );

    cvLine(
        image, cvPoint(leftLineX,0), cvPoint(leftLineX, 239), 
        CV_RGB(255, 0, 0), 1, CV_AA, 0
    );

    cvLine(
        image, cvPoint(rightLineX,0), cvPoint(rightLineX, 239), 
        CV_RGB(255, 0, 0), 1, CV_AA, 0
    );

    char * message = (char *) malloc(sizeof(char) * 100);
    CvFont font = cvFont(1, 0.5);
    sprintf(message, "topY: %d", topLineY);
    cvPutText(image, message, cvPoint(0, 200), &font, CV_RGB(255, 0, 0));
    sprintf(message, "botY: %d", bottomLineY);
    cvPutText(image, message, cvPoint(0, 220), &font, CV_RGB(255, 0, 0));
    free(message);
}

void drawDetectionResult(IplImage * image, int hasProturding, int hasRotate) {
    char * message = (char *) malloc(sizeof(char) * 100);
    CvFont font = cvFont(1.1, 0.5);
    sprintf(message, "proturding: %s", hasProturding ? "TRUE" : "FALSE");
    cvPutText(image, message, cvPoint(0, 20), &font, CV_RGB(255, 0, 0));
    sprintf(message, "rotate: %s", hasRotate ? "TRUE" : "FALSE");
    cvPutText(image, message, cvPoint(0, 40), &font, CV_RGB(255, 0, 0));
    free(message);
}

void drawFeaturePoints(IplImage * image, CvSeq *imageKeypoints, int keyPointSizeThreshold, 
                       int topLineY, int leftLineX, int rightLineX, int distanceThreshold) {

    for(int i = 0; i < imageKeypoints->total; i++ ) 
    {
        CvSURFPoint* r = (CvSURFPoint*)cvGetSeqElem( imageKeypoints, i );
        CvPoint center;
        int radius;
        center.x = cvRound(r->pt.x);
        center.y = cvRound(r->pt.y);
        radius = cvRound(r->size*1.2/9.*2);

        if (r->size <= keyPointSizeThreshold) {
            if (r->pt.x >= leftLineX + 5 && r->pt.x <= rightLineX - 5) {
                if (r->pt.y <= (topLineY + distanceThreshold)) {
                    cvCircle(image, center, radius, CV_RGB(0, 255, 0), 1, 8, 0);
                } else {
                    cvCircle(image, center, radius, CV_RGB(255, 0, 0), 1, 8, 0);
                }
            } else {
                cvCircle(image, center, radius, CV_RGB(0, 0, 255), 1, 8, 0);
            }
        } else {
            //cvCircle(image, center, radius, CV_RGB(255, 0, 255), 1, 8, 0);
        }
    }
}

void detectErrorInBounds(
    IplImage * image, CvMat * grayImage, CvMat * edgeImage, 
    int keyFrameFeature,
    int featureThreshold, int counter, 
    int topLineY, int bottomLineY, int leftLineX, int rightLineX) {

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq *imageKeypoints = 0;
    CvSeq *imageDescriptors = 0;
    CvSURFParams params = cvSURFParams(featureThreshold, 1);
    cvExtractSURF(grayImage, 0, &imageKeypoints, &imageDescriptors, storage, params);
    
    double slope = 0;

    int hasProturding = isProturding(
        image, topLineY, bottomLineY, leftLineX, rightLineX, 
        imageKeypoints, 30, 0.15, 2
    );

    int hasRotate = isRotate(
        image, topLineY, leftLineX, rightLineX, 
        imageKeypoints, 30, 2, 0.1, &slope
    );

    if (topLineY != bottomLineY) {

        drawSlopeInfo(image, slope);
        drawBoundary(image, topLineY, bottomLineY, leftLineX, rightLineX);
        drawDetectionResult(image, hasProturding, hasRotate);
        drawFeaturePoints(image, imageKeypoints, 30, topLineY, leftLineX, rightLineX, 2); 

        if (hasRotate) {
          char * filename = (char *) malloc(sizeof(char) * 200);
          sprintf(filename, "miss/%08d.png", counter);
          cvSaveImage(filename, image, 0);
          free(filename);
        }
    }
}


void detectError(IplImage * image, CvMat * grayImage, CvMat * edgeImage, 
               int keyFrameFeature,
               int featureThreshold, int counter) {

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq *imageKeypoints = 0;
    CvSeq *imageDescriptors = 0;
    CvSURFParams params = cvSURFParams(featureThreshold, 1);
    cvExtractSURF(grayImage, 0, &imageKeypoints, &imageDescriptors, storage, params);
    
    int leftLineX = -1;
    int rightLineX = -1;
    int topLineY = -1;
    int bottomLineY = -1;
    double slope = 0;

    findTopBottom(edgeImage, &topLineY, &bottomLineY);
    findLeftRight(imageKeypoints, &leftLineX, &rightLineX);

    int hasProturding = isProturding(
        image, topLineY, bottomLineY, leftLineX, rightLineX, 
        imageKeypoints, 30, 0.15, 2
    );

    int hasRotate = isRotate(
        image, topLineY, leftLineX, rightLineX, 
        imageKeypoints, 30, 2, 0.1, &slope
    );

    if (keyFrameFeature >= 45 && topLineY != bottomLineY) {

        drawSlopeInfo(image, slope);
        drawBoundary(image, topLineY, bottomLineY, leftLineX, rightLineX);
        drawDetectionResult(image, hasProturding, hasRotate);
        drawFeaturePoints(image, imageKeypoints, 30, topLineY, leftLineX, rightLineX, 2); 

        if (hasRotate) {
          char * filename = (char *) malloc(sizeof(char) * 200);
          sprintf(filename, "miss/%08d.png", counter);
          cvSaveImage(filename, image, 0);
          free(filename);
        }
    }
}

int getFeatureCount(CvMat * grayImage, int featureThreshold) {
    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq *imageKeypoints = 0;
    CvSeq *imageDescriptors = 0;
    CvSURFParams params = cvSURFParams(featureThreshold, 1);
    cvExtractSURF( grayImage, 0, &imageKeypoints, &imageDescriptors, storage, params );
    return imageKeypoints->total;
}

