#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define FONT_SIZE 0.8
#define MIN_WEIGHT 1.0
#define SCALE 1.5

void runDetection(Mat frame, HOGDescriptor hog);

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

    // Try to load video
    VideoCapture cap(video);
    if(!cap.isOpened())
    {
        cerr << "Please supply your videos using command line arguments: --video=faces.mp4" << endl;
        return -1;
    }

    // HOG descriptor with SVM for pedestrians
    HOGDescriptor hog;
    vector<Point> trackingPoints;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

    namedWindow("video", CV_WINDOW_AUTOSIZE);
    while(1) {
        Mat frame;
        cap >> frame;

        // No frames anymore
        if(frame.empty()) {
            cout << "No frames left, exiting" << endl;
            break;
        }

        // Pedestrian is too small at the beginning, scale the frame to improve detection
        resize(frame, frame, Size(SCALE*frame.cols, SCALE*frame.rows));

        // Detect faces using Viola and Jones boosted cascades
        runDetection(frame, hog);

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
void runDetection(Mat frame, HOGDescriptor hog) {
    Mat img = frame.clone(); // keep original

    vector<Rect> people;
    vector<double> weights;

    // HOG detection
    hog.detectMultiScale(img, people, weights);

    // Draw detections
    for(int i=0; i < people.size(); ++i) {
        if(weights.at(i) >= MIN_WEIGHT) {
            Point facesCenter = Point(people.at(i).x + people.at(i).width / 2,
                                      people.at(i).y + people.at(i).height / 2);
            circle(img, facesCenter, 5, Scalar(255, 255, 255));
            Rect box = Rect(Point(people.at(i).x, people.at(i).y),
                    Point(people.at(i).x + people.at(i).width, people.at(i).y + people.at(i).height));
            rectangle(img, box, Scalar(255, 255, 255));
            stringstream convert;
            convert << weights.at(i);
            string text = convert.str();
            putText(img, text, Point(people.at(i).x, people.at(i).y), FONT_HERSHEY_COMPLEX, FONT_SIZE,
                    Scalar(255, 255, 255));
        }
    }

    imshow("Pedestrian detector", img);
}

// LBP is 4x sneller omdat deze enkel met integers werkt ipv floats. Haar werkt met grijswaarden, LBP enkel Z/W. Helaas is de accuratie hierdoor slechter
// https://courses.nvidia.com/courses account maken en dan NIKS doen
//websocketstext.com -> YES port 80 op alles