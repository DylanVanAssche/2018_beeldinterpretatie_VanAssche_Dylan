#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                             "{ help h usage ? | | Shows this message.}"
                             "{ object o       | | Loads a image of the object you want to track <REQUIRED> }"
                             "{ input i        | | Loads an input image to analyze<REQUIRED> }"
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
    string object(parser.get<string>("object"));
    string input(parser.get<string>("input"));
    if(object.empty() || input.empty())
    {
        cerr << "Please supply your images using command line arguments: --object=fitness_object.png and --input=fitness_image.png" << endl;
        return -1;
    }

    // Try to load images
    Mat objectImg;
    Mat inputImg; // BGR
    objectImg = imread(object, IMREAD_GRAYSCALE);
    inputImg = imread(input, IMREAD_GRAYSCALE);

    if(object.empty() || input.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -1;
    }

    // Displays the images in a window
    namedWindow("Object image", WINDOW_AUTOSIZE);
    namedWindow("Input image", WINDOW_AUTOSIZE);
    imshow("Object image", objectImg);
    imshow("Input image", inputImg);


    // Find keypoints using ORB
    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> orbKeypointsObject, orbKeypointsInput;
    Mat orbDescriptorsObject, ordDescriptorsInput;

    // Detect ORB features and compute descriptors.
    Mat orbObjectImg = objectImg.clone();
    Mat orbInputImg = inputImg.clone();
    Ptr<Feature2D> orb = ORB::create(500); // use trackbar for MAX FEATURES
    orb->detectAndCompute(orbObjectImg, Mat(), orbKeypointsObject, orbDescriptorsObject);
    orb->detectAndCompute(orbInputImg, Mat(), orbKeypointsInput, ordDescriptorsInput);
    drawKeypoints(orbObjectImg, orbKeypointsObject, orbObjectImg);
    drawKeypoints(orbInputImg, orbKeypointsInput, orbInputImg);

    namedWindow("Object image ORB", WINDOW_AUTOSIZE);
    namedWindow("Input image ORB", WINDOW_AUTOSIZE);
    imshow("Object image ORB", orbObjectImg);
    imshow("Input image ORB", orbInputImg);

    // Find keypoints using BRISK
    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> briskKeypointsObject, briskKeypointsInput;
    Mat briskDescriptorsObject, briskDescriptorsInput;

    // Detect ORB features and compute descriptors.
    Mat briskObjectImg = objectImg.clone();
    Mat briskInputImg = inputImg.clone();
    Ptr<Feature2D> brisk = BRISK::create(500); // use trackbar for MAX FEATURES
    orb->detectAndCompute(briskObjectImg, Mat(), briskKeypointsObject, briskDescriptorsObject);
    orb->detectAndCompute(briskInputImg, Mat(), briskKeypointsInput, briskDescriptorsInput);
    drawKeypoints(briskObjectImg, briskKeypointsObject, briskObjectImg);
    drawKeypoints(briskInputImg, briskKeypointsInput, briskInputImg);

    namedWindow("Object image BRISK", WINDOW_AUTOSIZE);
    namedWindow("Input image BRISK", WINDOW_AUTOSIZE);
    imshow("Object image BRISK", briskObjectImg);
    imshow("Input image BRISK", briskInputImg);

    // Find keypoints using AKAZE
    // Variables to store keypoints and descriptors
    std::vector<KeyPoint> akazeKeypointsObject, akazeKeypointsInput;
    Mat akazeDescriptorsObject, akazeDescriptorsInput;

    // Detect ORB features and compute descriptors.
    Mat akazeObjectImg = objectImg.clone();
    Mat akazeInputImg = inputImg.clone();
    Ptr<Feature2D> akaze = AKAZE::create(500); // use trackbar for MAX FEATURES
    orb->detectAndCompute(akazeObjectImg, Mat(), akazeKeypointsObject, akazeDescriptorsObject);
    orb->detectAndCompute(akazeInputImg, Mat(), akazeKeypointsInput, akazeDescriptorsInput);
    drawKeypoints(akazeObjectImg, akazeKeypointsObject, akazeObjectImg);
    drawKeypoints(akazeInputImg, akazeKeypointsInput, akazeInputImg);

    namedWindow("Object image AKAZE", WINDOW_AUTOSIZE);
    namedWindow("Input image AKAZE", WINDOW_AUTOSIZE);
    imshow("Object image AKAZE", akazeObjectImg);
    imshow("Input image AKAZE", akazeInputImg);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}