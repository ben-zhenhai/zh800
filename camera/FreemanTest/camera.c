#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>


int main()
{
    int c;
    CvCapture *capture = cvCaptureFromCAM(0);
    IplImage *frame;
    cvNamedWindow("Show Image", 0);
    //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 320);
    //cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 240);

    for (;;)
    {
        frame = cvQueryFrame(capture);
        if (frame != 0)
        {
            cvShowImage("Show Image", frame);
            c = cvWaitKey(10);
            if(c == 27) // ESC key
            {
                break;
            }
        }
    }
    cvReleaseCapture(&capture);
    cvDestroyWindow("Show Image");
}
