#include <cmath>

 namespace DspCurves {
   static double freqdB(double index, double count, double sampleRate) {
        return (index + 0) * (sampleRate / count);
    }

    static double freqd(double index, double count, double sampleRate) {
        return (index + 1) * (sampleRate / count);
    }

   static double freq(int index, int count, int sampleRate) {
        return ((index + 1) * (sampleRate)) / count;
    }

    //plot for frequencies -fs/2 to fs/2
    static int frequency(int index, int countN, int sampleRate) {
        return index * (sampleRate / countN) - (sampleRate / 2);
    }

    //plot for frequencies -fs/2 to fs/2
    static double frequencied(double index, double countN, double sampleRate) {
        return index * (sampleRate / countN) - (sampleRate / 2.0);
    }

    static double myAWeight(double freq) {

        double f2 = freq * freq;
        double f4 = freq * freq * freq * freq;

        double c12200 = 12200.0 * 12200.0;
        double c206 = 20.6 * 20.6;
        double c177 = 107.7 * 107.7;
        double c7379 = 737.9 * 737.9;

        double Ra = c12200 * f4;
        Ra = Ra / ((f2 + c206) * (f2 + c12200) * sqrt(f2 + c177) *sqrt(f2 + c7379));

        return Ra;
    }

	static double aweight(double freq) {

        double f2 = freq * freq;
        double f4 = freq * freq * freq * freq;

        double fft = 10 * log(1.562339 * f4 / ((f2 + 107.65265 * 107.65265)
                * (f2 + 737.86223 * 737.86223))) / log(10)
                + 10.0 * log(2.242881E+16 * f4 / ((f2 + 20.598997 * 20.598997) * (f2 + 20.598997 * 20.598997)
                * (f2 + 12194.22 * 12194.22) * (f2 + 12194.22 * 12194.22))) / log(10);
       
        if (isnan(fft) ||isinf(fft)) {
            fft = 0;
        }
        return fft;
    }
}
