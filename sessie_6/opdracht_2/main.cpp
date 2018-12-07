#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void runDetection(Mat frame, CascadeClassifier violaJones);
#define SCALE_FACTOR 1.05
#define MIN_NEIGHBORS 3
#define FONT_SIZE 0.8

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ video v        | | Loads a video file <REQUIRED> }"
                             "{ lbp         | | Loads a LBP cascade file <REQUIRED> }"
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
    string lbp(parser.get<string>("lbp"));

    // Try to load video
    VideoCapture cap(video);
    if(!cap.isOpened())
    {
        cerr << "Please supply your videos using command line arguments: --video=faces.mp4" << endl;
        return -1;
    }

    // Try to open the HAAR file
    CascadeClassifier violaJones;
    if(!violaJones.load(lbp)) {
        cerr << "Can't load LBP cascade file, supply it with the following argument: --lbp=lbp.xml" << endl;
    }


    namedWindow("video", CV_WINDOW_AUTOSIZE);
    while(1) {
        Mat frame;
        cap >> frame;

        // No frames anymore
        if(frame.empty()) {
            cout << "No frames left, exiting" << endl;
            break;
        }

        // Detect faces using Viola and Jones boosted cascades
        runDetection(frame, violaJones);

        // Show frame
        imshow("video", frame);

        // Exit op ESC
        if(waitKey(10) == 27) {
            break;
        }
    }

    // Wait until the user decides to exit the program.
    return 0;
}

// Run classifier
void runDetection(Mat frame, CascadeClassifier violaJones) {
    Mat gray = frame.clone(); // keep original
    cvtColor(gray, gray, CV_BGR2GRAY); // Viola and Jones only requires intensity gray images
    vector<Rect> facesBoundingBox;
    vector<int> scores;
    equalizeHist(gray, gray); // Improve detection by equalizing intensity

    // Run Viola and Jones detections (frontal detector, perspective and rotated faces aren't detected well -> rotating detector could solve this)
    violaJones.detectMultiScale(gray, facesBoundingBox, scores, SCALE_FACTOR, MIN_NEIGHBORS); // Run Viola and Jones in multi scales using sliding window

    // Draw detections
    for(int i=0; i < facesBoundingBox.size(); ++i) {
        Point facesCenter = Point(facesBoundingBox.at(i).x + facesBoundingBox.at(i).width/2, facesBoundingBox.at(i).y + facesBoundingBox.at(i).height/2);
        circle(gray, facesCenter, 5, Scalar(255, 255, 255));
        Rect box = Rect(Point(facesBoundingBox.at(i).x , facesBoundingBox.at(i).y), Point(facesBoundingBox.at(i).x + facesBoundingBox.at(i).height , facesBoundingBox.at(i).y + facesBoundingBox.at(i).width));
        rectangle(gray, box, Scalar(255, 255, 255));
        stringstream convert;
        convert << scores.at(i);
        string text = convert.str();
        putText(gray, text, Point(facesBoundingBox.at(i).x , facesBoundingBox.at(i).y), FONT_HERSHEY_COMPLEX, FONT_SIZE, Scalar(255, 255, 255));
    }

    imshow("Viola and Jones", gray);
}
