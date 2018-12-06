#include "notes.h"

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
    namedWindow("Notes image", WINDOW_AUTOSIZE);
    imshow("Notes image", sheetImg);
    imshow("Symbol image", halfNoteImg);

    // Generate sound wave
    short waveform[NUM_SAMPLES];
    double frequency = NOTE_A;
    int length = NUM_SAMPLES;

    for(int i=0; i < length; i++) {
        double t = (double) i / WAVFILE_SAMPLES_PER_SECOND;
        waveform[i] = (short) (VOLUME * sin(2 * M_PI * frequency * t));
    }

    FILE* f = wavfile_open(outputSoundPath.c_str());
    if(!f)
    {
        cerr << "Opening sound file failed!" << endl;
        return -1;
    }

    wavfile_write(f, waveform, length);
    wavfile_close(f);

    //NoteSheet resultsImg = splitStaffLinesAndNotes(notesImg);

    //imshow("Notes lines", resultsImg.staffLines);
    //imshow("Notes results", resultsImg.notes);

    /*vector<Mat> symbols;
    symbols.push_back(symbolImg);
    detectNotes(notesImg, symbols);*/

    Mat sheetImgCleaned = clean(sheetImg);

    Mat histSheet = getHistogram(sheetImgCleaned);
    Mat sheetDrawing = Mat::zeros(sheetImgCleaned.rows, sheetImgCleaned.cols, CV_8UC1);
    Mat noteDrawing;

    double maxHistogram;
    minMaxIdx(histSheet, NULL, &maxHistogram);

    // Draw calculated horizontal histogram
    cerr << "Hist sheet size:" << histSheet.size() << endl;
    cerr << "max=" << maxHistogram << endl;
    for(int j=0; j < histSheet.cols; j++) {
        int normalize = sheetDrawing.rows * (histSheet.at<int>(0, j)/(float)maxHistogram);
        cerr << normalize << ", ";
        // Mind the order of X/Y and ROW/COLUMN: https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column
        line(sheetDrawing, Point(j, normalize), Point(j, 0), Scalar(255,255,255));
    }
    imshow("sheet drawing", sheetDrawing);
    waitKey(0);


    waitKey(0);
    Mat histNote = getHistogram(halfNoteImg);

    minMaxIdx(histNote, NULL, &maxHistogram);

    // Draw calculated horizontal histogram
    for(int j=0; j < histNote.cols; j++) {
        cerr << histNote.at<int>(0, j) << ", ";
        int normalize = noteDrawing.rows * ((histNote.at<int>(0, j))/maxHistogram);
        // Mind the order of X/Y and ROW/COLUMN: https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column
        line(noteDrawing, Point(j, normalize), Point(j, 0), Scalar(255,255,255));
    }
    waitKey(0);

    int step = histNote.cols/2;
    for(int c = step/2.0; c < histSheet.cols - 3.0*step/2.0; c = c + step) {
        Rect window(
                Point((int)(c - step/2.0), 0),
                Point((int)(c + 3.0*step/2.0), histSheet.rows)
                );

        Rect windowVisible(
                Point((int)(c - step/2.0), 0),
                Point((int)(c + 3.0*step/2.0), sheetDrawing.rows)
        );
        Mat clone = sheetDrawing.clone();

        rectangle(clone, windowVisible, Scalar(255,255,255));

        imshow("window", clone);
        Mat ROI = Mat(histSheet, window);
        ROI.convertTo(ROI, CV_32F);
        histNote.convertTo(histNote, CV_32F);
        cerr << "Size ROI: " << ROI.size << endl;
        cerr << "Size note: " << histNote.size << endl;
        cerr << "ROI type: " << ROI.type() << endl;
        cerr << "Note type: " << histNote.type() << endl;
        cerr << "ROI depth: " << ROI.depth() << endl;
        cerr << "Note depth" << histNote.depth() << endl;
        cerr << "CV_32F=" << CV_32F << endl;
        double corr = compareHist(ROI, histNote, CV_COMP_KL_DIV); // Seems to be the best result
        cout << "Correlation: " << corr << endl;
        waitKey(0);
    }

    // Wait until the user decides to exit the program.
    return 0;
}

// TODO Split code later
Mat clean(Mat input) {
    return input;
}

Mat getHistogram(Mat input) {
    //https://docs.opencv.org/3.4.4/d6/dc7/group__imgproc__hist.html#ga4b2b5fd75503ff9e6844cc4dcdaed35d
    Mat img = input.clone();
    Mat hist = Mat::zeros(input.rows, input.cols, CV_8UC1);

    // Remove noise using an opening operation
    erode(img, img, 0, Point(-1, -1), ERODE_DILATE_ITER);
    dilate(img, img, 0, Point(-1, -1), ERODE_DILATE_ITER);

    // Convert image to it's inverse and threshold the image using adaptive tthresholding.
    img = ~img; // Music notes are now white
    adaptiveThreshold(img, img, THRESHOLD_MAX, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, THRESHOLD_BLOCK_SIZE, THRESHOLD_C);
    imshow("Histogram input", img);

    // Specify size on vertical axis
    int verticalsize = img.rows / 30;

    // Create structure element for extracting vertical lines through morphology operations
    Mat verticalStructure = getStructuringElement(MORPH_RECT, Size( 1,verticalsize));

    // Apply morphology operations
    erode(img, img, verticalStructure, Point(-1, -1));
    dilate(img, img, verticalStructure, Point(-1, -1));
    imshow("Remove stafflines", img);
    /*
     * Calculate horizontal histogram (number of pixels in each row), idea from Ann Philips lab.
     * This can be achieved using the reduce() function: https://docs.opencv.org/3.4.4/d2/de8/group__core__array.html#ga4b78072a303f29d9031d56e5638da78e
     * as described here: http://answers.opencv.org/question/17765/analysis-of-the-vertical-and-horizontal-histogram/
     */
    Mat horizontalHistogram;
    reduce(img, horizontalHistogram, 0, CV_REDUCE_SUM, CV_32S);

    /*
     * Find global maximum to normalize the histogram later.
     * Global minimum is skipped using a NULL pointer as described in the docs.
     * https://docs.opencv.org/3.4.0/d2/de8/group__core__array.html#ga7622c466c628a75d9ed008b42250a73f
     */
    double maxHistogram;
    minMaxIdx(horizontalHistogram, NULL, &maxHistogram);

    // Draw calculated horizontal histogram
    for(int j=0; j < horizontalHistogram.cols; j++) {
        cerr << horizontalHistogram.at<int>(0, j) << ", ";
        int normalize = hist.rows * ((horizontalHistogram.at<int>(0, j))/maxHistogram);
        // Mind the order of X/Y and ROW/COLUMN: https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column
        line(hist, Point(j, normalize), Point(j, 0), Scalar(255,255,255));
    }

    imshow("Result", hist);

    // Find contours
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    RNG rng(123456789);
    findContours(img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
    // find moments of the image


    Mat drawing = Mat::zeros( img.size(), CV_8UC3 );
    for( size_t i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, (int)i, color, 2, 8, hierarchy, 0, Point() );
        Rect box = boundingRect(contours[i]);
        //Rect lowerBox = Rect(box.x, box.y + box.height/2, box.y + box.width / 2, box.x + box.height);
        Mat ROI = img(box);
        Moments m = moments(ROI,true);
        Point p(m.m10/m.m00 + box.x, m.m01/m.m00 + box.y);
        Point centerRectangle(box.x + box.height/2, box.y + box.width/2);

        Point point0 = Point(box.x, box.y);
        Point point1 = Point(box.x + box.width, box.y);
        Point point2 = Point(box.x + box.width, box.y + box.height);
        Point point3 = Point(box.x, box.y + box.height);

        // Split bounding box in 2 parts to see where the blob of the note can be found.
        Rect upperBox = Rect(point0, Point(box.x + box.width, box.y + box.height/2));
        Rect lowerBox = Rect(Point(box.x, box.y + box.height/2), Point(box.x + box.width, box.y + box.height));

        // RECT.contains() provides an easy way to check if a Point is laying inside that rectangle
        double dist = -1.0;
        if(lowerBox.contains(p)) {
            cout << "Centroid in lower part of the bounding box" << endl;
            circle( drawing, point2, 5, Scalar( 0, 0, 255) );
            line(drawing, point2, Point(point2.x, drawing.rows), Scalar(255, 255, 255));
            Point rectPoint = Point(box.x + m.m10/m.m00, drawing.rows);
            dist = norm(p - rectPoint);
        }
        else if(upperBox.contains(p)) {
            cout << "Centroid in upper part of the bounding box" << endl;
            circle( drawing, point0, 5, Scalar( 0, 0, 255) );
            line(drawing, point0, Point(point0.x, drawing.rows), Scalar(255, 255, 255));
            Point rectPoint = Point(box.x + m.m10/m.m00, drawing.rows);
            dist = norm(p - rectPoint);
        }
        else {
            cerr << "Centroid of the note lays outside the bounding box, this may not happen!" << endl;
            continue;
        }
        cout << "Center: " << p << " distance:" << dist << endl;
        circle(drawing, p, 5, Scalar(255,255,255), -1);
        rectangle(drawing, box, color);
        rectangle(drawing, upperBox, Scalar(255,0,0));
        rectangle(drawing, lowerBox, Scalar(0,0,255));
    }

    namedWindow("Contours", WINDOW_AUTOSIZE);
    imshow("Contours", drawing);
    return horizontalHistogram;
}

/*
 * By applying erosion (remove noise) and dilation (connect blobs) using a structure element, we can extract the
 * vertical and horizontal lines of the staff lines. We can remove those lines to make it easy for SURF to find local
 * features for each note. When we have those features, we can match them using RANSAC with our predefined notes.
 *
 * This is doable since only 5 different types of notes are in use (1, 1/2, 1/4, 1/8, 1/16). All of these types can have
 * a dot behind them, that means that the note is extended with the 1/2 of it's value (5 different types again).
 *
 * This approach is based on: https://docs.opencv.org/master/dd/dd7/tutorial_morph_lines_detection.html. I tried to use
 * SURF to locate the staff lines but only the first symbol has 1 - 2 local features which is too low. The lines itself
 * don't have any variation which lead to no local features. SURF is ideal to match the types of notes later!
 *
 * Another approach would be counting the number of pixels in a horizontal histogram of the binary image. All peaks in
 * the histogram are staff lines. However, the morfologic approach is a bit more robust in this case due the use of a
 * specific kernel (a difference of a couple of pixels are ignored).
 */
NoteSheet splitStaffLinesAndNotes(Mat input) {
    Mat binary = input.clone(); // Make sure we don't modify the input
    Mat img = input.clone();
    NoteSheet result;

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

    // Copy image for horizontal and vertical lines extraction
    Mat horizontalLines = binary.clone();
    Mat verticalLines = binary.clone();

    /*
     * Generate the structure elements for these lines.
     * Generally, we use a structure element that has the same form and size as the stuff we want to find.
     * Horizontal lines:
     *  - length = #cols/15 pixels -> only horizontal lines
     *  - height = 1 pixel
     *
     * Vertical lines:
     *  - length = 1 pixel
     *  - height = #rows/25 pixels -> detect the bullets of the notes too
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
    Mat vertical;
    binary.copyTo(vertical, ~horizontalLines);
    imshow("test", vertical);
    waitKey(0);
    //erode(verticalLines, verticalLines, verticalStructure, Point(-1, -1));
    //dilate(verticalLines, verticalLines, verticalStructure, Point(-1, -1));

    // Push the results into a NoteSheet struct
    result.staffLines = horizontalLines;
    result.notes = verticalLines;

    return result;
}