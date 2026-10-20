#include "phd2guidelog.h"

#include <QDateTime>

PHD2GuideLog::~PHD2GuideLog()
{
    close();
}

bool PHD2GuideLog::isOpen() const
{
    return _file.isOpen();
}

QString PHD2GuideLog::fileName() const
{
    return _file.fileName();
}

void PHD2GuideLog::open(const QString &webroot, const QString &baseName, const HeaderInfo &h)
{
    if (_file.isOpen())
        _file.close();

    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    const QString fname = webroot + "/" + baseName + "_" +
                           QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmmss") + ".txt";
    _file.setFileName(fname);
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;
    _frame = 0;
    _t0Ms  = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // C-locale number formatting (QString::number never localises; unlike arg()).
    auto n = [](double v, int p)
    {
        return QString::number(v, 'f', p);
    };

    QString s;
    s += "PHD2 version 2.6.13 [OST " + h.equipmentProfile + "], Log version 2.5. Log enabled at " + ts + "\n";
    s += "\n";
    s += "Guiding Begins at " + ts + "\n";
    s += "Equipment Profile = " + h.equipmentProfile + "\n";
    s += "Dither = RA only, Dither scale = 1.000, Image noise reduction = none, Guide-frame time lapse = 0, Server disabled\n";
    s += "Pixel scale = " + n(h.pixelScaleArcsecPerPx, 2) + " arc-sec/px, Binning = 1, Focal length = 800 mm\n";
    s += "Exposure = " + QString::number(h.exposureMs) + " ms\n";
    s += "Mount = \"" + h.mountName + "\", connected, guiding enabled, xAngle = " + n(h.xAngleDeg, 1) +
         ", xRate = " + n(h.xRateArcsecS, 3) + ", yAngle = " + n(h.yAngleDeg, 1) +
         ", yRate = " + n(h.yRateArcsecS, 3) + ", parity = +/+\n";
    s += "X guide algorithm = " + h.xGuideAlgorithm + ", Minimum move = " + n(h.minMoveX, 3) + "\n";
    s += "Y guide algorithm = " + h.yGuideAlgorithm + ", Minimum move = " + n(h.minMoveY, 3) + "\n";
    s += "Max RA duration = " + QString::number(h.maxRaDurationMs) +
         ", Max DEC duration = " + QString::number(h.maxDecDurationMs) +
         ", DEC guide mode = " + h.decGuideMode + "\n";
    s += "RA Guide Speed = " + n(h.raGuideSpeedArcsecS, 1) + " a-s/s, Dec Guide Speed = N/A, Cal Dec = 0.0, "
         "Last Cal Issue = None, Timestamp = " + ts + "\n";
    s += "RA = " + n(h.mountRAHours, 2) + " hr, Dec = " + n(h.mountDECDeg, 1) +
         " deg, Hour angle = N/A hr, Pier side = N/A, Rotator pos = N/A, Alt = " + n(h.mountALTDeg, 1) +
         " deg, Az = " + n(h.mountAZDeg, 1) + " deg\n";
    s += "Lock position = 0.000, 0.000, Star position = 0.000, 0.000, HFD = 2.00 px\n";
    s += "Frame,Time,mount,dx,dy,RARawDistance,DECRawDistance,RAGuideDistance,DECGuideDistance,"
         "RADuration,RADirection,DECDuration,DECDirection,XStep,YStep,StarMass,SNR,ErrorCode\n";

    _file.write(s.toUtf8());
    _file.flush();
}

void PHD2GuideLog::writeRow(const Row &r)
{
    if (!_file.isOpen())
        return;
    _frame++;

    auto n = [](double v, int p)
    {
        return QString::number(v, 'f', p);
    };

    const double tSec = (QDateTime::currentDateTime().toMSecsSinceEpoch() - _t0Ms) / 1000.0;

    QString row = QString::number(_frame) + "," + n(tSec, 3) + ",\"Mount\"," +
                  n(r.dx, 3) + "," + n(r.dy, 3) + "," +
                  n(r.raRawDistance, 3) + "," + n(r.decRawDistance, 3) + "," +
                  n(r.raGuideDistance, 3) + "," + n(r.decGuideDistance, 3) + "," +
                  QString::number(r.raDurationMs) + "," + r.raDirection + "," +
                  QString::number(r.decDurationMs) + "," + r.decDirection + ",,," +
                  n(r.starMass, 0) + "," + n(r.snr, 2) + "," +
                  QString::number(r.errorCode) + "\n";

    _file.write(row.toUtf8());
    _file.flush();
}

void PHD2GuideLog::close()
{
    if (!_file.isOpen())
        return;
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    _file.write(QString("Guiding Ends at " + ts + "\n\nLog closed at " + ts + "\n").toUtf8());
    _file.flush();
    _file.close();
}
