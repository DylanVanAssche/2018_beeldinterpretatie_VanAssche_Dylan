#ifndef SESSIE_6_EXTRA_PEOPLE_H
#define SESSIE_6_EXTRA_PEOPLE_H

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define FONT_SIZE 0.8
#define MIN_WEIGHT 1.0
#define SCALE 1.5
#define RADIUS 5
#define THICKNESS -1 // fill complete

vector<Point> runDetection(Mat frame, HOGDescriptor hog);
Mat drawTracking(Mat input, vector<Point> tracking);

#endif //SESSIE_6_EXTRA_PEOPLE_H
