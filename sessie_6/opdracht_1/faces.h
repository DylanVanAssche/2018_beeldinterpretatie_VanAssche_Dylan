#ifndef SESSIE_6_1_FACES_H
#define SESSIE_6_1_FACES_H

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

void runDetection(Mat frame, CascadeClassifier violaJones);
#define SCALE_FACTOR 1.05
#define MIN_NEIGHBORS 3
#define FONT_SIZE 0.8

#endif //SESSIE_6_1_FACES_H
