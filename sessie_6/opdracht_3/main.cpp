#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ video v        | | Loads a video file <REQUIRED> }"
    );

    // Help printing
    if(parser.has("help") || argc <= 1) {
        cerr << "Please use absolute paths when supplying your video." << endl;
        parser.printMessage();
        return 0;
    }

    // Parser fail
    if (!parser.check())
    {
        parser.printErrors();
        return -1;
    }

    // Required arguments supplied?
    string video(parser.get<string>("video"));
    VideoCapture cap(video);
    if(!cap.isOpened())
    {
        cerr << "Please supply your videos using command line arguments: --video=faces.mp4" << endl;
        return -1;
    }

    // Try to load video
    namedWindow("video", CV_WINDOW_AUTOSIZE);
    while(1) {
        Mat frame;
        cap >> frame;
        imshow("video", frame);

        // Exit
        if(waitKey(30) >= 0) {
            break;
        }
    }

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}