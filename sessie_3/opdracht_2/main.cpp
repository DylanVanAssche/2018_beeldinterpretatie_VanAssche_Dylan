#include <iostream>
#include <opencv2/opencv.hpp>
#define DEFAULT_MATCHING_METHOD 3 // TM CCORR default
#define DEFAULT_THRESHOLD_VALUE 80 // 80 % default thresholding

using namespace std;
using namespace cv;

void matchingFuction(int trackbarPos, void *data);

Mat inputImg;
Mat templateImg;

int matchMethod = DEFAULT_MATCHING_METHOD;
int thresholdValue = DEFAULT_THRESHOLD_VALUE;

int main(int argc, const char **argv)
{
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
    inputImg = imread(inputPath, IMREAD_COLOR);
    templateImg = imread(templatePath, IMREAD_COLOR);

    if(inputImg.empty() || templateImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }


    // Show loaded images
    namedWindow("Input image", WINDOW_AUTOSIZE);
    namedWindow("Template image", WINDOW_AUTOSIZE);
    imshow("Input image", inputImg);
    imshow("Template image", templateImg);

    // Create a window with trackbars to show different types of template matching methods and threshold.
    namedWindow("Template matching", WINDOW_AUTOSIZE);
    createTrackbar("Threshold %", "Template matching", &thresholdValue, 100, matchingFuction); // 0 - 100
    createTrackbar("0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED",
"Template matching", &matchMethod, 5, matchingFuction); // 0 - 5

    // Launch window
    matchingFuction(0, NULL);

    // Wait for key, windows stay open until a key has been pressed
    waitKey(0);

    return 0;
}

void matchingFuction(int trackbarPos, void *data)
{
    Mat result;
    double minVal;
    double maxVal;

    // Template matching and NCC
    matchTemplate(inputImg, templateImg, result, matchMethod); // Find matches using templates
    normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat()); // Normalize matches between 0 and 1

    // These template matching methods find the best match at the local minimum, convert the result
    if (matchMethod == TM_SQDIFF || matchMethod == TM_SQDIFF_NORMED)
    {
        result = 1 - result;
    }

    // Find maximum or minimum match
    minMaxLoc(result, &minVal, &maxVal);

    // Threshold image using inRange and convert mask to binary to find multiple matches
    Mat mask = Mat::zeros(Size(inputImg.cols, inputImg.rows), CV_32FC1);
    inRange(result, maxVal * ((double)thresholdValue/100.0), maxVal, mask);
    mask.convertTo(mask, CV_8UC1);

    // Keep the original input img available
    Mat copyImg(inputImg.clone());

    // Use contours to find the contours of all the matches
    vector<vector<Point> > contours;
    findContours(mask, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for(int i = 0; i < contours.size(); i++)
    {
        // Find bounding rectangle
        vector<Point> hull;
        convexHull(contours[i], hull);
        Rect rect = boundingRect(hull);

        // Find local result for this rectangle
        Point loc;
        minMaxLoc(result(rect), NULL, NULL, NULL, &loc);

        // Draw rectangle on image
        Point p(loc.x + rect.x, loc.y + rect.y);
        rectangle(copyImg, p, Point(p.x + templateImg.cols, p.y + templateImg.rows), Scalar(0, 0, 255));
    }

    // Show everything to the user
    imshow("Mask", mask);
    imshow("Template matching", copyImg);
    imshow("Template matching NCC", result);
}
