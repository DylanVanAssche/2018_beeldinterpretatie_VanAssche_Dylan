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
    #include <time.h>
    #include <string.h>
    #include <errno.h>
    #include "wavfile.h"
}

#define THRESHOLD_MAX 255
#define THRESHOLD_BLOCK_SIZE 25
#define THRESHOLD_C -2
#define ERODE_DILATE_ITER 5
#define HORIZONTAL_DIVIDER 15
#define VERTICAL_DIVIDER 75
#define HORIZONTAL_HEIGHT 1
#define VERTICAL_WIDTH 1

// Sound
#define SOUND_FILE "sound.wav"
#define NUM_SAMPLES (WAVFILE_SAMPLES_PER_SECOND*2)
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

NoteSheet splitStaffLinesAndNotes(Mat input);
void detectNotes(Mat noteImg, vector<Mat> symbols);
Mat getHistogram(Mat input);
Mat clean(Mat input);

#endif //NOTES_H
