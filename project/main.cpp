/*
 * Labo beeldinterpretatie 2018: project
 *
 * ---> PRELIMINARY MUSIC NOTES RECOGNITION <---
 *
 * @author Dylan Van Assche
 */
#include "notes.h"

Mat helpImg;
int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ?                | | Shows this message.                                     }"
                             "{ sheet s                       | | Loads an image of a music notes sheet <REQUIRED>        }"
                             "{ output o                      | | Path to the sound output file <REQUIRED>                }"
                             "{ full-note full                | | Loads an image of a full note symbol <REQUIRED>         }"
                             "{ half-note half                | | Loads an image of a half note symbol <REQUIRED>         }"
                             "{ quarter-note quarter          | | Loads an image of a quarter note symbol <REQUIRED>      }"
                             "{ eighth-note eighth            | | Loads an image of a eighth note symbol <REQUIRED>       }"
                             "{ sixteenth-note sixteenth      | | Loads an image of a sixteenth note symbol <REQUIRED>    }"
                             "{ double-half-note double-half  | | Loads an image of a double-half note symbol <REQUIRED>  }"
                             "{ triple-half-note double-half  | | Loads an image of a triple-half note symbol <REQUIRED>  }"
    );

    // Help printing
    if(parser.has("help") || argc <= 1) {
        cerr << "Please use absolute paths when supplying your images." << endl;
        parser.printMessage();
        return 0;
    }

    // Parser fail
    if (!parser.check())
    {
        parser.printErrors();
        return -1;
    }

    // Required arguments supplied?
    string sheet(parser.get<string>("sheet"));
    string outputSoundPath(parser.get<string>("output"));
    string halfNote(parser.get<string>("half-note"));
    if(sheet.empty() || outputSoundPath.empty() || halfNote.empty())
    {
        cerr << "Please supply your images using command line arguments: --grey=greyImage.png and --color=colorImage.png" << endl; // TODO
        return -1;
    }

    // Try to load images
    Mat sheetImg;
    Mat halfNoteImg;
    sheetImg = imread(sheet, IMREAD_GRAYSCALE);
    halfNoteImg = imread(halfNote, IMREAD_GRAYSCALE);

    if(sheetImg.empty() || halfNoteImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    cout << "Displaying input" << endl;
    namedWindow("Sheet image", WINDOW_AUTOSIZE);
    imshow("Sheet image", sheetImg);
    waitKey(0);

    // Split stafflines from input image
    NoteSheet noteSheet = splitStaffLinesAndNotes(sheetImg);
    imshow("Splitting notes", noteSheet.notes);
    imshow("Splitting stafflines", noteSheet.staffLines);
    waitKey(0);

    // Get horizontal histogram of the sheet and notes
    Mat histSheet = getHorizontalHistogram(noteSheet.notes);
    Mat histNote = getHorizontalHistogram(halfNoteImg);

    // Draw the histogram
    drawHistogram(histSheet, sheetImg.rows, sheetImg.cols);
    drawHistogram(histNote, halfNoteImg.rows, halfNoteImg.cols);

    // Find contours and display them
    ContoursData contoursSheet = getContours(noteSheet.notes);
    ContoursData contoursNote = getContours(halfNoteImg);
    drawContoursWithOrientation(contoursSheet, sheetImg.rows, sheetImg.cols);
    drawContoursWithOrientation(contoursNote, halfNoteImg.rows, halfNoteImg.cols);
    //double matchValue = matchShapes(contoursSheet.contours, contoursNote.contours, CV_CONTOURS_MATCH_I1, 0);
    //cout << matchValue << endl;

    // Find the distances between the staff lines
    vector<StaffLineData> distances = getStaffLineDistances(noteSheet.staffLines);

    for(int d=0; d < distances.size(); ++d) {
        cout << "staff line position:" << distances.at(d).position << endl;
    }

    helpImg = noteSheet.staffLines.clone();
    vector<Note> notes = convertDataToNote(contoursSheet, distances, sheetImg.rows, sheetImg.cols);

    // Generate wave
    vector<vector<short> > waves;
    for(int n=0; n < notes.size(); ++n) {
        waves.push_back(generateWaveform(notes.at(n).frequency, notes.at(n).length/10));
    }
    saveWaveforms(outputSoundPath, waves);


   /* int step = histNote.cols/2;
    vector<double> comparison;
    double maxComp = 0;
    for(int c = step/2.0; c < histSheet.cols - 3.0*step/2.0; c++) {
        Rect
        window(
                Point((int) (c - step / 2.0), 0),
                Point((int) (c + 3.0 * step / 2.0), histSheet.rows)
        );

        Rect
        windowVisible(
                Point((int) (c - step / 2.0), 0),
                Point((int) (c + 3.0 * step / 2.0), sheetDrawing.rows)
        );
        Mat clone = sheetDrawing.clone();

        rectangle(clone, windowVisible, Scalar(255, 255, 255));

        imshow("window", clone);
        Mat ROI = Mat(histSheet, window);
        ROI.convertTo(ROI, CV_32F);
        histNote.convertTo(histNote, CV_32F);
        cerr << "Size ROI: " << ROI.size << endl;
        cerr << "Size note: " << histNote.size << endl;
        double corr = compareHist(ROI, histNote, CV_COMP_BHATTACHARYYA); // Seems to be the best result
        cout << "Divergent: " << corr << endl;
        comparison.push_back(corr);
        if (corr > maxComp) {
            maxComp = corr;
        }
        if (corr > 0.99) {
            waitKey(0);
        }
    }*/
/*
    Mat graph = Mat::zeros(noteSheet.notes.rows, noteSheet.notes.cols, CV_8UC1);

    for(int j=1; j < comparison.size(); j++) {
        int normalize1 = graph.rows * ((comparison.at(j-1))/maxComp);
        int normalize2 = graph.rows * ((comparison.at(j))/maxComp);
        // Mind the order of X/Y and ROW/COLUMN: https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column
        line(graph, Point(j, normalize1), Point(j, normalize2), Scalar(255,255,255));
    }

    imshow("graph", graph);
    waitKey(0);*/

    // Wait until the user decides to exit the program.
    return 0;
}

/*
 * Calculate the horizontal histogram using the OpenCV reduce() function.
 * Internally, the pixels of each column are counted and written to a Mat object.
 *
 * @param Mat input
 * @returns Mat histogram
 * @author Dylan Van Assche
 */
Mat getHorizontalHistogram(Mat input) {
    //https://docs.opencv.org/3.4.4/d6/dc7/group__imgproc__hist.html#ga4b2b5fd75503ff9e6844cc4dcdaed35d
    Mat img = input.clone();
    Mat hist = Mat::zeros(input.rows, input.cols, CV_8UC1);

    /*
     * Calculate horizontal histogram (number of pixels in each row), idea from Ann Philips'lab.
     * This can be achieved using the reduce() function: https://docs.opencv.org/3.4.4/d2/de8/group__core__array.html#ga4b78072a303f29d9031d56e5638da78e
     * as described here: http://answers.opencv.org/question/17765/analysis-of-the-vertical-and-horizontal-histogram/
     */
    Mat horizontalHistogram;
    reduce(img, horizontalHistogram, 0, CV_REDUCE_SUM, CV_32S);
    return horizontalHistogram;
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

    /*
     * Find global maximum to normalize the histogram later.
     * Global minimum is skipped using a NULL pointer as described in the docs.
     * https://docs.opencv.org/3.4.0/d2/de8/group__core__array.html#ga7622c466c628a75d9ed008b42250a73f
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
        cerr << normalize << ",";
        // Mind the order of X/Y and ROW/COLUMN: https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column
        line(drawing, Point(i, normalize), Point(i, 0), Scalar(255,255,255));
    }
    cerr << endl;

    // Display histogram and wait for key
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

    // Remove noise using an opening operation
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
 * Calculates the contours of the input image. Afterwards, it finds the orientation of the notes in the image by using
 * the centroid of the image. Thanks to the blob on each note, the centroid will move towards the blob. This way we can
 * find the orientation of the note in an easy way.
 *
 * @param Mat input
 * @return ContoursData data
 * @author Dylan Van Assche
 */
ContoursData getContours(Mat input) {
    Mat img = input.clone();
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<Point> orientation;
    RNG rng(RNG_INIT);

    // Find contours
    findContours(img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    for(int i=0; i < contours.size(); ++i)
    {
        // Get the bounding box of the current contour
        Rect box = boundingRect(contours.at(i));

        // Find the centroid of the image by using OpenCV Moments in the ROI (=bouding box)
        Mat ROI = img(box);
        Moments m = moments(ROI,true);
        Point centroid(m.m10/m.m00 + box.x, m.m01/m.m00 + box.y);

        // Split bounding box in 2 parts to see where the blob of the note can be found.
        Rect upperBox = Rect(Point(box.x, box.y), Point(box.x + box.width, box.y + box.height/2));
        Rect lowerBox = Rect(Point(box.x, box.y + box.height/2), Point(box.x + box.width, box.y + box.height));

        // RECT.contains() provides an easy way to check if a Point is laying inside that rectangle
        if(lowerBox.contains(centroid)) {
            orientation.push_back(Point(box.x + box.width, box.y + box.height));
        }
        else if(upperBox.contains(centroid)) {
            orientation.push_back(Point(box.x, box.y));
        }
        else {
            cerr << "Centroid of the note lays outside the bounding box, this may not happen!" << endl;
            orientation.push_back(Point(-1, -1));
            continue;
        }
    }

    // Combine the extracted data into a ContoursData struct
    ContoursData data;
    data.contours = contours;
    data.hierarchy = hierarchy;
    data.orientation = orientation;
    return data;
}

/*
 * Draws the contours using a random color generators and displays it.
 *
 * @param ContoursData data
 * @param int rows
 * @param int cols
 * @author Dylan Van Assche
 */
void drawContoursWithOrientation(ContoursData data, int rows, int cols) {
    RNG rng(RNG_INIT);
    Mat drawing = Mat::zeros(rows, cols, CV_8UC3);

    double toneHeight = 0;
    Point orientationPoint = Point(0, 0);
    Point bottomPoint = Point(0, 0);
    for(int i = 0; i < data.contours.size(); ++i) {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        drawContours(drawing, data.contours, (int)i, color, 2, 8, data.hierarchy, 0, Point());

        orientationPoint = data.orientation.at(i);
        bottomPoint = Point(orientationPoint.x, drawing.rows);
        circle(drawing, orientationPoint, 5, Scalar( 0, 0, 255));
        line(drawing, orientationPoint, bottomPoint, Scalar(255, 255, 255));
        toneHeight = norm(orientationPoint - bottomPoint);
        cout << toneHeight << endl;
    }

    namedWindow("Contours notes", WINDOW_AUTOSIZE);
    imshow("Contours notes", drawing);
    waitKey(0);
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
     * Calculate horizontal histogram (number of pixels in each row), idea from Ann Philips'lab.
     * This can be achieved using the reduce() function: https://docs.opencv.org/3.4.4/d2/de8/group__core__array.html#ga4b78072a303f29d9031d56e5638da78e
     * as described here: http://answers.opencv.org/question/17765/analysis-of-the-vertical-and-horizontal-histogram/
     */
    Mat verticalHistogram;
    reduce(img, verticalHistogram, 1, CV_REDUCE_SUM, CV_32S);
    drawHistogram(verticalHistogram, input.rows, input.rows);

    /*
     * Finds the local maxima in the histogram.
     * Thanks to: https://stackoverflow.com/questions/28871043/how-do-i-find-the-maximum-number-in-an-array-using-a-function for the idea.
     */
    int previousVal = INT_MIN;

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

/*
 * Combines the contours information and the distances between staff lines to find the frequency of each note.
 *
 * @param ContoursData data
 * @param vector<int> staffLineDistances
 * @returns vector<Note>
 * @author Dylan Van Assche
 */
vector<Note> convertDataToNote(ContoursData data, vector<StaffLineData> staffLineDistances, int rows, int cols) {
    Mat drawing = Mat::zeros(rows, cols, CV_8UC3);
    helpImg.copyTo(drawing);
    cvtColor(drawing, drawing, CV_GRAY2BGR);
    double frequency = 0.0;
    double length = 0.0;
    Point noteLocation;
    vector<Rect> areas;
    vector<Note> notes;
    RNG rng(RNG_INIT);

    /*
     * Create areas for the staff lines
     * https://www.muzieklerenlezen.nl/les-7-muzieknoten-een-naam-geven
     * The height (measured from below) = frequency from the note (inverted, in comparison to OpenCV XY coordinates!)
     *
     * |
     * |                                                            C
     * |                                                        B
     * |                                                    A
     * |                                                G
     * +--------------------------------------------F----------------------
     * |                                        E
     * +------------------------------------D------------------------------
     * |                                C
     * +----------------------------B--------------------------------------
     * |                        A
     * +--------------------G----------------------------------------------
     * |                F
     * +------------E------------------------------------------------------
     * |        D
     * |    C
     * |
     *
     * /!\ To keep this proof-of-concept simple, we will focus on the notes within the reach of the staff lines.
     *      Every note before or after the staff lines are ignored and reduced to NOTE_D or NOTE_G.
     *
     */

    if(staffLineDistances.size() < 2) {
        cerr << "Number of staff lines is too low to find the frequency: " << staffLineDistances.size() << endl;
        return notes;
    }

    Scalar color1 = Scalar(0, 255, 0);
    Scalar color2 = Scalar(0, 0, 255);
    int distanceBetween = abs(staffLineDistances.at(0).position - staffLineDistances.at(1).position);

    // Before the first staff line
    Rect areaBefore = Rect(
            Point(0, 0),
            Point(cols, staffLineDistances.at(0).position - distanceBetween/4)
            );
    rectangle(drawing, areaBefore, color2);
    areas.push_back(areaBefore);

    for(int j=0; j < staffLineDistances.size(); ++j) {
        int yPosition = staffLineDistances.at(j).position;

        // Only area between when we are really between 2 staff lines
        if(j > 0) {
            distanceBetween = abs(staffLineDistances.at(j).position - staffLineDistances.at(j - 1).position);

            // Between staff lines
            Rect areaBetween = Rect(
                    Point(0, yPosition - 3*distanceBetween/4),
                    Point(cols, yPosition - distanceBetween/4)
                    );
            rectangle(drawing, areaBetween, color2);
            areas.push_back(areaBetween);
        }

        // On a staff line
        Rect areaOn = Rect(
                Point(0, yPosition - distanceBetween/4),
                Point(cols, yPosition + distanceBetween/4)
                );
        rectangle(drawing, areaOn, color1);
        areas.push_back(areaOn);
    }

    // After the last staff line
    Rect areaAfter = Rect(
            Point(0, staffLineDistances.at(staffLineDistances.size() - 1).position + distanceBetween/4),
            Point(cols, rows)
            );
    areas.push_back(areaAfter);
    rectangle(drawing, areaAfter, color2);

    // Find for every note, it's frequency by checking it's location
    for(int i=0; i < data.orientation.size(); ++i) {
        Note note;
        noteLocation = data.orientation.at(i);
        circle(drawing, noteLocation, 3, Scalar(255, 0, 0), -1);

        // Check between/on which staff lines the note is sitting
        for(int a=0; a < areas.size(); ++a) {
            if(areas.at(a).contains(noteLocation)) {
                frequency = _convertIndexToNote(a);
                cout << "Note frequency: " << frequency << endl;
                break;
            }
        }

        // Find the length of each note
        note.frequency = frequency;
        note.length = NUM_SAMPLES; //length; // TODO

        notes.push_back(note);
        circle(drawing, noteLocation, 10, Scalar(255, 0, 0), 5);
        imshow("Area test", drawing);
        waitKey(0);
    }

    return notes;
}

/*
 * Private function to retrieve the note frequency by index of the rectangles in convertDataToNote()
 * Since we use the points of the bounding box, the tone height is always shifted with 1 in it's index
 * A better option would be to locate the center of the blob instead of the bounding box points.
 * This could be done with a SimpleBlobDetector and a ROI based on the orientation of the note which we already know.
 *
 * @param int index
 * @return double frequency
 * @author Dylan Van Assche
 */
double _convertIndexToNoteFrequency(int index) {
    // index == 0 -> area before is ignored in this POC, every note there must stay on index 0.
    if(index > 0) {
        index++;
    }

    switch(index) {
        case 0:
            return NOTE_G;
        case 1:
            return NOTE_F;
        case 2:
            return NOTE_E;
        case 3:
            return NOTE_D;
        case 4:
            return NOTE_C;
        case 5:
            return NOTE_B;
        case 6:
            return NOTE_A;
        case 7:
            return NOTE_G;
        case 8:
            return NOTE_F;
        case 9:
            return NOTE_E;
        case 10:
        case 11:
            return NOTE_D;
        default:
            cerr << "Can't estimate note, frequency is set to 0.0" << endl;
            return 0.0;
    }
}

/*
 * Generates a sine wave with a given frequency and length.
 * The result is a vector of shorts which can be written to a WAV file later.
 *
 * @param double frequency
 * @param double length
 * @returns vector<short> waveForm
 * @author Dylan Van Assche
 */
vector<short> generateWaveform(double frequency = NOTE_A, double length = NUM_SAMPLES) {
    vector<short> waveform;

    for(int i=0; i < length; i++) {
        double t = (double) i / WAVFILE_SAMPLES_PER_SECOND;
        waveform.push_back((short) (VOLUME * sin(2 * M_PI * frequency * t)));
    }

    return waveform;
}

/*
 * Writes a vector of waveforms to a WAV file using the WAVFile C library.
 * If the file can't be opened, this function returns and writes an error message to the console.
 *
 * @param string outputPath
 * @param vector< vector<short> > waveforms
 * @author Dylan Van Assche
 */
void saveWaveforms(string outputPath, vector< vector<short> > waveforms) {
    // Open WAV file
    FILE* f = wavfile_open(outputPath.c_str());
    if(!f)
    {
        cerr << "Opening sound file failed!" << endl;
        return;
    }

    // Write each waveform to the WAV file
    for(int w=0; w < waveforms.size(); ++w) {
        // Convert C++ vector to C array (https://stackoverflow.com/questions/1733143/converting-between-c-stdvector-and-c-array-without-copying)
        short* array = &waveforms.at(w)[0];
        std::copy(waveforms.at(w).begin(), waveforms.at(w).end(), array);
        wavfile_write(f, array, (int)waveforms.at(w).size());
    }

    // Close the WAV file
    wavfile_close(f);
}