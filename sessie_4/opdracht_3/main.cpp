/*#include <iostream>
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

    // Find keypoints using BRISK
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
    imshow("Input image AKAZE", akazeInputImg);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}*/

#include "main.h"

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

    vector<Mat> objectImages, inputImages, matchImages;
    vector< Ptr<Feature2D> > detectors;
    Mat resultObjectImg, resultInputImg, resultMatchImg;
    detectors.push_back(ORB::create()); // Default parameters for every detector, you can change them if you want
    //detectors.push_back(BRISK::create()); // Not compatible with NORM_L2 brute force matching
    //detectors.push_back(AKAZE::create()); // Not compatible with NORM_L2 brute force matching

    for(int i=0; i < detectors.size(); i++) {
        resultObjectImg = drawKeypointsForDetector(objectImg, detectors.at(i));
        resultInputImg = drawKeypointsForDetector(inputImg, detectors.at(i));
        resultMatchImg = drawMatchesForDetector(inputImg, objectImg, detectors.at(i));
        objectImages.push_back(resultObjectImg);
        inputImages.push_back(resultInputImg);
        matchImages.push_back(resultMatchImg);
    }

    Mat objectImagesConcat = generateMultiHorizontalImage(objectImages);
    Mat inputImagesConcat = generateMultiHorizontalImage(inputImages);
    Mat matchImagesConcat = generateMultiHorizontalImage(matchImages);

    namedWindow("Object detectors ORB", WINDOW_AUTOSIZE);
    namedWindow("Input detectors ORB", WINDOW_AUTOSIZE);
    namedWindow("Matches detectors ORB", WINDOW_AUTOSIZE);
    imshow("Object detectors ORB", objectImagesConcat);
    imshow("Input detectors ORB", inputImagesConcat);
    imshow("Matches detectors ORB", matchImagesConcat);

    // Wait until the user decides to exit the program.
    waitKey(0);
    return 0;
}

/**
 * Draws the keypoints for a given detector on a new image and returns it.
 * @author Dylan Van Assche
 * @param Mat input
 * @param Ptr<Feature2D> detector
 * @return Mat output
 */
Mat drawKeypointsForDetector(Mat input, Ptr<Feature2D> detector) {
    vector<KeyPoint> keypoints;
    Mat output = input.clone(); // Keep original

    detector->detect(output, keypoints); // find keypoints
    drawKeypoints(output, keypoints, output);

    return output;
}

/**
 * Draws the matches for a given detector on a new image and returns it. Before drawing, the matches are filtered and
 * RANSAC is applied to remove any outliers.
 * @author Dylan Van Assche
 * @param Mat input
 * @param Mat object
 * @param Ptr<Feature2D> detector
 * @return Mat result
 */
Mat drawMatchesForDetector(Mat input, Mat object, Ptr<Feature2D> detector) {
    vector<KeyPoint> keypointsObject, keypointsInput;
    vector<DMatch> matches;
    Mat descriptorObject, descriptorInput;
    Mat copyInput = input.clone(); // Keep original
    Mat copyObject = object.clone(); // Keep original
    Mat result;

    detector->detectAndCompute(copyInput, Mat(), keypointsInput, descriptorInput); // find keypoints for input
    detector->detectAndCompute(copyObject, Mat(), keypointsObject, descriptorObject); // find keypoints for object

    BFMatcher matcher = BFMatcher(NORM_L2); // Eucledian distance is only compatible with ORB
    matcher.match(descriptorObject, descriptorInput, matches);

    // Filter matches by distance before applying RANSAC
    // https://docs.opencv.org/3.0-beta/doc/tutorials/features2d/feature_homography/feature_homography.html
    double distance, min, max;
    vector< DMatch > validMatches;

    // Find min and max distance
    for(int i=0; i < descriptorObject.rows; i++) {
        distance = matches.at(i).distance;

        // Avoid skipping the next for loops
        if(distance == 0) {
            continue;
        }

        // Valid distance, update
        if(distance < min) {
            min = distance;
        }
        if(distance > max) {
            max = distance;
        }
    }

    // Filter
    for(int i=0; i < descriptorObject.rows; i++ )
    {
        if(matches.at(i).distance < DISTANCE_MULTIPLIER * min)
        {
            validMatches.push_back(matches.at(i));
        }
    }

    // Show matches on result image
    drawMatches(copyObject, keypointsObject, copyInput, keypointsInput, validMatches, result);

    // Apply RANSAC to create a geometric model of the matches and filter out any outliers
    vector<Point2f> objectLoc;
    vector<Point2f> inputLoc;

    // Retrieve all the keypoints that were accepted by our filter
    for(int i=0; i < validMatches.size(); i++)
    {
        objectLoc.push_back(keypointsObject[validMatches.at(i).queryIdx].pt);
        inputLoc.push_back(keypointsInput[validMatches.at(i).trainIdx].pt);
    }

    Mat H = findHomography(objectLoc, inputLoc, RANSAC);

    // Find the corners of the object image
    vector<Point2f> sceneCorners(4);
    vector<Point2f> objectCorners(4);
    objectCorners[0] = cvPoint(0,0);
    objectCorners[1] = cvPoint(copyObject.cols, 0 );
    objectCorners[2] = cvPoint(copyObject.cols, copyObject.rows);
    objectCorners[3] = cvPoint(0, copyObject.rows);


    // Transform from object to scene using the given RANSAC homography
    perspectiveTransform(objectCorners, sceneCorners, H);

    // Draw a box around the scene location after transforming
    RNG rng(RNG_INIT);
    Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
    line(result, sceneCorners[0] + Point2f(copyObject.cols, 0), sceneCorners[1] + Point2f(copyObject.cols, 0), color, 4);
    line(result, sceneCorners[1] + Point2f(copyObject.cols, 0), sceneCorners[2] + Point2f(copyObject.cols, 0), color, 4);
    line(result, sceneCorners[2] + Point2f(copyObject.cols, 0), sceneCorners[3] + Point2f(copyObject.cols, 0), color, 4);
    line(result, sceneCorners[3] + Point2f(copyObject.cols, 0), sceneCorners[0] + Point2f(copyObject.cols, 0), color, 4);

    return result;
}

/**
 * Horizontally concats a vector of Mat objects to each other and returns the result.
 * @author Dylan Van Assche
 * @param vector<Mat> images
 * @return Mat result
 */
Mat generateMultiHorizontalImage(vector<Mat> images) {
    Mat result;

    // No empty vectors are allowed
    if(images.size() < 0) {
        return result;
    }

    // Save the first image without modifications
    result = images.at(0).clone();

    // Concat the rest of the images with the first one
    for(int i=1; i < images.size(); i++) {
        hconcat(result, images.at(i).clone(), result);
    }
    return result;
}