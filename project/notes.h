#ifndef NOTES_H
#define NOTES_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>

#define THRESHOLD_MAX 255
#define THRESHOLD_BLOCK_SIZE 25
#define THRESHOLD_C -2
#define HORIZONTAL_DIVIDER 15
#define VERTICAL_DIVIDER 75
#define HORIZONTAL_HEIGHT 1
#define VERTICAL_WIDTH 1

using namespace std;
using namespace cv;

struct NoteSheet {
    Mat notes;
    Mat staffLines;
};

NoteSheet splitStaffLinesAndNotes(Mat input);
void detectNotes(Mat noteImg, vector<Mat> symbols);
Mat getHistogram(Mat input);

#endif //NOTES_H
