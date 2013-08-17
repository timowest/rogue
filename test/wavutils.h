#ifndef WAVUTILS
#define WAVUTILS

#define SR 44100.0
#define SIZE 44100
#define DOUBLE_SIZE 88200
#define CHANNELS 1

void write_wav(char* filename, float* buffer) {
    static const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    SndfileHandle outfile(filename, SFM_WRITE, FORMAT, CHANNELS, SR);
    if (outfile) {
        outfile.write(&buffer[0], SIZE);
    }
}

void write_wav(char* filename, float* left, float* right) {
    static const int FORMAT = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
    float buffer[DOUBLE_SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer[2 * i] = left[i];
        buffer[2 * i + 1] = right[i];
    }
    SndfileHandle outfile(filename, SFM_WRITE, FORMAT, 2, SR);
    if (outfile) {
        outfile.write(&buffer[0], SIZE);
    }
}

int count_clicks(float* buffer) {
    float threshold = 0.4f;
    float diff = 0.0f;
    float diff2 = 0.0f;
    float diff3 = 0.0f;
    int clicks = 0;
    for (int i = 1; i < SIZE; i++) {
        diff = buffer[i] - buffer[i - 1];
        // one sample click
        if (fabs(diff) > threshold && fabs(diff2) > threshold &&
            ((diff < 0.0f && diff2 > 0.0f) || (diff > 0.0f && diff2 < 0.0))) {
            clicks++;
        // two sample click
        } else  if (fabs(diff) > threshold && fabs(diff2) < 0.1f && fabs(diff3) > threshold &&
            ((diff < 0.0f && diff3 > 0.0f) || (diff > 0.0f && diff3 < 0.0))) {
            clicks++;
        }
        diff3 = diff2;
        diff2 = diff;
    }
    return clicks;
}

int count_resets(float* sync) {
    int resets = 0;
    for (int i = 0; i< SIZE; i++) {
        if (sync[i] >= 0.0) resets++;
    }
    return resets;
}

#endif
