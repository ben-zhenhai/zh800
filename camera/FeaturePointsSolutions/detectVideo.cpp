#include "detection.hpp"

void drawFeatureCount(IplImage * image, int featureCount) {
    char * message = (char *) malloc(sizeof(char) * 20);
    sprintf(message, "f:%d", featureCount);
    CvFont font = cvFont(1.1, 0.5);
    cvPutText(image, message, cvPoint(0, 80), &font, CV_RGB(255, 0, 0));
    free(message);
}

int main(int argc, char ** argv) {

    if (argc != 2) {
      printf("usage: %s dirname\n", argv[0]);
      return 0;
    }

    int filenameLength = sizeof(char) * (strlen(argv[1]) + 11);
    char * filename = (char *) malloc(filenameLength);
    memset(filename, 0, filenameLength);
    strcat(filename, argv[1]);
    strcat(filename, "/%08d.jpg");

    printf("%s\n", filename);

    CvCapture* capture = cvCaptureFromFile(filename);

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
        detectError(resized, grayImage, edgeImage, featureCount, 800, counter);
        cvShowImage("Image", resized);

        char * filename = (char *) malloc(sizeof(char) * 200);
        sprintf(filename, "output/%08d.png", counter);
        cvSaveImage(filename, resized, 0);
        free(filename);

        counter++;
        cvWaitKey(30);
    }
}
