#include "strawberry.h"

vector<Point2d> savedPositivePoints;
vector<Point2d> savedNegativePoints;
Mat strawberryImg;
int mode = 0;

void mouse(int event, int x, int y, int flags, void* userdata) {
    if  ( event == EVENT_LBUTTONDOWN )
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        Point2d p = Point2d(x, y);
        if(mode) {
            savedPositivePoints.push_back(p);
        }
        else {
            savedNegativePoints.push_back(p);
        }
    }
    else if  ( event == EVENT_RBUTTONDOWN )
    {
        cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        if(mode) {
            savedPositivePoints.pop_back();
        }
        else {
            savedNegativePoints.pop_back();
        }
    }
    else if  ( event == EVENT_MBUTTONDOWN )
    {
        cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        if(mode) {
            cout << "POSITIVE" << endl;
            for (int i = 0; i < savedPositivePoints.size(); i++) {
                cout << savedPositivePoints.at(i) << endl;
            }
        }
        else {
            cout << "NEGATIVE" << endl;
            for (int i = 0; i < savedNegativePoints.size(); i++) {
                cout << savedNegativePoints.at(i) << endl;
            }
        }
    }

    runner(0, NULL);
}

void descriptor(Mat img, vector<Point2d> foregroundPoints, vector<Point2d> backgroundPoints, Mat& trainingData, Mat& labels) {
    Mat hsv;
    Mat trainingDataForeground(foregroundPoints.size(), 3, CV_32FC1);
    Mat trainingDataBackground(backgroundPoints.size(), 3, CV_32FC1);
    Mat labels_fg = Mat::ones(foregroundPoints.size(), 1, CV_32SC1);
    Mat labels_bg = Mat::zeros(backgroundPoints.size(), 1, CV_32SC1);
    cvtColor(img, hsv, CV_BGR2HSV);

    // foreground
    for(int i=0; i < foregroundPoints.size(); i++) {
        Vec3b pixel = hsv.at<Vec3b>(foregroundPoints.at(i).y, foregroundPoints.at(i).x);
        trainingDataForeground.at<float>(i, 0) = pixel[0];
        trainingDataForeground.at<float>(i, 1) = pixel[1];
        trainingDataForeground.at<float>(i, 2) = pixel[2];
    }

    // background
    for(int i=0; i < backgroundPoints.size(); i++) {
        Vec3b pixel = hsv.at<Vec3b>(backgroundPoints.at(i).y, backgroundPoints.at(i).x);
        trainingDataBackground.at<float>(i, 0) = pixel[0];
        trainingDataBackground.at<float>(i, 1) = pixel[1];
        trainingDataBackground.at<float>(i, 2) = pixel[2];
    }

    // group both sets
    vconcat(trainingDataForeground, trainingDataBackground, trainingData);
    vconcat(labels_fg, labels_bg, labels);

    cout << trainingData.size() << endl;
    cout << labels.size() << endl;
}


int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ strawberry s   | | Loads a strawberry image <REQUIRED> }"
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
    string strawberry(parser.get<string>("strawberry"));
    if(strawberry.empty())
    {
        cerr << "Please supply your images using command line arguments: --strawberry=strawberry1.tif" << endl;
        return -1;
    }

    // Try to load images
    strawberryImg = imread(strawberry, IMREAD_COLOR);

    if(strawberryImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Strawberry image", WINDOW_AUTOSIZE);
    setMouseCallback("Strawberry image", mouse, NULL);
    createTrackbar("Mode:\n1=POSITIVE\n0=NEGATIVE", "Strawberry image", &mode, 1, runner); // 0 - 1

    runner(0, NULL);

    // Wait until the user decides to exit the selection of points.
    waitKey(0);

    // Calculate the descriptor
    Mat trainingData, labels;
    descriptor(strawberryImg, savedPositivePoints, savedNegativePoints, trainingData, labels);

    // Execute machine learning (Naive Bayes, K-Means Nearest Neighbor, Support Vector Machine)
    KNN(trainingData, labels);

    return 0;
}

void runner(int trackbarPos, void *data)
{
    Mat showPointsImg = strawberryImg.clone();
    GaussianBlur(showPointsImg, showPointsImg, Size(KERNEL_SIZE, KERNEL_SIZE), 0);
    int thickness = -1;
    int radius = 5;
    for(int i=0; i < savedNegativePoints.size(); i++) {
        circle(showPointsImg, savedNegativePoints.at(i), radius, Scalar(0, 0, 255), thickness);
    }
    for(int i=0; i < savedPositivePoints.size(); i++) {
        circle(showPointsImg, savedPositivePoints.at(i), radius, Scalar(0, 255, 0), thickness);
    }

    imshow("Strawberry image", showPointsImg);
}

void KNN(Mat trainingsData, Mat labels) {

}

void NaiveBayes(Mat trainingsData, Mat labels) {

}

void SVM(Mat trainingsData, Mat labels) {

}
// substract green channel