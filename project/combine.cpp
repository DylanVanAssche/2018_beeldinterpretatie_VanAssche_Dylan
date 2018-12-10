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

// std::sort helper function
bool sortNotesBySmallestPositionFirst(const Note &a, const Note &b) {
    return a.position < b.position; // smallest first
}

/*
 * Private function to retrieve the note frequency by index of the rectangles in convertDataToNote()
 * Since we use the points of the bounding box, the tone height is always shifted with 1 in it's index
 * A better option would be to locate the center of the blob instead of the bounding box points.
 * This could be done with a SimpleBlobDetector and a ROI based on the orientation of the note which we already know.
 *
 * @param int index
 * @return double frequency
 * @author Dylan Van Assche
 */
double _convertIndexToNoteFrequency(int index) {
    // index == 0 -> area before is ignored in this POC, every note there must stay on index 0.
    if(index > 0) {
        index++;
    }

    switch(index) {
        case 0:
            return NOTE_G;
        case 1:
            return NOTE_F;
        case 2:
            return NOTE_E;
        case 3:
            return NOTE_D;
        case 4:
            return NOTE_C;
        case 5:
            return NOTE_B;
        case 6:
            return NOTE_A;
        case 7:
            return NOTE_G;
        case 8:
            return NOTE_F;
        case 9:
            return NOTE_E;
        case 10: // index++ -> 10 becomes 11
        case 11:
            return NOTE_D;
        default:
            cerr << "Can't estimate note frequency, frequency is set to 0.0" << endl;
            return 0.0;
    }
}

/*
 * Combines the contours information and the distances between staff lines to find the frequency of each note.
 *
 * @param vector<ContoursData> data
 * @param vector<int> staffLineDistances
 * @returns vector<Note>
 * @author Dylan Van Assche
 */
vector<Note> convertDataToNote(Mat input, vector<ContoursData> data, vector<StaffLineData> staffLineDistances, int rows, int cols) {
    Mat drawing = Mat::zeros(rows, cols, CV_8UC3);
    input.copyTo(drawing);
    Mat img = input.clone();
    cvtColor(drawing, drawing, CV_GRAY2BGR);
    double frequency = NOTE_A; // fallback in case detection fails
    double length = NOTE_LENGTH; // fallback in case detection fails
    Point noteLocation;
    vector<Rect> areas;
    vector<Note> notes;

    /*
     * Create areas for the staff lines
     * https://www.muzieklerenlezen.nl/les-7-muzieknoten-een-naam-geven
     * The height (measured from below) = frequency from the note (inverted, in comparison to OpenCV XY coordinates!)
     *
     * |
     * |                                                            C
     * |                                                        B
     * |                                                    A
     * |                                                G
     * +--------------------------------------------F----------------------
     * |                                        E
     * +------------------------------------D------------------------------
     * |                                C
     * +----------------------------B--------------------------------------
     * |                        A
     * +--------------------G----------------------------------------------
     * |                F
     * +------------E------------------------------------------------------
     * |        D
     * |    C
     * |
     *
     * /!\ To keep this proof-of-concept simple, we will focus on the notes within the reach of the staff lines.
     *      Every note before or after the staff lines are ignored and reduced to NOTE_D or NOTE_G.
     *
     */

    if(staffLineDistances.size() < 2) {
        cerr << "Number of staff lines is too low to find the frequency: " << staffLineDistances.size() << endl;
        return notes;
    }

    Scalar color1 = Scalar(0, 255, 0);
    Scalar color2 = Scalar(0, 0, 255);
    int distanceBetween = abs(staffLineDistances.at(0).position - staffLineDistances.at(1).position);

    // Before the first staff line
    Rect areaBefore = Rect(
            Point(0, 0),
            Point(cols, staffLineDistances.at(0).position - distanceBetween/4)
    );
    rectangle(drawing, areaBefore, color2);
    areas.push_back(areaBefore);

    for(int j=0; j < staffLineDistances.size(); ++j) {
        int yPosition = staffLineDistances.at(j).position;

        // Only area between when we are really between 2 staff lines
        if(j > 0) {
            distanceBetween = abs(staffLineDistances.at(j).position - staffLineDistances.at(j - 1).position);

            // Between staff lines
            Rect areaBetween = Rect(
                    Point(0, yPosition - 3*distanceBetween/4),
                    Point(cols, yPosition - distanceBetween/4)
            );
            rectangle(drawing, areaBetween, color2);
            areas.push_back(areaBetween);
        }

        // On a staff line
        Rect areaOn = Rect(
                Point(0, yPosition - distanceBetween/4),
                Point(cols, yPosition + distanceBetween/4)
        );
        rectangle(drawing, areaOn, color1);
        areas.push_back(areaOn);
    }

    // After the last staff line
    Rect areaAfter = Rect(
            Point(0, staffLineDistances.at(staffLineDistances.size() - 1).position + distanceBetween/4),
            Point(cols, rows)
    );
    areas.push_back(areaAfter);
    rectangle(drawing, areaAfter, color2);

    // Find for every note the frequency by checking it's location
    cout << "Note frequency: [";
    for(int d=0; d < data.size(); ++d) {
        for (int i = 0; i < data.at(d).orientation.size(); ++i) {
            Note note;
            noteLocation = data.at(d).orientation.at(i);
            circle(drawing, noteLocation, 3, Scalar(255, 0, 0), -1);

            // Check between/on which staff lines the note is sitting
            for (int a = 0; a < areas.size(); ++a) {
                if (areas.at(a).contains(noteLocation)) {
                    frequency = _convertIndexToNoteFrequency(a);
                    cout << frequency << "Hz, ";
                    break;
                }
            }

            // Get the length of the note
            length = data.at(d).length.at(i);

            // Merge the retrieved data
            note.frequency = frequency;
            note.length = length;
            note.position = noteLocation.x;
            notes.push_back(note);
        }
    }
    cout << "]" << endl;

    /*
     * Because of template matching, the order of the notes is dropped. We can retrieve it by sorting the notes based
     * on their position.
     */
    sort(notes.begin(), notes.end(), sortNotesBySmallestPositionFirst);

    cout << "Displaying matches notes and staff lines" << endl;
    imshow("Matching notes with staff lines", drawing);
    waitKey(0);

    return notes;
}