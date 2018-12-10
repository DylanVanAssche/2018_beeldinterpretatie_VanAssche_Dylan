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
 * Generates a sine wave with a given frequency and length.
 * The result is a vector of shorts which can be written to a WAV file later.
 *
 * @param double frequency
 * @param double length
 * @returns vector<short> waveForm
 * @author Dylan Van Assche
 */
vector<short> generateWaveform(double frequency = NOTE_A, double length = NOTE_LENGTH) {
    vector<short> waveform;

    for(int i=0; i < length; i++) {
        double t = (double) i / WAVFILE_SAMPLES_PER_SECOND;
        waveform.push_back((short) (VOLUME * sin(2 * M_PI * frequency * t)));
    }

    return waveform;
}

/*
 * Writes a vector of waveforms to a WAV file using the WAVFile C library.
 * If the file can't be opened, this function returns and writes an error message to the console.
 *
 * @param string outputPath
 * @param vector< vector<short> > waveforms
 * @author Dylan Van Assche
 */
void saveWaveforms(string outputPath, vector< vector<short> > waveforms) {
    // Open WAV file
    FILE* f = wavfile_open(outputPath.c_str());
    if(!f)
    {
        cerr << "Opening sound file failed!" << endl;
        return;
    }

    // Write each waveform to the WAV file
    for(int w=0; w < waveforms.size(); ++w) {
        // Convert C++ vector to C array (https://stackoverflow.com/questions/1733143/converting-between-c-stdvector-and-c-array-without-copying)
        short* array = &waveforms.at(w)[0];
        std::copy(waveforms.at(w).begin(), waveforms.at(w).end(), array);
        wavfile_write(f, array, (int)waveforms.at(w).size());
    }

    // Close the WAV file
    wavfile_close(f);

    cout << "Saved WAV file as '" << outputPath << "'" << endl;
}

