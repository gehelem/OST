#pragma once

#include <QString>
#include <vector>

// Reads back a PHD2GuideLog file (see libs/phd2guidelog.h) and computes the
// same diagnostics PHD Log Viewer's Guiding Assistant window does: residual
// RMS/peak/elongation, RA/DEC drift, a polar-alignment-error estimate, and
// the dominant periods in the RA periodic error (worm) spectrum.
//
// Designed to be called repeatedly on a file still being written (a live
// guiding session) - it just reads whatever rows are flushed so far.
class PHD2LogAnalyzer
{
public:
    struct Peak
    {
        double periodS;
        double amplitudePx;
    };

    struct Result
    {
        bool ok = false;
        QString error;

        int frameCount = 0;
        double pixelScaleArcsecPerPx = 0;

        double rmsRaPx = 0, rmsDecPx = 0;
        double peakRaPx = 0, peakDecPx = 0;
        double elongation = 0;                    // 0 (round) .. 1 (elongated)
        double driftRaPxPerMin = 0, driftDecPxPerMin = 0;
        double polarAlignErrArcmin = 0;            // 0 if declination unknown

        // "Raw" undoes every applied guide pulse before the FFT, so it shows
        // the worm's true periodic error as if unguided (useful to read off
        // a PEC curve). "Corrected" FFTs the actual guided residual as-is,
        // so it shows what the guide loop left behind (useful to judge how
        // well it's compensating). topPeriods is the raw spectrum's
        // strongest components; both full spectra are kept for graphing.
        std::vector<Peak> topPeriods;
        std::vector<Peak> spectrumRaw;
        std::vector<Peak> spectrumCorrected;
    };

    // maxPeaks: how many of the strongest RA periodic-error components to report.
    static Result Analyze(const QString &logFilePath, int maxPeaks = 5);
};
