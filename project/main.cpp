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

int main(int argc, const char** argv) {
    CommandLineParser parser(argc, argv,
                 "{ help h usage ?                    | | Shows this message.                                       }"
                 "{ sheet s                           | | Loads an image of a music notes sheet <REQUIRED>          }"
                 "{ output o                          | | Path to the sound output file <REQUIRED>                  }"
                 "{ quarter-note quarter              | | Loads an image of a quarter note symbol <REQUIRED>        }"
                 "{ double-eighth-note double-eighth  | | Loads an image of a double-eighth note symbol <REQUIRED>  }"
    );

    // Help printing
    if(parser.has("help") || argc <= 1) {
        cerr << "Please use absolute paths when supplying your images." << endl;
        parser.printMessage();
        return 0;
    }

    // Parser fail
    if (!parser.check()) {
        parser.printErrors();
        return -1;
    }

    // Required arguments supplied?
    string sheet(parser.get<string>("sheet"));
    string outputSoundPath(parser.get<string>("output"));
    string quarterNote(parser.get<string>("quarter"));
    string doubleEighthNote(parser.get<string>("double-eighth"));
    if(sheet.empty() || outputSoundPath.empty() || quarterNote.empty() || doubleEighthNote.empty()) {
        cerr << "Please supply your parameters using command line arguments: "
        << "--sheet=sheet.png "
        << "--output=ouput.wav "
        << "--quarter-note=quarter-note.png "
        << "--double-eighth-note=double-eighth-note.png"
        << endl;
        return -2;
    }

    // Try to load images
    Mat sheetImg, quarterImg, doubleEighthImg;
    sheetImg = imread(sheet, IMREAD_GRAYSCALE);
    quarterImg = imread(quarterNote, IMREAD_GRAYSCALE);
    doubleEighthImg = imread(doubleEighthNote, IMREAD_GRAYSCALE);

    if(sheetImg.empty() || quarterImg.empty() || doubleEighthImg.empty()) {
        cerr << "Loading images failed, please verify the paths to the images." << endl;
        return -3;
    }

    // Displays the images in a window
    cout << "Displaying input" << endl;
    namedWindow("Sheet image", WINDOW_AUTOSIZE);
    imshow("Sheet image", sheetImg);
    namedWindow("Quarter note image", WINDOW_AUTOSIZE);
    imshow("Quarter note image", quarterImg);
    namedWindow("Double eighth note image", WINDOW_AUTOSIZE);
    imshow("Double eighth note image", doubleEighthImg);
    waitKey(0);

    // Associate the length of the note with each template
    NoteTemplate doubleEighthTempl;
    doubleEighthTempl.templ = doubleEighthImg;
    doubleEighthTempl.length = NOTE_LENGTH_16;
    NoteTemplate quarterTempl;
    quarterTempl.templ = quarterImg;
    quarterTempl.length = NOTE_LENGTH_4;

    // Split stafflines from input image
    NoteSheet noteSheet = splitStaffLinesAndNotes(sheetImg);
    imshow("Splitting notes", noteSheet.notes);
    imshow("Splitting stafflines", noteSheet.staffLines);
    waitKey(0);

    /*
     * Find contours and display them
     *
     * /!\ To keep this a proof of concept, only non rotated notes are detected.
     *     If you would like to detect notes that are drawn upside down, you can do the same steps as below
     *     but with your input image or template rotated by 180 degrees.
     */
    // Input image is inverted too
    doubleEighthImg = ~doubleEighthImg;
    quarterImg = ~quarterImg;

    // Find double eight notes
    ContoursData contoursDoubleEight = getContoursData(noteSheet.notes, doubleEighthTempl);
    drawContoursWithOrientation(contoursDoubleEight, sheetImg.rows, sheetImg.cols);

    // Find quarter notes (double eight notes are removed in the previous step)
    ContoursData contoursQuarter = getContoursData(contoursDoubleEight.image, quarterTempl);
    drawContoursWithOrientation(contoursQuarter, sheetImg.rows, sheetImg.cols);

    // Find the distances between the staff lines
    vector<StaffLineData> distances = getStaffLineDistances(noteSheet.staffLines);

    for(int d=0; d < distances.size(); ++d) {
        cout << "staff line position:" << distances.at(d).position << endl;
    }

    vector<ContoursData> data;
    data.push_back(contoursDoubleEight);
    data.push_back(contoursQuarter);
    vector<Note> notes = convertDataToNote(noteSheet.notes, data, distances, sheetImg.rows, sheetImg.cols);

    // Generate wave
    vector<vector<short> > waves;
    for(int n=0; n < notes.size(); ++n) {
        waves.push_back(generateWaveform(notes.at(n).frequency, notes.at(n).length));
    }
    saveWaveforms(outputSoundPath, waves);

    // Wait until the user decides to exit the program.
    return 0;
}