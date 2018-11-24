#ifndef SESSIE_4_1_MAIN_H
#define SESSIE_4_1_MAIN_H

#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

Mat generateMultiHorizontalImage(vector<Mat> images);
Mat drawKeypointsForDetector(Mat input, Ptr<Feature2D> detector);

#endif //SESSIE_4_1_MAIN_H
