/*
 * @title Labo beeldinterpretatie 2018: project
 * @author Dylan Van Assche
 *
 * ---> BASIC MUSIC NOTES RECOGNITION <---
 *
 * This Proof-Of-Concept (POC) can extract the music notes from a music sheet and save the sound of them into a
 * WAV audio file.
 *
 * Features:
 *  - Detect non-rotated 1/4 and 1/16 notes using template matching.
 *  - Find the tone height of each note using the staff lines extraction and vertical histograms.
 *  - Merge both into a music tone and save it to a WAV audio file using a WAV library.
 *
 * Usage:
 *  - cmake CMakeLists.txt
 *  - make
 *  - ./project --sheet=musicSheet.png --output=output.wav --quarter-note=quarter-note.png \
 *    --double-eighth-note=double-eighth-note.png
 *
 */
#include "notes.h"

// std::sort helper function
bool sortStaffLinesBiggestValueFirst(const StaffLineData &a, const StaffLineData &b) {
    return a.value > b.value; // biggest first
}

// std::sort helper function
bool sortStaffLinesSmallestPositionFirst(const StaffLineData &a, const StaffLineData &b) {
    return a.position < b.position; // smallest first
}

/*
 * Plots a histogram on an image with size (rows, cols).
 *
 * @param Mat histogram
 * @param int rows
 * @param int cols
 * @author Dylan Van Assche
 */
void drawHistogram(Mat histogram, int rows, int cols) {
    Mat drawing = Mat::zeros(rows, cols, CV_8UC1);
    double maxHistogram = 0;
    int normalize = 0;
    Scalar colorWhite = Scalar::all(255);

    /*
     * Find global maximum to normalize the histogram later.
     * Global minimum is skipped using a NULL pointer as described in the docs.
     * https://docs.opencv.org/3.4.0/d2/de8/group__core__array.html#ga7622c466c628a75d9ed008b42250a73f
     * Mat input, min, max
     * min = NULL, the function will skip this since we don't need it anyway
     */
    minMaxIdx(histogram, NULL, &maxHistogram);

    // Draw calculated horizontal histogram
    int counter = histogram.cols;

    // Vertical histograms are rotated by 90 degrees
    if(histogram.cols < histogram.rows) {
        counter = histogram.rows;
    }

    for(int i=0; i < counter; i++) {
        normalize = drawing.rows * ((histogram.at<int>(0, i))/maxHistogram);
        // Mind the order of X/Y and ROW/COLUMN: https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column !
        // Mat to draw on, Point 1, Point 2, color
        line(drawing, Point(i, normalize), Point(i, 0), colorWhite);
    }

    // Display histogram and wait for key
    cout << "Displaying histogram" << endl;
    namedWindow("Histogram", CV_WINDOW_AUTOSIZE);
    imshow("Histogram", drawing);
    waitKey(0);
}

/*
 * By applying erosion (remove noise) and dilation (connect blobs) using a structure element, we can extract the
 * vertical and horizontal lines of the staff lines.
 *
 * This approach is based on: https://docs.opencv.org/master/dd/dd7/tutorial_morph_lines_detection.html. I tried to use
 * local feature matching techniques like ORB, SURF, ... to locate the staff lines but only the first symbol has 1 - 2
 * local features which is too low. The lines itself don't have any variation which lead to no local features.
 *
 * Another approach would be counting the number of pixels in a horizontal histogram of the binary image. All peaks in
 * the histogram are staff lines. However, the morfologic approach is a bit more robust in this case due the use of a
 * specific kernel (a difference of a couple of pixels are ignored).
 *
 * @param Mat input
 * @returns NoteSheet sheet
 * @author Dylan Van Assche
 */
NoteSheet splitStaffLinesAndNotes(Mat input) {
    Mat binary = input.clone(); // Make sure we don't modify the input
    NoteSheet result;

    /*
     * Remove noise using an opening operation
     * input, output, kernel, anchor point, iterations
     * kernel = structuring element form
     * anchor = anchor of the structuring element, Point(-1, -1) = center
     */
    erode(binary, binary, 0, Point(-1, -1), ERODE_DILATE_ITER);
    dilate(binary, binary, 0, Point(-1, -1), ERODE_DILATE_ITER);

    /*
     * Threshold the gray image to a binary image using adaptive threshold (better resistance against different light
     * conditions). We invert the image before applying the threshold since we want a black background and white notes.
     *
     * ADAPTIVE_THRESH_GAUSSIAN_C seems to improve the extraction a little bit, the alternative ADAPTIVE_THRESH_MEAN_C
     * uses the same weight for all neighbors.
     *
     * THRESHOLD_BLOCK_SIZE set to 15 is a good window size for music notes.
     * THRESHOLD_C set to -2, constant subtracted from the mean or weighted mean.
     *
     * https://docs.opencv.org/3.2.0/d7/d1b/group__imgproc__misc.html#ga72b913f352e4a1b1b397736707afcde3
     *
     * Input, output, maximum threshold value, mode, threshold type, block size, constant for subtraction
     */
    binary = ~binary; // Invert image
    adaptiveThreshold(binary, binary, THRESHOLD_MAX, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, THRESHOLD_BLOCK_SIZE, THRESHOLD_C);

    // Clone binary image for horizontal and vertical lines extraction
    Mat horizontalLines = binary.clone();
    Mat verticalLines = binary.clone();

    /*
     * Generate the structure elements for these lines.
     * Generally, we use a structure element that has the same form and size as the stuff we want to find.
     * Horizontal lines:
     *  - length = #cols/30 pixels
     *  - height = 1 pixel
     *
     * Vertical lines:
     *  - length = 1 pixel
     *  - height = #rows/30 pixels
     *
     *  https://docs.opencv.org/master/d4/d86/group__imgproc__filter.html#gaeb1e0c1033e3f6b891a25d0511362aeb
     */

    // Generate structure element
    int horizontalSize = horizontalLines.cols / HORIZONTAL_DIVIDER;
    int verticalSize = verticalLines.rows / VERTICAL_DIVIDER;
    // type, Size of structure element
    Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontalSize, HORIZONTAL_HEIGHT));
    Mat verticalStructure = getStructuringElement(MORPH_RECT, Size(VERTICAL_WIDTH, verticalSize));

    // Apply morphology operations on both, anchor = element center (Point(-1, -1))
    erode(horizontalLines, horizontalLines, horizontalStructure, Point(-1, -1));
    dilate(horizontalLines, horizontalLines, horizontalStructure, Point(-1, -1));
    erode(verticalLines, verticalLines, verticalStructure, Point(-1, -1));
    dilate(verticalLines, verticalLines, verticalStructure, Point(-1, -1));

    // Push the results into a NoteSheet struct
    result.staffLines = horizontalLines;
    result.notes = verticalLines;

    return result;
}

/*
 * First approach: Hough line detector to find the staff lines. However, the detector gets confused with notes that are
 * connected to each other:
 *
 *      ------
 *      |    |
 *     *    *
 *
 * The rectangle that connects both notes is the evil one here!
 *
 * Second approach: Vertical histogram of the staff lines. By counting the pixels in the image we retrieve a maximum when
 * we encounter a staff line. This approach circumvents these notes we mentioned above since they won't reach the maximum
 * value like the staff lines.
 *
 * @param Mat input
 * @returns vector<int> distances
 * @author Dylan Van Assche
 */
vector<StaffLineData> getStaffLineDistances(Mat input) {
    Mat img = input.clone();
    vector<StaffLineData> distances;
    vector<StaffLineData> distancesFiltered;

    /*
     * Calculate horizontal histogram (number of pixels in each row), idea from Ann Philips'lab by reducing the matrix
     * to a vector. This can be achieved using the reduce() function: https://docs.opencv.org/3.4.4/d2/de8/group__core__array.html#ga4b78072a303f29d9031d56e5638da78e
     * as described here: http://answers.opencv.org/question/17765/analysis-of-the-vertical-and-horizontal-histogram/
     *
     * Mat input, Mat output, dimension (0 = single row, 1 = single column)
     */
    Mat verticalHistogram;
    reduce(img, verticalHistogram, REDUCE_DIMENSION, CV_REDUCE_SUM, CV_32S);
    drawHistogram(verticalHistogram, input.rows, input.rows);

    /*
     * Finds the local maxima in the histogram.
     * Thanks to: https://stackoverflow.com/questions/28871043/how-do-i-find-the-maximum-number-in-an-array-using-a-function for the idea.
     */
    int previousVal = INT_MIN;

    enum Direction { ASCENDING, DESCENDING };
    Direction direction = ASCENDING;
    for (int i=0; i < verticalHistogram.rows; i++) {
        int currentVal = verticalHistogram.at<int>(i, 0);

        // (still) ascending?
        if (previousVal < currentVal) {
            direction = ASCENDING;
        }
            // (still) descending?
        else if (previousVal > currentVal) {
            // Starts descending? Local maximum retrieved!
            if (direction != DESCENDING) {
                StaffLineData data;
                data.position = i - 1;
                data.value = verticalHistogram.at<int>(i - 1, 0);
                distances.push_back(data);
                direction = DESCENDING;
            }
        }
        // Update previous value
        previousVal = currentVal;
    }

    // Only the 5 biggest results are staff lines, sort them from BIG to SMALL
    sort(distances.begin(), distances.end(), sortStaffLinesBiggestValueFirst);
    for(int s=0; s < NUMBER_OF_STAFF_LINES; ++s) {
        distancesFiltered.push_back(distances.at(s));
    }

    // Sort by position for later usage
    sort(distancesFiltered.begin(), distancesFiltered.end(), sortStaffLinesSmallestPositionFirst);

    return distancesFiltered;
}