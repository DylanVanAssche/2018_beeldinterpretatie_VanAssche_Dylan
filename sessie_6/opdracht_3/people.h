#ifndef SESSIE_6_3_PEOPLE_H
#define SESSIE_6_3_PEOPLE_H

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define FONT_SIZE 0.8
#define MIN_WEIGHT 1.0
#define SCALE 1.5

void runDetection(Mat frame, HOGDescriptor hog);

#endif //SESSIE_6_3_PEOPLE_H
