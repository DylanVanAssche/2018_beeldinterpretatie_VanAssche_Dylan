#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void runner(int pos, void *userdata);

Mat inputImg;
Mat templateImg;

int matchMethod = 5;
int thresholdValue = 50;
int stepAngleValue = 19;
int maxAngleValue = 360;

vector<Rect> matcher(Mat& src);
void showDetections(Mat image, vector<Rect> detections, Mat rotationMatrix);

int main(int argc, const char **argv)
{
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ input i        | | Loads a color image as target for Template matching rotation invariant <REQUIRED> }"
                             "{ template t     | | Loads a color image as template for Template matching rotation invariant <REQUIRED> }"
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

    // Create a window with trackbars to show different types of Template matching rotation invariant methods and threshold.
    namedWindow("Template matching rotation invariant", WINDOW_AUTOSIZE);
    createTrackbar("Threshold %", "Template matching rotation invariant", &thresholdValue, 100, runner); // 0 - 100
    createTrackbar("Max angle in degrees", "Template matching rotation invariant", &maxAngleValue, 360, runner); //  0 - 360 degrees
    createTrackbar("Step angle in degrees", "Template matching rotation invariant", &stepAngleValue, 60, runner); // 0 - 60 degrees
    createTrackbar("0: SQDIFF \n 1: SQDIFF NORMED \n 2: TM CCORR \n 3: TM CCORR NORMED \n 4: TM COEFF \n 5: TM COEFF NORMED",
                   "Template matching rotation invariant", &matchMethod, 5, runner); // 0 - 5

    runner(0, NULL);

    // Sleep & do event loop.
    waitKey(0);

    return 0;
}

vector<Rect> matcher(Mat& src)
{
    Mat result;
    matchTemplate(src, templateImg, result, matchMethod);

    if (matchMethod == TM_SQDIFF || matchMethod == TM_SQDIFF_NORMED)
    {
        result = 1 - result;
    }

    Mat mask = Mat::zeros(Size(src.cols, src.rows), CV_8UC1);
    inRange(result, ((double)thresholdValue/100.0), 1, mask);

    vector<Rect> detections;
    vector<vector<Point> > contours;
    findContours(mask, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (int i = 0; i < contours.size(); ++i)
    {
        // Improve contouring and find bounding box
        vector<Point> hull;
        convexHull(contours[i], hull);
        Rect rect = boundingRect(hull);

        // Find local maximum
        Point loc;
        minMaxLoc(result(rect), NULL, NULL, NULL, &loc);
        Point c(loc.x + rect.x, loc.y + rect.y);

        Rect detectedRect(c, c + Point(templateImg.cols, templateImg.rows));
        detections.push_back(detectedRect);
    }

    return detections;
}

// Visualize detections on the input frame and show in the given window
// https://github.com/OpenCVBlueprints/OpenCVBlueprints/blob/master/chapter_5/source_code/rotation_invariant_detection/rotation_invariant_object_detection.cpp#L107-L152
void showDetections(Mat image, vector<Rect> detections, Mat rotationMatrix)
{
    RNG rng(123456);
    vector<Rect> temp = detections;
    Scalar color(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
    for( int j = 0; j < (int)temp.size(); j++ ) {
        // Use a rectangle representation on the frame but warp back the coordinates
        // Retrieve the 4 corners detected in the rotation image
        Point p1(temp[j].x, temp[j].y); // Top left
        Point p2((temp[j].x + temp[j].width), temp[j].y); // Top right
        Point p3((temp[j].x + temp[j].width), (temp[j].y + temp[j].height)); // Down right
        Point p4(temp[j].x, (temp[j].y + temp[j].height)); // Down left

        // Add the 4 points to a matrix structure
        Mat coordinates = (Mat_<double>(3, 4) << p1.x, p2.x, p3.x, p4.x, \
                                                    p1.y, p2.y, p3.y, p4.y, \
                                                    1, 1, 1, 1);

        // Apply a new inverse tranformation matrix
        Mat result = rotationMatrix * coordinates;

        // Retrieve the new coordinates from the tranformed matrix
        Point p1_back, p2_back, p3_back, p4_back;
        p1_back.x = (int) result.at<double>(0, 0);
        p1_back.y = (int) result.at<double>(1, 0);

        p2_back.x = (int) result.at<double>(0, 1);
        p2_back.y = (int) result.at<double>(1, 1);

        p3_back.x = (int) result.at<double>(0, 2);
        p3_back.y = (int) result.at<double>(1, 2);

        p4_back.x = (int) result.at<double>(0, 3);
        p4_back.y = (int) result.at<double>(1, 3);

        // Draw a rotated rectangle by lines, using the reverse warped points
        line(image, p1_back, p2_back, color, 2);
        line(image, p2_back, p3_back, color, 2);
        line(image, p3_back, p4_back, color, 2);
        line(image, p4_back, p1_back, color, 2);
    }
}

void runner(int trackbarPos, void *data)
{
    Mat outputImg(inputImg.clone());
    Point center;
    center.x = inputImg.cols / 2;
    center.y = inputImg.rows / 2;

    // Do detections for each rotation
    for (int i = 0; i < stepAngleValue; i++)
    {
        // https://github.com/OpenCVBlueprints/OpenCVBlueprints/blob/master/chapter_5/source_code/rotation_invariant_detection/rotation_invariant_object_detection.cpp#L36-L42
        double angle = i*((double)maxAngleValue/(double)stepAngleValue);
        Mat rotatedImg;
        warpAffine(inputImg, rotatedImg, getRotationMatrix2D(center, angle, 1.0), inputImg.size());
        vector<Rect> detections = matcher(rotatedImg);
        showDetections(outputImg, detections, getRotationMatrix2D(center, -angle, 1.0));
    }

    imshow("Template matching rotation invariant", outputImg);
}
