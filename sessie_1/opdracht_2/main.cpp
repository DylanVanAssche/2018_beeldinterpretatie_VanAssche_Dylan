#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
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
    string color(parser.get<string>("color"));
    if(color.empty())
    {
        cerr << "Please supply your images using command line arguments: --color=imageColorAdapted.jpg" << endl;
        return -1;
    }

    // Try to load images
    Mat colorImg; // BGR
    colorImg = imread(color, IMREAD_COLOR);

    if(colorImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Thresholding skin pixels
    // Approach 1: looping through all the pixels using a double for lus
    // Print all pixels of our colorImg image to the command line;
    Mat maskerLoop(colorImg.size(), CV_8UC1);
    Mat segmentationLoop(colorImg.size(), CV_8UC3);
    for(int rowIndex = 0; rowIndex < colorImg.rows; rowIndex++)
    {
        for(int columnIndex = 0; columnIndex < colorImg.cols; columnIndex++) {
            cout << "(" << (int)colorImg.at<Vec3b>(rowIndex, columnIndex)[0] << "," << (int)colorImg.at<Vec3b>(rowIndex, columnIndex)[1] << "," << (int)colorImg.at<Vec3b>(rowIndex, columnIndex)[2] << ")"; // typecasting to show it properly in the terminal
            cout << " ";
            int blue = (int)colorImg.at<Vec3b>(rowIndex, columnIndex)[0];
            int green = (int)colorImg.at<Vec3b>(rowIndex, columnIndex)[1];
            int red = (int)colorImg.at<Vec3b>(rowIndex, columnIndex)[2];
            // Filter formula from the assignment
            if((red > 95) && (green > 40) && (blue > 20) && ((max(red, max(green, blue)) - min(red, min(green, blue))) > 15) && (abs(red - green) > 15) && (red > green) && (red > blue))
            {
                maskerLoop.at<uchar>(rowIndex, columnIndex) = 255;
            }
            else {
                maskerLoop.at<uchar>(rowIndex, columnIndex) = 0;
            }
        }
        cout << endl; // new line when row++
    }
    cout << endl;

    // Combine masker and image
    colorImg.copyTo(segmentationLoop, maskerLoop);

    namedWindow("Skin extraction pixel loop", WINDOW_AUTOSIZE);
    imshow("Skin extraction pixel loop", maskerLoop);
    namedWindow("Segmentation pixel loop", WINDOW_AUTOSIZE);
    imshow("Segmentation pixel loop", segmentationLoop);
    waitKey(0); // Wait until the user presses a key

    // Approach 2: using OpenCV matrix operations to retrieve all the pixels (binary image) that are valid for the filter
    Mat splitted[3];
    Mat maskerMatrixOperation(colorImg.size(), CV_8UC1);
    Mat segmentationMatrixOperation(colorImg.size(), CV_8UC3);
    split(colorImg, splitted);
    Mat blue = splitted[0]; // merge() replaces this boiler plate code
    Mat green = splitted[1];
    Mat red = splitted[2];
    maskerMatrixOperation = (red > 95) & (green > 40) & (blue > 20) & ((max(red, max(green, blue)) - min(red, min(green, blue))) > 15) & (abs(red - green) > 15) & (red > green) & (red > blue);
    // Combine masker and image
    colorImg.copyTo(segmentationMatrixOperation, maskerMatrixOperation);

    namedWindow("Skin extraction matrix operations", WINDOW_AUTOSIZE);
    imshow("Skin extraction matrix operations", maskerMatrixOperation);
    namedWindow("Segmentation matrix operations", WINDOW_AUTOSIZE);
    imshow("Segmentation matrix operations", segmentationMatrixOperation);
    waitKey(0); // Wait until the user presses a key

    // Optimize mask with opening, closing; dilation and erosion
    cerr << "Optimizing mask" << endl;
    erode(maskerMatrixOperation, maskerMatrixOperation, Mat(), Point(-1, -1), 2); // noise suppression x2 due huge pixels
    dilate(maskerMatrixOperation, maskerMatrixOperation, Mat(), Point(-1, -1), 2); // fix erode data loss
    namedWindow("Remove noise", WINDOW_AUTOSIZE);
    imshow("Remove noise", maskerMatrixOperation);
    waitKey(0);

    // Connect blobs
    dilate(maskerMatrixOperation, maskerMatrixOperation, Mat(), Point(-1, -1), 2); // connect blobs
    erode(maskerMatrixOperation, maskerMatrixOperation, Mat(), Point(-1, -1), 2); // fix dilate data loss
    namedWindow("Connect blobs", WINDOW_AUTOSIZE);
    imshow("Connect blobs", maskerMatrixOperation);
    waitKey(0);

    // Convex hull approach -> contours
    vector< vector<Point> > contours;
    findContours(maskerMatrixOperation.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_NONE); // explain defines
    vector< vector<Point> > hulls;
    for(size_t i=0; i < contours.size(); i++) {
        vector<Point> hull;
        convexHull(contours[i], hull);
        hulls.push_back(hull);
    }
    drawContours(maskerMatrixOperation, hulls, -1, 255, -1); // check docs -1

    Mat contouredImg(colorImg.size(), CV_8UC3);
    colorImg.copyTo(contouredImg, maskerMatrixOperation);
    namedWindow("Draw contours", WINDOW_AUTOSIZE);
    imshow("Draw contours", contouredImg);
    waitKey(0);
/*
    // Bimodal OTSU
    namedWindow("OTSU original", WINDOW_AUTOSIZE);
    imshow("OTSU original", );

    Mat threshedImg;
    threshold(bimodalImg, threshedImg, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    namedWindow("OTSU thresholding", WINDOW_AUTOSIZE);
    imshow("OTSU thresholding", threshedImg);
    waitKey(0); // Wait until the user presses a key
    // A part of the ticket is completely black (left, bottom), OTSU depends on a bimodal image -> no uniform background color
    // We can use histogram equalisation and CLAHE to optimize the image before applying OTSU
    // 1. histogram equalisation: isn't that great either
    Mat bimodalImgEqualized;
    Mat threshedImgEqualized;
    equalizeHist(bimodalImg, bimodalImgEqualized); // previously: .clone() to avoid losing original matrix (obselete by now)
    threshold(bimodalImgEqualized, threshedImgEqualized, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    namedWindow("Equalized OTSU", WINDOW_AUTOSIZE);
    imshow("Equalized OTSU", threshedImgEqualized);
    waitKey(0);

    // 2. CLAHE: advanced locally equalisation using a window, a bit better than normal equalisation since the
    // thresholding is applied locally, non equal backgrounds are difficult for OTSU, CLAHE makes the background more
    // equally then histogram thresholding
    Mat bimodalImgCLAHE;
    Mat threshedImgCLAHE;
    Ptr<CLAHE> clahe_p = createCLAHE();
    clahe_p->setTilesGridSize(Size(15, 15)); // window size
    clahe_p->setClipLimit(1); // 1 - 10 contrast
    clahe_p->apply(bimodalImg.clone(), bimodalImgCLAHE); // still old C function, use .clone() to be sure
    threshold(bimodalImgCLAHE, threshedImgCLAHE, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    namedWindow("OTSU CLAHE", WINDOW_AUTOSIZE);
    imshow("OTSU CLAHE", threshedImgCLAHE);
    waitKey(0);*/

    return 0;
}