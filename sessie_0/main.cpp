#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ grey g         | | Loads a grey image <REQUIRED> }"
                             "{ color c        | | Loads a color image <REQUIRED> }"
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
    string grey(parser.get<string>("grey"));
    string color(parser.get<string>("color"));
    if(grey.empty() || color.empty())
    {
        cerr << "Please supply your images using command line arguments: --grey=greyImage.png and --color=colorImage.png" << endl;
        return -1;
    }

    // Try to load images
    Mat greyImg;
    Mat colorImg; // BGR
    greyImg = imread(grey, IMREAD_GRAYSCALE);
    colorImg = imread(color, IMREAD_COLOR);

    if(greyImg.empty() || colorImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Grey image", WINDOW_AUTOSIZE);
    namedWindow("Color image", WINDOW_AUTOSIZE);
    imshow("Grey image", greyImg);
    imshow("Color image", colorImg);

    // Splitting color channels
    Mat splitted[3];
    split(colorImg, splitted);

    namedWindow("Color BLUE channel");
    namedWindow("Color GREEN channel");
    namedWindow("Color RED channel");
    imshow("Color BLUE channel", splitted[0]);
    imshow("Color GREEN channel", splitted[1]);
    imshow("Color RED channel", splitted[2]);

    // Transform color image to grey image using cvtColor
    Mat colorImg2Grey;
    cvtColor(colorImg, colorImg2Grey, COLOR_BGR2GRAY);
    namedWindow("Color to grey");
    imshow("Color to grey", colorImg2Grey);

    // Print all pixels of our color2grey image to the command line;
    for(int rowIndex = 0; rowIndex < colorImg2Grey.rows; rowIndex++)
    {
        for(int columnIndex = 0; columnIndex < colorImg2Grey.cols; columnIndex++) {
            cout << (int)colorImg2Grey.at<uchar>(rowIndex, columnIndex); // typecasting to show it properly in the terminal
            cout << " ";
        }
        cout << endl; // new line when row++
    }
    cout << endl;

    // Canvas circle + rectangle + line drawing
    Mat canvas = Mat::zeros(512, 512, CV_8UC3); // 512 x 512 pixel: 8 bits unsigned integer 3 channels, all black
    circle(canvas, Point(100, 100), 20, Scalar(127, 255, 0), 1);
    rectangle(canvas, Point(150, 150), Point(400, 300), Scalar(255, 0, 127), 10);
    line(canvas, Point(512, 0), Point(0, 512), Scalar(255, 255, 255), 20);
    namedWindow("Canvas", WINDOW_AUTOSIZE);
    imshow("Canvas", canvas);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}