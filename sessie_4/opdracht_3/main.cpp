#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


// ORB can use NORM_L2
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
    Mat ransacObjectImg = inputImg.clone();
    Ptr<Feature2D> orb = ORB::create(500); // use trackbar for MAX FEATURES
    orb->detectAndCompute(orbObjectImg, Mat(), orbKeypointsObject, orbDescriptorsObject);
    orb->detectAndCompute(orbInputImg, Mat(), orbKeypointsInput, ordDescriptorsInput);
    drawKeypoints(orbObjectImg, orbKeypointsObject, orbObjectImg);
    drawKeypoints(orbInputImg, orbKeypointsInput, orbInputImg);

    namedWindow("Object image ORB keypoints", WINDOW_AUTOSIZE);
    namedWindow("Input image ORB keypoints", WINDOW_AUTOSIZE);
    imshow("Object image ORB keypoints", orbObjectImg);
    imshow("Input image ORB keypoints", orbInputImg);

    // Match features.
    std::vector<DMatch> matches;
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
    matcher->match(ordDescriptorsInput, orbDescriptorsObject, matches, Mat());

    // Sort matches by score
    std::sort(matches.begin(), matches.end());
    // Remove not so good matches
#define GOOD_MATCH_PERCENT 0.8 // trackbar
    const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
    matches.erase(matches.begin()+numGoodMatches, matches.end())

    // better is min/max since you can't be sure how much keypoints are returned for an image

    //min and max distance is also a solution for thresholding
    //for(int i=0)
    // zoek min and max distance
    // filter alle matches uit die > 3*min
    //

    // use perspectiveTransform

    // Draw top matches
    Mat orbMatches;
    drawMatches(orbInputImg, orbKeypointsInput, orbObjectImg, orbKeypointsObject, matches, orbMatches);

    namedWindow("Image ORB matches", WINDOW_AUTOSIZE);
    imshow("Image ORB matches", orbMatches);

    // Extract location of good matches
    std::vector<Point2f> pointsInput, pointsObject;

    for( int i = 0; i < matches.size(); i++ )
    {
        pointsInput.push_back(orbKeypointsInput[matches.at(i).queryIdx].pt ); // order depends on the matcher: first input, then object or reverse.
        pointsObject.push_back(orbKeypointsObject[matches.at(i).trainIdx].pt );
    }

    // Find homography using RANSAC
    Mat ransacHomography = findHomography(pointsInput, pointsObject, CV_RANSAC);

    // Use homography to warp image
    Mat ransacOutputImg;
    //warpPerspective(ransacObjectImg, ransacOutputImg, ransacHomography, cv::Size(2*ransacObjectImg.cols, 2*ransacObjectImg.rows));
    vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0);
    //perspectiveTransform()

    namedWindow("Image ORB RANSAC", WINDOW_AUTOSIZE);
    imshow("Image ORB RANSAC", ransacOutputImg);

    /*// Find keypoints using BRISK
    // Variables to store keypoints and descriptors
    int threshold=60; // trackbar?
    int octaves=4; //(pyramid layer) from which the keypoint has been extracted
    float patternScale=1.0f;
    std::vector<KeyPoint> briskKeypointsObject, briskKeypointsInput;
    Mat briskDescriptorsObject, briskDescriptorsInput;


    // Detect BRISK features and compute descriptors.
    Mat briskObjectImg = objectImg.clone();
    Mat briskInputImg = inputImg.clone();
    Ptr<Feature2D> brisk = BRISK::create(threshold, octaves, patternScale); // use trackbar for MAX FEATURES
    brisk->detectAndCompute(briskObjectImg, Mat(), briskKeypointsObject, briskDescriptorsObject);
    brisk->detectAndCompute(briskInputImg, Mat(), briskKeypointsInput, briskDescriptorsInput);
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

    // Detect AKAZE features and compute descriptors.
    Mat akazeObjectImg = objectImg.clone();
    Mat akazeInputImg = inputImg.clone();
    Ptr<Feature2D> akaze = AKAZE::create(); // use trackbar for MAX FEATURES
    akaze->detectAndCompute(akazeObjectImg, Mat(), akazeKeypointsObject, akazeDescriptorsObject);
    akaze->detectAndCompute(akazeInputImg, Mat(), akazeKeypointsInput, akazeDescriptorsInput);
    drawKeypoints(akazeObjectImg, akazeKeypointsObject, akazeObjectImg);
    drawKeypoints(akazeInputImg, akazeKeypointsInput, akazeInputImg);

    namedWindow("Object image AKAZE", WINDOW_AUTOSIZE);
    namedWindow("Input image AKAZE", WINDOW_AUTOSIZE);
    imshow("Object image AKAZE", akazeObjectImg);
    imshow("Input image AKAZE", akazeInputImg);*/

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}