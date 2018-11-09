#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

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
    const int low_H = 5; // Set upper and lower boundaries
    const int low_S = 255;
    const int low_V = 255;
    const int high_H = 165;
    const int high_S = 115;
    const int high_V = 115;
    // Convert from BGR to HSV colorspace
    cvtColor(signImg, convertedToHSVImg, COLOR_BGR2HSV);
    inRange(convertedToHSVImg, Scalar(0, 115, 145), Scalar(low_H, low_S, low_V), hsvSegmentedImgUpper); // thresholding on 3 channels for 170 degrees to 180 degrees (red in HSV), saturation and intensity at least 90 or higher
    inRange(convertedToHSVImg, Scalar(high_H, high_S, high_V), Scalar(180, 255, 255), hsvSegmentedImgLower); // thresholding on 3 channels for 0 to 10 degrees (red in HSV), saturation and intensity at least 90 or higher

    // Merge 2 results
    addWeighted(hsvSegmentedImgUpper, 1.0, hsvSegmentedImgLower, 1.0, 0.0, hsvSegmentedImgMerged); // Same weights

    /*
     * HSV color space
     * Advantages: Easy to use, open GIMP, pick the color you want to threshold, copy the BGR values and add them as boundaries
     * Disadvantages: Shadows and other colors which are close to the one you want to filter can change the result significantly! Change the light intensity (sign.jpg and sign2.jpg: sign.jpg works but sign2.jpg is almost completely black).
     *
     * HSV is a better solution for this since you can select only the Hue of the color, in BGR is red a combination of R and a bit of B and G.
     */

    namedWindow("Sign segmented using HSV", WINDOW_AUTOSIZE);
    imshow("Sign segmented using HSV", hsvSegmentedImgMerged);
    waitKey(0); // Wait for key input to continue

    return 0;
}