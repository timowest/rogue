#include <fftw3.h>

int main() {
    const int width = 250;
    float* in = (float*) fftwf_malloc(sizeof(float) * width);
    float* out = (float*) fftwf_malloc(sizeof(float) * width);

    // fft
    fftwf_plan plan = fftwf_plan_r2r_1d(width, in, out, FFTW_R2HC, FFTW_MEASURE);
    fftwf_execute(plan);

    fftwf_destroy_plan(plan);
    fftwf_free(in);
    fftwf_free(out);
}
