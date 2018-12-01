#ifndef SESSIE_5_3_STRAWBERRY_H
#define SESSIE_5_3_STRAWBERRY_H

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define KERNEL_SIZE 5
#define CIRCLE_THICKNESS -1
#define CIRCLE_RADIUS 5
#define KNN_GROUPS 3
#define ML_OPENING_ITER 0
#define ML_CLOSING_ITER 0
#define SVM_ITER 100
#define SVM_EPSILON 1e-6

void runner(int trackbarPos, void *data);
void mouse(int event, int x, int y, int flags, void* userdata);
void descriptor(Mat img, vector<Point2d> foregroundPoints, vector<Point2d> backgroundPoints, Mat& trainingData, Mat& labels);
void KNN(Mat trainingsData, Mat labels);
void NaiveBayes(Mat trainingsData, Mat labels);
void SVM(Mat trainingsData, Mat labels);
void showResult(Ptr<ml::StatModel> classifier);

#endif //SESSIE_5_3_STRAWBERRY_H
