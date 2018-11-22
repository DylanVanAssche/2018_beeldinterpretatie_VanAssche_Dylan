#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ object o       | | Loads a image of the object you want to track <REQUIRED> }"
                             "{ input i        | | Loads an input image to analyze<REQUIRED> }"
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
    string object(parser.get<string>("object"));
    string input(parser.get<string>("input"));
    if(object.empty() || input.empty())
    {
        cerr << "Please supply your images using command line arguments: --object=fitness_object.png and --input=fitness_image.png" << endl;
        return -1;
    }

    // Try to load images
    Mat objectImg;
    Mat inputImg; // BGR
    objectImg = imread(object, IMREAD_GRAYSCALE);
    inputImg = imread(input, IMREAD_GRAYSCALE);

    if(object.empty() || input.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Object image", WINDOW_AUTOSIZE);
    namedWindow("Input image", WINDOW_AUTOSIZE);
    imshow("Object image", objectImg);
    imshow("Input image", inputImg);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}