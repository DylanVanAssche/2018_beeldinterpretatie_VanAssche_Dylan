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

    vector<Mat> objectImages;
    vector<Mat> inputImages;
    vector< Ptr<Feature2D> > detectors;
    Mat resultObjectImg, resultInputImg;
    detectors.push_back(ORB::create()); // Default parameters for every detector, you can change them if you want
    detectors.push_back(BRISK::create());
    detectors.push_back(AKAZE::create());

    for(int i=0; i < detectors.size(); i++) {
        resultObjectImg = drawKeypointsForDetector(objectImg, detectors.at(i));
        resultInputImg = drawKeypointsForDetector(inputImg, detectors.at(i));
        objectImages.push_back(resultObjectImg);
        inputImages.push_back(resultInputImg);
    }

    Mat objectImagesConcat = generateMultiHorizontalImage(objectImages);
    Mat inputImagesConcat = generateMultiHorizontalImage(inputImages);

    namedWindow("Object detectors ORB | BRISK | AKAZE", WINDOW_AUTOSIZE);
    namedWindow("Input detectors ORB | BRISK | AKAZE", WINDOW_AUTOSIZE);
    imshow("Object detectors ORB | BRISK | AKAZE", objectImagesConcat);
    imshow("Input detectors ORB | BRISK | AKAZE", inputImagesConcat);

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