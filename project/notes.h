#ifndef NOTES_H
#define NOTES_H

#include <iostream>
#include <opencv2/opencv.hpp>

// Sound lib
extern "C" {
    #include <stdio.h>
    #include <math.h>
    #include <stdlib.h>
    #include "lib/wavfile.h"
}

#define THRESHOLD_MAX 255
#define THRESHOLD_BLOCK_SIZE 25
#define THRESHOLD_C -2
#define TEMPLATE_MATCH_PERCENTAGE 99.0
#define ERODE_DILATE_ITER 5
#define HORIZONTAL_DIVIDER 30
#define VERTICAL_DIVIDER 30
#define HORIZONTAL_HEIGHT 1
#define VERTICAL_WIDTH 1
#define REDUCE_DIMENSION 1
#define NUMBER_OF_STAFF_LINES 5

// Drawing
#define CIRCLE_RADIUS 3
#define CIRCLE_THICKNESS -1
#define RECTANGLE_THICKNESS -1
#define CONTOURS_THICKNESS 2
#define CONTOURS_MAX_LEVEL 0

// Sound
#define NOTE_LENGTH (2 * WAVFILE_SAMPLES_PER_SECOND)
#define NOTE_LENGTH_16 NOTE_LENGTH/16 // 1/16 note
#define NOTE_LENGTH_4 NOTE_LENGTH/4 // 1/4 note
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

typedef struct NoteSheet {
    Mat notes;
    Mat staffLines;
} NoteSheet;

typedef struct NoteTemplate {
    Mat templ;
    double length;
} NoteTemplate;

typedef struct ContoursData {
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<Point> orientation;
    vector<Rect> box;
    vector<double> length;
    Mat image;
    NoteTemplate templ;
} ContoursData;

typedef struct StaffLineData {
    int position;
    int value;
} StaffLineData;

typedef struct Note {
    double frequency;
    double length;
    double position;
} Note;

NoteSheet splitStaffLinesAndNotes(Mat input);
void drawHistogram(Mat histogram, int rows, int cols);
ContoursData getContoursData(Mat input, NoteTemplate templ);
void drawContoursWithOrientation(Mat input, ContoursData data, int rows, int cols);
vector<StaffLineData> getStaffLineDistances(Mat input);
vector<Note> convertDataToNote(Mat input, vector<ContoursData> data, vector<StaffLineData> staffLineDistances, int rows, int cols);
vector<short> generateWaveform(double frequency, double length);
void saveWaveforms(string outputPath, vector< vector<short> > waveforms);

#endif //NOTES_H
