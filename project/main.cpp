#include "notes.h"

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ notes n        | | Loads an image of a music notes sheet <REQUIRED> }"
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
    if(notes.empty())
    {
        cerr << "Please supply your images using command line arguments: --grey=greyImage.png and --color=colorImage.png" << endl;
        return -1;
    }

    // Try to load images
    Mat notesImg;
    Mat staffLinesImg;
    notesImg = imread(notes, IMREAD_GRAYSCALE);

    if(notesImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Notes image", WINDOW_AUTOSIZE);
    imshow("Notes image", notesImg);

    NoteSheet resultsImg = splitStaffLinesAndNotes(notesImg);

    imshow("Notes lines", resultsImg.staffLines);
    imshow("Notes results", resultsImg.notes);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
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
    Mat img = input.clone(); // Make sure we don't modify the input
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
    img = ~img; // Invert image
    adaptiveThreshold(img, img, THRESHOLD_MAX, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, THRESHOLD_BLOCK_SIZE, THRESHOLD_C);

    // Copy image for horizontal and vertical lines extraction
    Mat horizontalLines = img.clone();
    Mat verticalLines = img.clone();

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
    erode(verticalLines, verticalLines, verticalStructure, Point(-1, -1));
    dilate(verticalLines, verticalLines, verticalStructure, Point(-1, -1));

    // Push the results into a NoteSheet struct
    result.staffLines = horizontalLines;
    result.notes = verticalLines;

    return result;
}