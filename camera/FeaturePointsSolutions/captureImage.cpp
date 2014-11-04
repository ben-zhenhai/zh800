#include "detection.hpp"

void drawFeatureCount(IplImage * image, int featureCount) {
    char * message = (char *) malloc(sizeof(char) * 20);
    sprintf(message, "f:%d", featureCount);
    CvFont font = cvFont(1.1, 0.5);
    cvPutText(image, message, cvPoint(0, 80), &font, CV_RGB(255, 0, 0));
    free(message);
}

int main(int argc, char ** argv) {

    CvCapture* capture = cvCaptureFromCAM(1);

    int counter = 0;

    while (1) {
        IplImage* frame = cvQueryFrame(capture);

        if (!frame) {
          break;
        }

        IplImage * resized = resizeTo(frame, 320, 240);
        CvMat * grayImage = toGrayImage(resized);
        CvMat * edgeImage = toEdgeImage(grayImage);

        int featureCount = getFeatureCount(grayImage, 500);

        drawFeatureCount(resized, featureCount);
        detectErrorInBounds(resized, grayImage, edgeImage, 100, 800, 0, 70, 130, 120, 170);

        int key = cvWaitKey(30);

        cvShowImage("Image", resized);


        if (key == 1048689) {
            printf("Saved.\n");
            char * filename = (char *) malloc(sizeof(char) * 200);
            sprintf(filename, "shot/%08d.png", counter);
            cvSaveImage(filename, resized, 0);
            free(filename);
            counter++;
        }
    }
}
