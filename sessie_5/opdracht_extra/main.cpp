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

    // Remove the green channel to improve red strawberries detection
    Mat channels[3];
    split(strawberryImg, channels);
    channels[1] = Mat::zeros(strawberryImg.rows, strawberryImg.cols, CV_8UC1);
    merge(channels, strawberryImg.channels(), strawberryImg);

    // Displays the images in a window
    namedWindow("Strawberry image", WINDOW_AUTOSIZE);
    setMouseCallback("Strawberry image", mouse, NULL);
    createTrackbar("Mode:\n1=POSITIVE\n0=NEGATIVE", "Strawberry image", &mode, 1, runner); // 0 - 1

    runner(0, NULL);

    // Wait until the user decides to exit the selection of points.
    waitKey(0);

    if(savedPositivePoints.size() == 0 || savedNegativePoints.size() == 0) {
        cerr << "You should annotate at least some POSITIVE and NEGATIVE points on the image to continue";
        return -1;
    }

    // Calculate the descriptor
    Mat trainingData, labels;
    descriptor(strawberryImg, savedPositivePoints, savedNegativePoints, trainingData, labels);

    // Execute machine learning (Naive Bayes, K-Means Nearest Neighbor, Support Vector Machine)
    KNN(trainingData, labels);
    NaiveBayes(trainingData, labels);
    SVM(trainingData, labels);

    return 0;
}

void runner(int trackbarPos, void *data)
{
    Mat showPointsImg = strawberryImg.clone();
    GaussianBlur(showPointsImg, showPointsImg, Size(KERNEL_SIZE, KERNEL_SIZE), 0);

    for(int i=0; i < savedNegativePoints.size(); i++) {
        circle(showPointsImg, savedNegativePoints.at(i), CIRCLE_RADIUS, Scalar(0, 0, 255), CIRCLE_THICKNESS);
    }

    for(int i=0; i < savedPositivePoints.size(); i++) {
        circle(showPointsImg, savedPositivePoints.at(i), CIRCLE_RADIUS, Scalar(0, 255, 0), CIRCLE_THICKNESS);
    }

    imshow("Strawberry image", showPointsImg);
}

void KNN(Mat trainingsData, Mat labels) {
    cout << "Validating each pixel with KNN" << endl;

    // Create KNearest classifier
    Ptr<ml::KNearest> knn = ml::KNearest::create();
    knn->setDefaultK(KNN_GROUPS);
    knn->setIsClassifier(true);
    knn->setAlgorithmType(ml::KNearest::BRUTE_FORCE);

    // Train classifier
    knn->train(trainingsData, ml::ROW_SAMPLE, labels);

    // Show results
    cout << "Showing KNN" << endl;
    showResult(knn);
}

void NaiveBayes(Mat trainingsData, Mat labels) {
    cout << "Validating each pixel with NaiveBayes" << endl;

    // Create KNearest classifier
    Ptr<ml::NormalBayesClassifier> nb = ml::NormalBayesClassifier::create();

    // Train classifier
    nb->train(trainingsData, ml::ROW_SAMPLE, labels);

    // Show results
    cout << "Showing NB" << endl;
    showResult(nb);
}

void SVM(Mat trainingsData, Mat labels) {
    cout << "Validating each pixel with SVM" << endl;

    // Create KNearest classifier
    Ptr<ml::SVM> svm = ml::SVM::create();
    svm->setKernel(ml::SVM::LINEAR); // Exponential and other kernels are possible too (https://docs.opencv.org/3.0.0/d1/d2d/classcv_1_1ml_1_1SVM.html)
    svm->setType(ml::SVM::C_SVC); // https://docs.opencv.org/3.0.0/d1/d2d/classcv_1_1ml_1_1SVM.html -> default is SVM::C_SVC
    svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, SVM_ITER, SVM_EPSILON)); // Stop after SVM_ITER iterations, SVM_EPSILON is the accuary or change in parameters (https://docs.opencv.org/3.0.0/d9/d5d/classcv_1_1TermCriteria.html)

    // Train classifier
    svm->train(trainingsData, ml::ROW_SAMPLE, labels);

    // Show results
    cout << "Showing SVM" << endl;
    showResult(svm);
}

void showResult(Ptr<ml::StatModel> classifier) {
    Mat label;
    Mat mask = Mat::zeros(strawberryImg.rows, strawberryImg.cols, CV_8UC1);
    Mat result;
    Mat HSV = strawberryImg.clone();
    cvtColor(HSV, HSV, CV_BGR2HSV);

    // Validate each pixel
    for(int r=0; r < HSV.rows; r++) {
        for(int c=0; c < HSV.cols; c++) {
            Vec3b pixel = HSV.at<Vec3b>(r, c); // findNearest wants it as a Mat object
            Mat pixelMat = Mat(1, 3,
                               CV_32FC1); // 1 pixel, 3 channels (HSV), CV_32FC1=defines both the depth of each element and the number of channels.
            pixelMat.at<float>(0, 0) = pixel[0];
            pixelMat.at<float>(0, 1) = pixel[1];
            pixelMat.at<float>(0, 2) = pixel[2];
            // https://docs.opencv.org/3.1.0/dd/de1/classcv_1_1ml_1_1KNearest.html
            classifier->predict(pixelMat, label);
            if (label.at<float>(0, 0)) {
                mask.at<uchar>(r, c) = 1;
            }
        }
    }
    mask = mask * 255; // 0-1 -> 0-255

    // Remove noise (opening)
    erode(mask, mask, Mat(), Point(-1, -1), ML_OPENING_ITER);
    dilate(mask, mask, Mat(), Point(-1, -1), ML_OPENING_ITER);

    // Connect blobs (closing
    dilate(mask, mask, Mat(), Point(-1, -1), ML_CLOSING_ITER);
    erode(mask, mask, Mat(), Point(-1, -1), ML_CLOSING_ITER);

    // Map mask and show result
    strawberryImg.copyTo(result, mask);
    imshow("Mask " + classifier->getDefaultName(), mask);
    imshow("Result " + classifier->getDefaultName(), result);
    waitKey(0);
}