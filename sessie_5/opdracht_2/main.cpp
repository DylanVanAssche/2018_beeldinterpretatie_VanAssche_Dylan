#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ strawberry s   | | Loads a strawberry image <REQUIRED> }"
    );

    // Help printing
    if(parser.has("help") || argc <= 1) {
        cerr << "Please use absolute paths when supplying your images." << endl;
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
    string strawberry(parser.get<string>("strawberry"));
    if(strawberry.empty())
    {
        cerr << "Please supply your images using command line arguments: --strawberry=strawberry1.tif" << endl;
        return -1;
    }

    // Try to load images
    Mat strawberryImg;
    strawberryImg = imread(strawberry, IMREAD_COLOR);

    if(strawberryImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Strawberry image", WINDOW_AUTOSIZE);
    imshow("Strawberry image", strawberryImg);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}