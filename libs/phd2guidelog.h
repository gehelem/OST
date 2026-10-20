#pragma once

#include <QFile>
#include <QString>

// Writes a PHD2-compatible GuideLog, openable in PHD Log Viewer (and in any
// future in-house analysis tool built on the same format, e.g. FFT / drift
// plots a la https://github.com/agalasso/phdlogview). Shared across guiding
// modules (BlindPEC, Guider, ...) so the file format and session bookkeeping
// (frame counter, elapsed time) live in one place.
//
// Only a subset of PHD2's header is emitted - PHD Log Viewer tolerates the
// missing INFO / calibration / summary sections. What must be right: the
// "Guiding Begins at", the "Pixel scale =" line, the "Mount = ..." line with
// xAngle / xRate, the exact 18-column header, per-frame rows with Frame/Time
// reset per session, and the closing "Guiding Ends at".
//
// All guiding math (residuals, sign conventions, which axis is even guided)
// stays in the caller - this class only knows how to lay the result out in
// the PHD2 file format.
class PHD2GuideLog
{
public:
    struct HeaderInfo
    {
        QString equipmentProfile;                  ///< e.g. "BlindPEC", "Guider"
        QString mountName = "Mount";
        double  pixelScaleArcsecPerPx = 0;
        int     exposureMs = 0;
        double  xAngleDeg = 0, xRateArcsecS = 0;    ///< RA axis
        double  yAngleDeg = 0, yRateArcsecS = 0;    ///< DEC axis (= xAngle-90 / xRate when DEC isn't actually guided)
        double  minMoveX = 0, minMoveY = 0;
        QString xGuideAlgorithm = "Lead-compensated PI";
        QString yGuideAlgorithm = "None";
        int     maxRaDurationMs = 0, maxDecDurationMs = 0;
        QString decGuideMode = "Off";
        double  raGuideSpeedArcsecS = 0;
        double  mountRAHours = 0, mountDECDeg = 0;
        double  mountALTDeg = 0, mountAZDeg = 0;
    };

    struct Row
    {
        double  dx = 0, dy = 0;                     ///< camera-frame displacement, px
        double  raRawDistance = 0, decRawDistance = 0;
        double  raGuideDistance = 0, decGuideDistance = 0;
        int     raDurationMs = 0, decDurationMs = 0;
        QString raDirection;                        ///< "E" / "W" / empty
        QString decDirection;                        ///< "N" / "S" / empty
        double  starMass = 1000;
        double  snr = 0;
        int     errorCode = 0;
    };

    ~PHD2GuideLog();

    // webroot/<baseName>_<timestamp>.txt, e.g. baseName "PHD2_GuideLog".
    void open(const QString &webroot, const QString &baseName, const HeaderInfo &h);
    void writeRow(const Row &r);
    void close();
    bool isOpen() const;
    QString fileName() const;

private:
    QFile  _file;
    int    _frame = 0;
    qint64 _t0Ms  = 0;
};
