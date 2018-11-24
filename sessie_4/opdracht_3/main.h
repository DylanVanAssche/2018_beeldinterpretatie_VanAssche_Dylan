#ifndef SESSIE_4_3_MAIN_H
#define SESSIE_4_3_MAIN_H

#include <iostream>
#include <opencv2/opencv.hpp>

#define DISTANCE_MULTIPLIER 0.7
#define RNG_INIT 12345

using namespace std;
using namespace cv;

Mat generateMultiHorizontalImage(vector<Mat> images);
Mat drawKeypointsForDetector(Mat input, Ptr<Feature2D> detector);
Mat drawMatchesForDetector(Mat input, Mat object, Ptr<Feature2D> detector);


#endif //SESSIE_4_3_MAIN_H
