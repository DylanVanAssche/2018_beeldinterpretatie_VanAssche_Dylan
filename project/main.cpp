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
    notesImg = imread(notes, IMREAD_GRAYSCALE);

    if(notesImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Notes image", WINDOW_AUTOSIZE);
    imshow("Notes image", notesImg);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}

Mat findStaffLines(Mat input) {

}