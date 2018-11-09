#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
const int max_value_H = 360/2;
const int max_value = 255;
const String window_trackbar_name = "Draw contours with trackbar";
int low_H = 0, low_S = 0, low_V = 0;
int high_H = max_value_H, high_S = max_value, high_V = max_value;

static void on_low_H_thresh_trackbar(int, void *)
{
    low_H = min(high_H-1, low_H);
    setTrackbarPos("Low H", window_trackbar_name, low_H);
}
static void on_high_H_thresh_trackbar(int, void *)
{
    high_H = max(high_H, low_H+1);
    setTrackbarPos("High H", window_trackbar_name, high_H);
}
static void on_low_S_thresh_trackbar(int, void *)
{
    low_S = min(high_S-1, low_S);
    setTrackbarPos("Low S", window_trackbar_name, low_S);
}
static void on_high_S_thresh_trackbar(int, void *)
{
    high_S = max(high_S, low_S+1);
    setTrackbarPos("High S", window_trackbar_name, high_S);
}
static void on_low_V_thresh_trackbar(int, void *)
{
    low_V = min(high_V-1, low_V);
    setTrackbarPos("Low V", window_trackbar_name, low_V);
}
static void on_high_V_thresh_trackbar(int, void *)
{
    high_V = max(high_V, low_V+1);
    setTrackbarPos("High V", window_trackbar_name, high_V);
}

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ sign s        | | Loads a color image with a traffic sign <REQUIRED> }"
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
    string sign(parser.get<string>("sign"));
    if(sign.empty())
    {
        cerr << "Please supply your images using command line arguments: --sign=sign.png" << endl;
        return -1;
    }

    // Try to load images
    Mat signImg;
    signImg = imread(sign, IMREAD_COLOR);

    if(signImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Sign image", WINDOW_AUTOSIZE);
    imshow("Sign image", signImg);

    Mat rgbSegmentedImg;
    const int low_B = 0; // Set upper and lower boundaries
    const int low_G = 0;
    const int low_R = 200;
    const int high_B = 15;
    const int high_G = 15;
    const int high_R = 255;
    inRange(signImg, Scalar(low_B, low_G, low_R), Scalar(high_B, high_G, high_R), rgbSegmentedImg); // thresholding on 3 channels

    /*
     * BGR color space
     * Advantages: Easy to use, open GIMP, pick the color you want to threshold, copy the BGR values and add them as boundaries.
     * Disadvantages: Shadows and other colors which are close to the one you want to filter can change the result significantly! Change the light intensity (sign.jpg and sign2.jpg: sign.jpg works but sign2.jpg is almost completely black).
     *
     * HSV is a better solution for this since you can select only the Hue of the color, in BGR is red a combination of R and a bit of B and G.
     */

    namedWindow("Sign segmented using BGR", WINDOW_AUTOSIZE);
    imshow("Sign segmented using BGR", rgbSegmentedImg);
    waitKey(0); // Wait for key input to continue

    Mat hsvSegmentedImgUpper, hsvSegmentedImgLower, hsvSegmentedImgMerged;
    Mat convertedToHSVImg;
    const int low_H1 = 0; // Set upper and lower boundaries
    const int low_S1 = 115;
    const int low_V1 = 145;
    const int low_H2 = 5; // Set upper and lower boundaries
    const int low_S2 = 255;
    const int low_V2 = 255;
    const int high_H1 = 165;
    const int high_S1 = 115;
    const int high_V1 = 115;
    const int high_H2 = 180;
    const int high_S2 = 255;
    const int high_V2 = 255;
    // Convert from BGR to HSV colorspace
    cvtColor(signImg, convertedToHSVImg, COLOR_BGR2HSV);
    inRange(convertedToHSVImg, Scalar(low_H1, low_S1, low_V1), Scalar(low_H2, low_S2, low_V2), hsvSegmentedImgUpper); // thresholding on 3 channels for 170 degrees to 180 degrees (red in HSV), saturation and intensity at least 90 or higher
    inRange(convertedToHSVImg, Scalar(high_H1, high_S1, high_V1), Scalar(high_H2, high_S2, high_V2), hsvSegmentedImgLower); // thresholding on 3 channels for 0 to 10 degrees (red in HSV), saturation and intensity at least 90 or higher

    // Merge 2 results
    addWeighted(hsvSegmentedImgUpper, 1.0, hsvSegmentedImgLower, 1.0, 0.0, hsvSegmentedImgMerged); // Same weights

    /*
     * HSV color space
     * Advantages: Easy to segment properly a specified color using Hue, use GIMP to figure out the S and V values and convert them to OpenCV using a Python script (see folder sessie_2/gimpHSVtoOpencv.py)
     * Disadvantages: A bit more difficult to think in this different color space, red requires 2 operations since it's laying around the border of the Hue.
     *
     * HSV is a better solution for this since you can select only the Hue of the color, in BGR is red a combination of R and a bit of B and G.
     */

    namedWindow("Sign segmented using HSV", WINDOW_AUTOSIZE);
    imshow("Sign segmented using HSV", hsvSegmentedImgMerged);
    waitKey(0); // Wait for key input to continue

    // Connect blobs to improve segmentation
    dilate(hsvSegmentedImgMerged, hsvSegmentedImgMerged, Mat(), Point(-1, -1), 5); // connect blobs, more times than removing noise
    erode(hsvSegmentedImgMerged, hsvSegmentedImgMerged, Mat(), Point(-1, -1), 5); // fix dilate data loss

    namedWindow("Sign segmented using HSV connected", WINDOW_AUTOSIZE);
    imshow("Sign segmented using HSV connected", hsvSegmentedImgMerged);
    waitKey(0); // Wait for key input to continue

    // Convex hull approach -> contours
    vector< vector<Point> > contours;
    // Find contours of a binary image.
    findContours(hsvSegmentedImgMerged.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); // explain defines
    vector<Point> hull;
    convexHull(contours[0], hull);
    vector<Point> biggest_blob = hull;
    for(size_t i=0; i < contours.size(); i++) {
        if(contourArea(contours[i]) > contourArea(biggest_blob)) {
            convexHull(contours[i], hull); // Improve contouring
            biggest_blob = hull;
        }
    }
    vector < vector<Point> > temp;
    temp.push_back(biggest_blob);

    // input image, contours, contourIdx (-1 = draw all contours), color, thickness (< 0, draw contour interiors), lineType, hierarchy, maxLevel, offset
    drawContours(hsvSegmentedImgMerged, temp, -1, 255, -1);

    Mat contouredImg(signImg.size(), CV_8UC3);
    signImg.copyTo(contouredImg, hsvSegmentedImgMerged);
    namedWindow("Draw contours", WINDOW_AUTOSIZE);
    imshow("Draw contours", contouredImg);
    waitKey(0);

    // Add trackbars to select the right threshold in an easy way
    namedWindow("Draw contours with trackbar", WINDOW_AUTOSIZE);
    createTrackbar("Low H", window_trackbar_name, &low_H, max_value_H, on_low_H_thresh_trackbar);
    createTrackbar("High H", window_trackbar_name, &high_H, max_value_H, on_high_H_thresh_trackbar);
    createTrackbar("Low S", window_trackbar_name, &low_S, max_value, on_low_S_thresh_trackbar);
    createTrackbar("High S", window_trackbar_name, &high_S, max_value, on_high_S_thresh_trackbar);
    createTrackbar("Low V", window_trackbar_name, &low_V, max_value, on_low_V_thresh_trackbar);
    createTrackbar("High V", window_trackbar_name, &high_V, max_value, on_high_V_thresh_trackbar);
    cvtColor(signImg, convertedToHSVImg, COLOR_BGR2HSV);
    Mat hsvSegmentedImg;

    while (true) {
        inRange(convertedToHSVImg, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), hsvSegmentedImg); // thresholding on 3 channels using trackbars

        // Connect blobs to improve segmentation
        dilate(hsvSegmentedImg, hsvSegmentedImg, Mat(), Point(-1, -1), 5); // connect blobs, more times than removing noise
        erode(hsvSegmentedImg, hsvSegmentedImg, Mat(), Point(-1, -1), 5); // fix dilate data loss

        // Convex hull approach -> contours
        vector< vector<Point> > contours;
        // Find contours of a binary image.
        findContours(hsvSegmentedImg.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); // explain defines
        convexHull(contours[0], hull);
        vector<Point> biggest_blob = hull;
        for(size_t i=0; i < contours.size(); i++) {
            if(contourArea(contours[i]) > contourArea(biggest_blob)) {
                convexHull(contours[i], hull); // Improve contouring
                biggest_blob = hull;
            }
        }
        vector < vector<Point> > temp;
        temp.push_back(biggest_blob);

        // input image, contours, contourIdx (-1 = draw all contours), color, thickness (< 0, draw contour interiors), lineType, hierarchy, maxLevel, offset
        drawContours(hsvSegmentedImgMerged, temp, -1, 255, -1); // check docs -1

        Mat contouredImg(signImg.size(), CV_8UC3);
        signImg.copyTo(contouredImg, hsvSegmentedImg);
        // Show the frames
        imshow(window_trackbar_name, contouredImg);
        char key = (char) waitKey(1);
        if (key == 'q' || key == 'Q' || key == 27) // ESC
        {
            break;
        }
    }

    return 0;
}