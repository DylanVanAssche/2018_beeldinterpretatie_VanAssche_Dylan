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
    cout << "Found " << matches.size() << " matches using BFMatcher" << endl;

    /*
     * Filter matches by distance before applying RANSAC
     * https://docs.opencv.org/3.0-beta/doc/tutorials/features2d/feature_homography/feature_homography.html
     */
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

    // Filter on distance
    for(int i=0; i < descriptorObject.rows; i++ )
    {
        if(matches.at(i).distance < DISTANCE_MULTIPLIER * (max-min) + min) // min + DISTANCE_MULTIPLIER * difference
        {
            validMatches.push_back(matches.at(i));
        }
    }

    // Show matches on result image
    drawMatches(copyObject, keypointsObject, copyInput, keypointsInput, validMatches, result);

    /*
     * Apply RANSAC to create a geometric model of the matches and filter out any outliers
     * This will make sure that the box is drawn around the object with most matches.
     * FUTURE WORK: By removing the detection from vector<DMatch> validMatches and do everything again we can find
     * multiple matches.
     */
    vector<Point2f> objectLoc;
    vector<Point2f> inputLoc;

    // Retrieve all the keypoints that were accepted by our filter
    cout << "Found " << validMatches.size() << " valid matches after distance filtering" << endl;
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
    RNG rng(RNG_INIT); // Random color for the box
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