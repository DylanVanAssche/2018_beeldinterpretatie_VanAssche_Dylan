#include "notes.h"

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ notes n        | | Loads an image of a music notes sheet <REQUIRED> }"
                             "{ symbols s      | | Loads an image of a music notes symbol <REQUIRED> }"
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
    string notes(parser.get<string>("notes"));
    string symbol(parser.get<string>("symbols"));
    if(notes.empty() || symbol.empty())
    {
        cerr << "Please supply your images using command line arguments: --grey=greyImage.png and --color=colorImage.png" << endl;
        return -1;
    }

    // Try to load images
    Mat notesImg;
    Mat symbolImg;
    notesImg = imread(notes, IMREAD_GRAYSCALE);
    symbolImg = imread(symbol, IMREAD_GRAYSCALE);

    if(notesImg.empty() || symbolImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Notes image", WINDOW_AUTOSIZE);
    imshow("Notes image", notesImg);
    imshow("Symbol image", symbolImg);


    //NoteSheet resultsImg = splitStaffLinesAndNotes(notesImg);

    //imshow("Notes lines", resultsImg.staffLines);
    //imshow("Notes results", resultsImg.notes);

    /*vector<Mat> symbols;
    symbols.push_back(symbolImg);
    detectNotes(notesImg, symbols);*/

    getHistogram(notesImg);
    getHistogram(symbolImg);

    // Wait until the user decides to exit the program.
    return 0;
}

Mat getHistogram(Mat input) {
    //https://docs.opencv.org/3.4.4/d6/dc7/group__imgproc__hist.html#ga4b2b5fd75503ff9e6844cc4dcdaed35d
    Mat img = input.clone();
    Mat hist = Mat::zeros(input.rows, input.cols, CV_8UC1);

    // Remove noise using an opening operation
    erode(img, img, 0, Point(-1, -1));
    dilate(img, img, 0, Point(-1, -1));

    // Convert image to it's inverse and threshold the image using adaptive tthresholding.
    img = ~img;
    adaptiveThreshold(img, img, THRESHOLD_MAX, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, THRESHOLD_BLOCK_SIZE, THRESHOLD_C);
    imshow("Histogram input", img);

    /*
     * Calculate histogram (number of pixels in each row), idea from Ann Philips lab.
     * TODO calcHist() can't be used since it calculates the distribution of pixels instead of the number of pixels 0/1, I can't get it to work? ASKING STEVEN PUTTEMANS
     */
    vector<int> numberOfPixels;
    int max = 0;
    for(int c = 0; c < img.cols; c++) {
        int sum = 0;
        for(int r = 0; r < img.rows; r++) {
            sum += img.at<uchar>(r, c);
        }
        numberOfPixels.push_back(sum);
        if(sum > max) {
            max = sum;
        }
    }

    // Display calculated histogram
    for(int j=0; j < numberOfPixels.size(); j++) {
        int normalize = hist.rows * (numberOfPixels.at(j)/((double)max));
        cout << normalize << ",";
        // Mind the gap/order: https://stackoverflow.com/questions/25642532/opencv-pointx-y-represent-column-row-or-row-column
        line(hist, Point(j, 0), Point(j, normalize), Scalar(255,0,0));
    }
    cout << endl;

    imshow("Result", hist);
    waitKey(0);

    return Mat();
}

/*void detectNotes(Mat noteImg, vector<Mat> symbols) {
    Mat img = noteImg.clone();
    Ptr<Feature2D> detector = BRISK::create(); // SURF is faster than SIFT, NON FREE, fix this later
    vector<KeyPoint> keypoints;
    Mat descriptor;
    detector->detectAndCompute(img, Mat(), keypoints, descriptor);
    drawKeypoints(img, keypoints, img);
    imshow("Keypoints ORB", img);

    vector< vector<DMatch> > matches;
    vector<Mat> descriptorSymbol;

    BFMatcher matcher = BFMatcher(NORM_L2); // Eucledian distance is only compatible with ORB
    for(int i=0; i < symbols.size(); i++) {
        vector<KeyPoint> keys;
        vector<DMatch> match;
        Mat desc;
        detector->detectAndCompute(symbols.at(i), Mat(), keys, desc);
        drawKeypoints(symbols.at(i), keys, symbols.at(i));
        matcher.match(desc, descriptor, match);
        Mat result;
        drawMatches(symbols.at(i), keys, img, keypoints, match, result);
        imshow("matches", result);
        waitKey(0);
    }
}*/

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

/*#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(int, char**)
{
    Mat gray=imread("test.png",0);
    gray = ~gray;
    namedWindow( "Gray", 1 );    imshow( "Gray", gray );

    // Initialize parameters
    int histSize = 256;    // bin size
    float range[] = { 0, 255 };
    const float *ranges[] = { range };

    // Calculate histogram
    MatND hist;
    calcHist( &gray, 1, 0, Mat(), hist, 1, &histSize, ranges, true, false );

    // Show the calculated histogram in command window
    double total;
    total = gray.rows * gray.cols;
    for( int h = 0; h < histSize; h++ )
    {
        float binVal = hist.at<float>(h);
        cout<<" "<<binVal;
    }

    // Plot the histogram
    int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound( (double) hist_w/histSize );

    Mat histImage( hist_h, hist_w, CV_8UC1, Scalar( 0,0,0) );
    normalize(hist, hist, 0, histImage.cols, NORM_MINMAX, -1, Mat() );

    for( int i = 1; i < histSize; i++ )
    {
        line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
              Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
              Scalar( 255, 0, 0), 2, 8, 0  );
    }

    namedWindow( "Result", 1 );    imshow( "Result", histImage );

    waitKey(0);
    return 0;
}*/