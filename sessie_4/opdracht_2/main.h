#ifndef SESSIE_4_2_MAIN_H
#define SESSIE_4_2_MAIN_H

#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

Mat generateMultiHorizontalImage(vector<Mat> images);
Mat drawKeypointsForDetector(Mat input, Ptr<Feature2D> detector);
Mat drawMatchesForDetector(Mat input, Mat object, Ptr<Feature2D> detector);

#endif //SESSIE_4_2_MAIN_H
