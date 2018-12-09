#ifndef SESSIE_5_2_STRAWBERRY_H
#define SESSIE_5_2_STRAWBERRY_H

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define KERNEL_SIZE 5
#define CIRCLE_THICKNESS -1
#define CIRCLE_RADIUS 5

void runner(int trackbarPos, void *data);
void mouse(int event, int x, int y, int flags, void* userdata);
void descriptor(Mat img, vector<Point2d> foregroundPoints, vector<Point2d> backgroundPoints, Mat& trainingData, Mat& labels);

#endif //SESSIE_5_2_STRAWBERRY_H
