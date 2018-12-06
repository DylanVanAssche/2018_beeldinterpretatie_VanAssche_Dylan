#ifndef NOTES_H
#define NOTES_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>

// Sound lib
extern "C" {
    #include <stdio.h>
    #include <math.h>
    #include <stdlib.h>
    #include "wavfile.h"
}

#define THRESHOLD_MAX 255
#define THRESHOLD_BLOCK_SIZE 25
#define THRESHOLD_C -2
#define ERODE_DILATE_ITER 5
#define HORIZONTAL_DIVIDER 30
#define VERTICAL_DIVIDER 30
#define HORIZONTAL_HEIGHT 1
#define VERTICAL_WIDTH 1
#define RNG_INIT 12345

// Sound
#define NUM_SAMPLES (2 * WAVFILE_SAMPLES_PER_SECOND)
#define VOLUME 32000
#define NOTE_C 261.6
#define NOTE_D 293.7
#define NOTE_E 329.6
#define NOTE_F 349.2
#define NOTE_G 392.0
#define NOTE_A 440.0
#define NOTE_B 493.9

using namespace std;
using namespace cv;

struct NoteSheet {
    Mat notes;
    Mat staffLines;
};

struct ContoursData {
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<Point> orientation;
};

enum {
    ASCENDING,
    DESCENDING
} direction = ASCENDING;

NoteSheet splitStaffLinesAndNotes(Mat input);
Mat getHorizontalHistogram(Mat input);
void drawHistogram(Mat histogram, int rows, int cols);
ContoursData getContours(Mat input);
void drawContoursWithOrientation(ContoursData data, int rows, int cols);
vector<int> getStaffLineDistances(Mat input);
vector<short> generateWaveform(double frequency, double length);
void saveWaveforms(string outputPath, vector< vector<short> > waveforms);

#endif //NOTES_H
