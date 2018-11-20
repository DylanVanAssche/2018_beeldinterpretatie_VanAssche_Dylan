#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ input i        | | Loads a color image as target for template matching <REQUIRED> }"
                             "{ template t     | | Loads a color image as template for template matching <REQUIRED> }"
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
    string inputPath(parser.get<string>("input"));
    string templatePath(parser.get<string>("template"));
    if(inputPath.empty() || templatePath.empty())
    {
        cerr << "Please supply your images using command line arguments: --sign=sign.png" << endl;
        return -1;
    }

    // Try to load images
    Mat inputImg;
    Mat templateImg;
    inputImg = imread(inputPath, IMREAD_COLOR);
    templateImg = imread(templatePath, IMREAD_COLOR);
    cvtColor(inputImg, inputImg, COLOR_BGR2GRAY);
    cvtColor(templateImg, templateImg, COLOR_BGR2GRAY);

    if(inputImg.empty() || templateImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Input image", WINDOW_AUTOSIZE);
    imshow("Input image", inputImg);
    namedWindow("Template image", WINDOW_AUTOSIZE);
    imshow("Template image", templateImg);
    namedWindow("Result image", WINDOW_AUTOSIZE);
    Mat copyImg = inputImg.clone(); // Keep original image
    Mat result;

    int cols = copyImg.cols - templateImg.cols + 1;
    int rows = copyImg.rows - templateImg.rows + 1;
    result.create(rows, cols, CV_32FC1);

    matchTemplate(copyImg, templateImg, result, TM_SQDIFF); // make this configurable
    normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat()); // NCC convolution: borders don't have any information [0, 1]
    result = 1 - result; // min to max converting since best match TM_SQDIFF is minimum
    double minVal;
    double maxVal;
    Point minLoc;
    Point maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
    rectangle(copyImg, maxLoc, Point(maxLoc.x + templateImg.cols, maxLoc.y + templateImg.rows), Scalar::all(0));

    Mat scaledResult;
    resize(result, result, Size(copyImg.cols, copyImg.rows));
    cout << "result" << result.size << endl;
    cout << "img" << copyImg.size << endl;
    imshow("Result image", result);
    imshow("Result image", copyImg);
    waitKey(0);

    return 0;
}