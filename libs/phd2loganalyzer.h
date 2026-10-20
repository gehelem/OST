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

        std::vector<Peak> topPeriods;              // RA worm spectrum, strongest first
        std::vector<Peak> spectrum;                 // full RA worm spectrum, for graphing
    };

    // maxPeaks: how many of the strongest RA periodic-error components to report.
    static Result Analyze(const QString &logFilePath, int maxPeaks = 5);
};
