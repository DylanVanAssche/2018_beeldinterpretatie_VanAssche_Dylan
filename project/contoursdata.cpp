/*
 * @title Labo beeldinterpretatie 2018: project
 * @author Dylan Van Assche
 *
 * ---> BASIC MUSIC NOTES RECOGNITION <---
 *
 * This Proof-Of-Concept (POC) can extract the music notes from a music sheet and save the sound of them into a
 * WAV audio file.
 *
 * Features:
 *  - Detect non-rotated 1/4 and 1/16 notes using template matching.
 *  - Find the tone height of each note using the staff lines extraction and vertical histograms.
 *  - Merge both into a music tone and save it to a WAV audio file using a WAV library.
 *
 * Usage:
 *  - cmake CMakeLists.txt
 *  - make
 *  - ./project --sheet=musicSheet.png --output=output.wav --quarter-note=quarter-note.png \
 *    --double-eighth-note=double-eighth-note.png
 *
 */
#include "notes.h"

/*
 * Calculates the contours of the input image. Afterwards, it finds the orientation of the notes in the image by using
 * the centroid of the image. Thanks to the blob on each note, the centroid will move towards the blob. This way we can
 * find the orientation of the note in an easy way.
 *
 * @param Mat input
 * @param NoteTemplate templ
 * @return ContoursData data
 * @author Dylan Van Assche
 */
ContoursData getContoursData(Mat input, NoteTemplate templ) {
    Mat img = input.clone();
    Mat matchResult;
    double minValue, maxValue;
    Point minLoc, maxLoc;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    vector<Point> orientation;
    vector<double> length ;

    // Perform template matching on the image (for each template)
    matchTemplate(img, templ.templ, matchResult, CV_TM_SQDIFF);

    // NCC convolution: borders don't have any information [0, 1]
    normalize(matchResult, matchResult, 0, 1, NORM_MINMAX, -1, Mat());

    // Convert MIN to MAX since best match TM_SQDIFF is minimum
    matchResult = 1 - matchResult;
    minMaxLoc(matchResult, &minValue, &maxValue, &minLoc, &maxLoc, Mat());

    // Create mask for multiple matching using a threshold match percentage
    Mat mask = Mat::zeros(Size(img.cols, img.rows), CV_32FC1);
    inRange(matchResult, maxValue * ((double) TEMPLATE_MATCH_PERCENTAGE / 100.0), maxValue, mask);
    mask.convertTo(mask, CV_8UC1);

    // Find contours
    findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    // Use contours to find the contours of all the matches
    findContours(mask, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);

    // Process every contour: bounding box,
    for(int i=0; i < contours.size(); ++i)
    {
        // Get the bounding box of the current contour
        vector<Point> hull;
        convexHull(contours.at(i), hull);
        Rect rect = boundingRect(hull);

        // Find local result for this rectangle
        Point loc;
        minMaxLoc(matchResult(rect), NULL, NULL, NULL, &loc);

        // Make this local result accessible to ROI later
        Point p1(loc.x + rect.x, loc.y + rect.y);
        Point p2(p1.x + templ.templ.cols, p1.y + templ.templ.rows);
        Rect box = Rect(p1, p2);

        // Find the centroid of the image by using OpenCV Moments in the ROI (=bouding box)
        Mat ROI = img(box);
        Moments m = moments(ROI,true);
        Point centroid(m.m10/m.m00 + box.x, m.m01/m.m00 + box.y);

        // Split bounding box in 2 parts to see where the blob of the note can be found.
        Rect upperBox = Rect(Point(box.x, box.y), Point(box.x + box.width, box.y + box.height/2));
        Rect lowerBox = Rect(Point(box.x, box.y + box.height/2), Point(box.x + box.width, box.y + box.height));

        // RECT.contains() provides an easy way to check if a Point is laying inside that rectangle
        if(lowerBox.contains(centroid)) {
            orientation.push_back(Point(box.x + box.width, box.y + box.height));
        }
        else if(upperBox.contains(centroid)) {
            orientation.push_back(Point(box.x, box.y));
        }
        else {
            cerr << "Centroid of the note lays outside the bounding box, this may not happen!" << endl;
            orientation.push_back(Point(-1, -1));
            continue;
        }

        // Keep length associated with the note
        length.push_back(templ.length);

        // Remove note from image for further processing (avoid double results with other templates)
        rectangle(img, box, Scalar::all(0), -1); // -1 for complete fill
    }

    imshow("Rectangle img", img);
    waitKey(0);

    // Combine the extracted data into a ContoursData struct
    ContoursData data;
    data.contours = contours;
    data.hierarchy = hierarchy;
    data.orientation = orientation;
    data.image = img;
    data.length = length;
    return data;
}

/*
 * Draws the contours using a random color generators and displays it.
 *
 * @param ContoursData data
 * @param int rows
 * @param int cols
 * @author Dylan Van Assche
 */
void drawContoursWithOrientation(ContoursData data, int rows, int cols) {
    RNG rng(RNG_INIT);
    Mat drawing = Mat::zeros(rows, cols, CV_8UC3);

    double toneHeight = 0;
    Point orientationPoint = Point(0, 0);
    Point bottomPoint = Point(0, 0);
    for(int i = 0; i < data.contours.size(); ++i) {
        Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        drawContours(drawing, data.contours, (int)i, color, 2, 8, data.hierarchy, 0, Point());

        orientationPoint = data.orientation.at(i);
        bottomPoint = Point(orientationPoint.x, drawing.rows);
        circle(drawing, orientationPoint, 5, Scalar( 0, 0, 255));
        line(drawing, orientationPoint, bottomPoint, Scalar(255, 255, 255));
        toneHeight = norm(orientationPoint - bottomPoint);
        cout << toneHeight << endl;
    }

    namedWindow("Contours notes", WINDOW_AUTOSIZE);
    imshow("Contours notes", drawing);
    waitKey(0);
}