#include "allsky.h"
#include <QPainter>
#include <libnova/solar.h>
#include <libnova/julian_day.h>
#include <libnova/rise_set.h>
#include <libnova/transform.h>
#include <algorithm>
#include "version.cc"

// Write to a .tmp file then rename() atomically so nginx never serves a partially-written file.
static void atomicSaveJpeg(const QImage &img, const QString &finalPath)
{
    const QString tmp = finalPath + ".tmp";
    if (img.save(tmp, "JPG", 100))
        ::rename(tmp.toLocal8Bit().constData(), finalPath.toLocal8Bit().constData());
}

Allsky *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Allsky *basemodule = new Allsky(name, label, profile, availableModuleLibs);
    return basemodule;
}

Allsky::Allsky(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    giveMeAnActions();

    loadOstPropertiesFromFile(":allsky.json");
    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Allsky camera module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "default");


    giveMeADevice("camera", "Camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("gps", "GPS", INDI::BaseDevice::GPS_INTERFACE);
    giveMeADevice("meteo", "Météo", INDI::BaseDevice::GENERAL_INTERFACE);
    defineMeAsSequencer();


    if (getString("devices", "camera") != "") connectDevice(getString("devices", "camera"));
    if (getString("devices", "gps") != "") connectDevice(getString("devices", "gps"));
    if (getString("devices", "meteo") != "") connectDevice(getString("devices", "meteo"));


    OST::ElementBool* b = new OST::ElementBool("loop", "Play", "0", "");
    getProperty("actions")->addElt( b);
    b->setAutoUpdate(true);
    b = new OST::ElementBool("abort", "Stop", "2", "");
    getProperty("actions")->addElt( b);
    b->setAutoUpdate(true);
    b = new OST::ElementBool("pause", "Pause", "1", "");
    b->setAutoUpdate(true);
    getProperty("actions")->addElt(b);
    getProperty("actions")->deleteElt("startsequence");
    getProperty("actions")->deleteElt("abortsequence");
    getProperty("actions")->setElt("abort", true);
    getProperty("actions")->setRule(OST::SwitchsRule::OneOfMany);


    OST::ElementInt* i = new OST::ElementInt("delay", "Delay (s)", "0", "");
    i->setAutoUpdate(false);
    i->setDirectEdit(true);
    i->setValue(5);
    i->setSlider(OST::SliderAndValue, false);
    i->setMinMax(1, 120, false);
    i->setStep(1, false);
    getProperty("parms")->addElt( i);
    getEltFloat("parms", "exposure")->setAutoUpdate(false);

    _process = new QProcess(this);
    connect(_process, &QProcess::readyReadStandardOutput, this, &Allsky::processOutput);
    //connect(_process, &QProcess::readyReadStandardError, this, &Allsky::processError);
    connect(_process, &QProcess::errorOccurred, this, &Allsky::processError);
    connect(_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            &Allsky::processFinished);

    mScheduleTimer.setInterval(5000); // every 5s check
    connect(&mScheduleTimer, &QTimer::timeout, this, &Allsky::OnScheduleTimer);
    mScheduleTimer.start();

    getProperty("gps")->disable();
    getProperty("geogps")->disable();

    connectIndi();
    if (getString("devices", "camera") != "") connectDevice(getString("devices", "camera"));
    if (getString("devices", "gps") != "") connectDevice(getString("devices", "gps"));
    if (getString("devices", "meteo") != "") connectDevice(getString("devices", "meteo"));

    calculateSunset();

}

Allsky::~Allsky()
{
    //Q_CLEANUP_RESOURCE(dummy);
}

void Allsky::onAfterInit(void)
{
    checkArchives();
};

void Allsky::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        getProperty(event.prpkey)->setElt(event.eltkey, true, true);
        if (event.eltkey == "pause")
        {
            if (getBool(event.prpkey, event.eltkey)) enableParms(false);
            getProperty("actions")->setState(OST::Busy, true);
            //startTimelapseBatch();
        }
        if (event.eltkey == "loop")
        {
            if (getBool(event.prpkey, event.eltkey)) enableParms(false);
            getProperty("actions")->setState(OST::Busy, true);
            //startLoop();
        }
        if (event.eltkey == "abort")
        {
            if (getBool(event.prpkey, event.eltkey)) enableParms(true);
            getProperty("actions")->setState(OST::Ok, true);
            stopLoop();
        }
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "gps" && event.eltkey == "add")
    {
        addGPSLocalization();
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "daily" && event.eltkey == "enable")
    {
        //bool b = e.data[keyprop].toMap()["elements"].toMap()[keyelt].toBool();
        //should i disable here other prog ?
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "parms" && event.eltkey == "exposure")
    {
        double f = event.data[event.eltkey].toDouble();
        int  d = getInt("parms", "delay");
        if ( f > d )
        {
            logWarning("Delay must be greater than exposure, increasing delay");
            int step = getEltInt("parms", "delay")->step();
            int dStepped = step * (int)(f / step) + step;
            getEltInt("parms", "delay")->setValue(dStepped, true);
        }
        getEltFloat("parms", "exposure")->setValue(f, true);
    }

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "parms" && event.eltkey == "delay")
    {
        double d = event.data[event.eltkey].toInt();
        double  e = getFloat("parms", "exposure");
        if ( e > d )
        {
            logWarning("Delay must be greater than exposure, decreasing exposure value - 5%");
            getEltFloat("parms", "exposure")->setValue(d * 0.95, true);
        }
        getEltInt("parms", "delay")->setValue(d, true);
    }

    if (event.ev == OST::ExtEvType::GF && event.prpkey == "archives")
    {
        getProperty(event.prpkey)->fetchLine(event.line);
    }

    //    if (getModuleName() == e.module)
    //    {
    //        if (e.type == "afterinit")
    //        {
    //            checkArchives();
    //        }
    //        foreach(const QString &keyprop, e.data.keys())
    //        {
    //            if (e.type == "Flcreate" && keyprop == "geo")
    //            {
    //                getStore()[keyprop]->newLine(e.data[keyprop].toMap()["elements"].toMap());
    //            }
    //            if (e.type == "Fldelete" && keyprop == "geo")
    //            {
    //                double line = e.data[keyprop].toMap()["line"].toDouble();
    //                getStore()[keyprop]->deleteLine(line);
    //            }
    //            if (e.type == "Flupdate"  && keyprop == "geo")
    //            {
    //                double line = e.data[keyprop].toMap()["line"].toDouble();
    //                getStore()[keyprop]->updateLine(line, e.data[keyprop].toMap()["elements"].toMap());
    //            }
    //            if (e.type == "Flselect" && keyprop == "geo")
    //            {
    //                double line = e.data[keyprop].toMap()["line"].toDouble();
    //                QString id = getString("geo", "id", line);
    //                float lat = getFloat("geo", "lat", line);
    //                float lng = getFloat("geo", "long", line);
    //                getEltString("geo", "id")->setValue(id);
    //                getEltFloat("geo", "lat")->setValue(lat);
    //                getEltFloat("geo", "long")->setValue(lng);
    //            }
    //
    //        }
    //
    //    }
}
void Allsky::startLoop()
{
    if (mIsLooping)
    {
        logWarning("Already looping");
        return;
    }

    if (getBool("type", "manual")) logInfo("Start manual schedule");
    if (getBool("type", "fixed")) logInfo("Start fixed schedule");
    if (getBool("type", "sunset")) logInfo("Start sunset schedule");


    _index = 0;
    mKeog = QImage();
    mFolder = QDateTime::currentDateTime().toString("yyyyMMdd-hh-mm-ss");

    getProperty("log")->clearGrid();

    QDir dir0(getWebroot() + "/" + getModuleName() + "/" + mFolder + "/", {"*"});
    for(const QString &filename : dir0.entryList())
    {
        dir0.remove(filename);
    }

    QDir dir;
    dir.mkdir(getWebroot() + "/" + getModuleName());
    dir.mkdir(getWebroot() + "/" + getModuleName() + "/" + mFolder);
    dir.mkdir(getWebroot() + "/" + getModuleName() + "/" + mFolder + "/images");
    connectIndi();
    connectDevice(getString("devices", "camera"));
    setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
    enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);

    if (!setFocalLengthAndDiameter())
    {
        getProperty("actions")->setState(OST::Error, true);
        mIsLooping = false;
        return;
    }

    if (!requestCapture(getString("devices", "camera"), getFloat("parms", "exposure"), getInt("parms", "gain"), getInt("parms",
                        "offset")))
    {
        getProperty("actions")->setState(OST::Error, true);
        mIsLooping = false;
        return;
    }

    mTimer.setInterval(getInt("parms", "delay") * 1000);
    connect(&mTimer, &QTimer::timeout, this, &Allsky::OnTimer);
    mTimer.start();
    getProperty("actions")->setState(OST::Busy, true);
    mIsLooping = true;

}
void Allsky::stopLoop()
{
    if (!mIsLooping) return;

    if (getBool("type", "manual")) logInfo("Stop manual schedule");
    if (getBool("type", "fixed")) logInfo("Stop fixed schedule");
    if (getBool("type", "sunset")) logInfo("Stop sunset schedule");

    mTimer.stop();
    disconnect(&mTimer, &QTimer::timeout, this, &Allsky::OnTimer);

    mIsLooping = false;
    firstStack = true;
    startTimelapseBatch();
    getProperty("actions")->setState(OST::Ok, true);
}
void Allsky::startTimelapseBatch()
{
    logInfo("Generating timelapse");
    if (_process->state() != 0)
    {
        qDebug() << "can't start process";
    }
    else
    {
        QString program = "ffmpeg";
        QStringList arguments;
        arguments << "-framerate" << "30";
        arguments << "-pattern_type" << "glob";
        arguments << "-i" << getWebroot() + "/" + getModuleName() + "/" + mFolder +  "/images/*.jpeg";
        arguments << "-c:v" << "libx264";
        arguments << "-pix_fmt" << "yuv420p";
        arguments << "-framerate" << "30";
        arguments << "-y";
        arguments << getWebroot() + "/" + getModuleName() + "/" + mFolder + "/timelapse.mp4";
        qDebug() << "PROCESS ARGS " << arguments;
        _process->start(program, arguments);

    }

}
void Allsky::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    OST::VideoData v;
    v.url = getModuleName() + "/" + mFolder + "/timelapse.mp4";
    getEltVideo("timelapse", "video1")->setValue(v, true);
    logInfo("Timelapse ready (%1)", {QString::number(exitCode)});
    if (!mIsLooping) moveCurrentToArchives();
}
void Allsky::processOutput()
{
    QString output = _process->readAllStandardOutput();
    logDebug("Process output log : %1", {output});
}
void Allsky::processError(QProcess::ProcessError error)
{
    //QString output = _process->readAllStandardError();
    logWarning("Process error log : %1", {static_cast<int>(error)});
    logWarning("Timelapse failed");
    if (!mIsLooping) moveCurrentToArchives();

}
void Allsky::newBLOB(INDI::PropertyBlob pblob)
{
    if
    (
        (QString(pblob.getDeviceName()) == getString("devices", "camera"))
    )
    {
        getProperty("actions")->setState(OST::Ok, true);
        delete _image;
        _image = new fileio();
        _image->loadBlob(pblob, 64);

        QList<fileio::Record> rec = _image->getRecords();
        stats = _image->getStats();
        _image->saveAsFITS(getWebroot() + "/" + getModuleName() + QString(pblob.getDeviceName()) + ".FITS", stats,
                           _image->getImageBuffer(),
                           FITSImage::Solution(), rec, false);
        _index++;
        QImage rawImage = _image->getRawQImage();
        QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
        im.setColorTable(rawImage.colorTable());
        QRect r;
        r.setRect(rawImage.width() / 2, 1, 1, rawImage.height());
        if (firstStack)
        {
            imageStacked = im;
            firstStack = false;
        }
        else
        {
            for (int y = 0; y < im.height(); ++y)
            {
                QRgb *line_current = reinterpret_cast<QRgb*>(im.scanLine(y));
                QRgb *line_stacked = reinterpret_cast<QRgb*>(imageStacked.scanLine(y));
                for (int x = 0; x < im.width(); ++x)
                {
                    QRgb &rgb_current = line_current[x];
                    QRgb &rgb_stacked = line_stacked[x];
                    rgb_stacked = qRgb(
                                      std::max(qRed(rgb_current), qRed(rgb_stacked)),
                                      std::max(qGreen(rgb_current), qGreen(rgb_stacked)),
                                      std::max(qBlue(rgb_current), qBlue(rgb_stacked))
                                  );
                }
            }
        }
        atomicSaveJpeg(imageStacked, getWebroot() + "/" + getModuleName() + "/" + mFolder + "/stacked.jpeg");


        QImage image1 = mKeog;
        QImage image2 = im.copy(r);

        // Draw weather data overlay on the 1px band
        QPainter bandPainter(&image2);

        // Temperature: red point (range -15°C to +35°C)
        float tempKeog = getFloat("measures", "temp");
        // Always draw temperature (0°C is a valid value)
        float tempMin = -15.0;
        float tempMax = 35.0;
        float tempNormalized = (tempKeog - tempMin) / (tempMax - tempMin); // 0 to 1
        int tempY = image2.height() - (int)(tempNormalized * image2.height()); // Invert Y (0=top)

        // Clamp to image bounds
        if (tempY < 0) tempY = 0;
        if (tempY >= image2.height()) tempY = image2.height() - 1;

        bandPainter.setPen(QPen(QColor(255, 0, 0), 1)); // Red
        bandPainter.drawPoint(0, tempY);

        // Humidity: green point (range 0% to 100%)
        float humKeog = getFloat("measures", "hum");
        if (humKeog != 0)
        {
            float humNormalized = humKeog / 100.0; // 0 to 1
            int humY = image2.height() - (int)(humNormalized * image2.height());

            if (humY < 0) humY = 0;
            if (humY >= image2.height()) humY = image2.height() - 1;

            bandPainter.setPen(QPen(QColor(0, 255, 0), 1)); // Green
            bandPainter.drawPoint(0, humY);
        }

        // Pressure: blue point (range 950 to 1050 hPa)
        float pressKeog = getFloat("measures", "press");
        if (pressKeog != 0)
        {
            float pressMin = 950.0;
            float pressMax = 1050.0;
            float pressNormalized = (pressKeog - pressMin) / (pressMax - pressMin);
            int pressY = image2.height() - (int)(pressNormalized * image2.height());

            if (pressY < 0) pressY = 0;
            if (pressY >= image2.height()) pressY = image2.height() - 1;

            bandPainter.setPen(QPen(QColor(0, 0, 255), 1)); // Blue
            bandPainter.drawPoint(0, pressY);
        }

        bandPainter.end();

        QImage result(mKeog.width() + 1, rawImage.height(), QImage::Format_RGB32);
        QPainter painter(&result);
        painter.drawImage(0, 0, image1);
        painter.drawImage(mKeog.width(), 0, image2);
        mKeog = result;

        atomicSaveJpeg(mKeog, getWebroot() + "/" + getModuleName() + "/" + mFolder + "/keogram.jpeg");
        OST::ImgData keo;
        keo.mUrlJpeg = getModuleName() + "/" + mFolder + "/keogram" + ".jpeg";
        getEltImg("keogram", "image1")->setValue(keo, true);

        r.setRect(0, 0, im.width(), im.height() / 10);
        QPainter p;
        p.begin(&im);
        p.setPen(QPen(Qt::red));
        p.setFont(QFont("Times", 15, QFont::Bold));
        p.drawText(r, Qt::AlignLeft, QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss zzz") );

        // Display weather data below date/time with color-coded text (vertical layout)
        float temp = getFloat("measures", "temp");
        float hum = getFloat("measures", "hum");
        float press = getFloat("measures", "press");

        int lineHeight = 18; // Reduced line height for tighter spacing
        int yStart = im.height() / 10; // Start position below date/time
        int xPos = 5; // Left margin

        p.setFont(QFont("Times", 12, QFont::Bold)); // Slightly smaller font

        // Temperature in red (always displayed)
        p.setPen(QPen(QColor(255, 0, 0))); // Red
        QString tempText = QString("T: %1°C").arg(temp, 0, 'f', 1);
        p.drawText(xPos, yStart, tempText);

        // Humidity in green
        if (hum != 0)
        {
            p.setPen(QPen(QColor(0, 255, 0))); // Green
            QString humText = QString("H: %1%").arg(hum, 0, 'f', 0);
            p.drawText(xPos, yStart + lineHeight, humText);
        }

        // Pressure in blue
        if (press != 0)
        {
            p.setPen(QPen(QColor(0, 0, 255))); // Blue
            QString pressText = QString("P: %1 hPa").arg(press, 0, 'f', 0);
            p.drawText(xPos, yStart + 2 * lineHeight, pressText);
        }

        p.end();



        atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + QString(pblob.getDeviceName()) + ".jpeg");
        OST::ImgData dta = _image->ImgStats();
        dta.mUrlJpeg = getModuleName() + QString(pblob.getDeviceName()) + ".jpeg";
        dta.mAlternates.clear();
        dta.mAlternates.push_front(getModuleName() + "/" + mFolder + "/keogram" + ".jpeg");
        dta.mAlternates.push_front(getModuleName() + "/" + mFolder + "/stacked" + ".jpeg");
        getEltImg("image", "image")->setValue(dta, true);

        QString _n = QStringLiteral("%1").arg(_index, 10, 10, QLatin1Char('0'));
        im.save(getWebroot() + "/" + getModuleName() + "/" + mFolder + + "/images/" + _n + ".jpeg", "JPG", 100);

        getEltDateTime("log", "time")->setValue(QDateTime::currentDateTime(), false);
        getEltFloat("log", "snr")->setValue(_image->getStats().SNR, true);
        getProperty("log")->push();

        if (getBool("autoparms", "enabled"))
        {
            if (getString("autoparms", "measure") == "mean") computeExposureOrGain(_image->getStats().mean[0]);
            if (getString("autoparms", "measure") == "median") computeExposureOrGain(_image->getStats().median[0]);
        }

    }


}
void Allsky::updateProperty(INDI::Property property)
{
    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
    }
    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (!getProperty("gps")->isEnabled())
    )
    {
        getProperty("gps")->enable();
        getProperty("geogps")->enable();
    }

    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (property.getName()   == std::string("GEOGRAPHIC_COORD"))
        &&  (property.getState() == IPS_OK)
    )
    {
        INDI::PropertyNumber n = property;
        getEltFloat("geogps", "lat")->setValue(n[0].value, true);
        getEltFloat("geogps", "long")->setValue(n[1].value, true);
        getEltFloat("geogps", "elev")->setValue(n[2].value, true);
    }
    if (
        (property.getDeviceName() == getString("devices", "meteo"))
        &&  (property.getName()   == std::string("WEATHER_PARAMETERS"))
    )
    {
        getEltDateTime("history", "D")->setValue(QDateTime::currentDateTime(), true);
        INDI::PropertyNumber n = property;
        for (size_t i = 0; i < n.size(); i++)
        {
            if (n[i].getName() == std::string("WEATHER_TEMPERATURE"))
            {
                getEltFloat("measures", "temp")->setValue(n[i].value, true);
                getEltString("history", "S")->setValue("temp", false);
                getEltFloat("history", "Y")->setValue(n[i].value, false);
                getProperty("history")->push();
            }
            if (n[i].getName() == std::string("WEATHER_HUMIDITY"))
            {
                getEltFloat("measures", "hum")->setValue(n[i].value, true);
                getEltString("history", "S")->setValue("hum", false);
                getEltFloat("history", "Y")->setValue(n[i].value, false);
                getProperty("history")->push();
            }
            if (n[i].getName() == std::string("WEATHER_PRESSURE"))
            {
                getEltFloat("measures", "press")->setValue(n[i].value, true);
                getEltString("history", "S")->setValue("press", false);
                getEltFloat("history", "Y")->setValue(n[i].value, false);
                getProperty("history")->push();
            }
        }
    }
    if (
        (property.getDeviceName() == getString("devices", "gps"))
        &&  (property.getName()   == std::string("TIME_UTC"))
        &&  (property.getState() == IPS_OK)
    )
    {
        INDI::PropertyText t = property;
        QString time_format = "yyyy-MM-ddTHH:mm:ss";
        QString s = t[0].text;
        QDateTime d = QDateTime::fromString(s, time_format);
        getEltDate("geogps", "date")->setValue(d.date(), false);
        getEltTime("geogps", "time")->setValue(d.time(), false);
        getEltString("geogps", "offset")->setValue(t[1].text, true);
    }

}

void Allsky::OnTimer()
{
    if (getBool("actions", "abort") || getBool("actions", "pause")) return;
    if (!requestCapture(getString("devices", "camera"), getFloat("parms", "exposure"), getInt("parms", "gain"), getInt("parms",
                        "offset")))
    {
        getProperty("actions")->setState(OST::Error, true);
    }
    else
    {
        getProperty("actions")->setState(OST::Busy, true);
    }

}
void Allsky::OnScheduleTimer()
{
    calculateSunset();

    if (getBool("actions", "abort") || getBool("actions", "pause")) return;

    QTime now = QDateTime::currentDateTime().time();

    if (getBool("type", "sunset"))
    {
        QTime start = getTime("coming", "sunset"); // coucher
        QTime stop = getTime("coming", "sunrise"); // lever
        if (start < stop) // daytime requested
        {
            logError("sunrise > sunset ???");
        }
        if (start > stop) // nighttime requested
        {
            if (((now > start ) || (now < stop)) && (!mIsLooping))
            {
                logInfo("Start sunset schedule");
                startLoop();
            }
            if (((now < start ) && (now > stop)) && (mIsLooping))
            {
                logInfo("Stop sunset schedule");
                stopLoop();
            }
        }
        return;
    }

    if (getBool("type", "fixed"))
    {
        QTime start = getTime("daily", "begin");
        QTime stop = getTime("daily", "end");
        if (start == stop)
        {
            logError("Can't do anything when start time equals stop time, daily schedule disabled.");
            //getEltBool("daily", "enable")->setValue(false, true);
            return;
        }
        if (start < stop) // daytime requested
        {
            if ((now > start ) && (now < stop) && (!mIsLooping))
            {
                logInfo("Start daily schedule");
                startLoop();
            }
            if (((now < start ) || (now > stop)) && (mIsLooping))
            {
                logInfo("Stop daily schedule");
                stopLoop();
            }
        }
        if (start > stop) // nighttime requested
        {
            if (((now > start ) || (now < stop)) && (!mIsLooping))
            {
                logInfo("Start nightly schedule");
                startLoop();
            }
            if (((now < start ) && (now > stop)) && (mIsLooping))
            {
                logInfo("Stop nightly schedule");
                stopLoop();
            }
        }
        return;
    }

    if (getBool("type", "manual") && !mIsLooping)
    {
        startLoop();

    }
    if (!getBool("type", "manual") && mIsLooping)
    {
        stopLoop();
    }


}
void Allsky::computeExposureOrGain(double fromValue)
{
    QString elt = "";
    if (getString("autoparms", "expgain") == "exp") elt = "exposure";
    if (getString("autoparms", "expgain") == "gain") elt = "gain";
    double target = getFloat("autoparms", "target");
    double threshold = getFloat("autoparms", "threshold") / 100; // not used ATM, we'll see that later
    double coef = target / fromValue;
    double val = 0;
    if (elt == "exposure") val = getFloat("parms", elt);
    if (elt == "gain") val = getInt("parms", elt);

    int    delay = getInt("parms", "delay");
    double newval = val * coef;
    if (elt == "exposure" && newval > delay)
    {
        newval = 0.95 * delay;
    }
    if (newval < getFloat("autoparms", "min"))
    {
        newval = getFloat("autoparms", "min");
    }
    if (newval > getFloat("autoparms", "max"))
    {
        newval = getFloat("autoparms", "max");
    }
    if (elt == "exposure") getEltFloat("parms", elt)->setValue(newval, true);
    if (elt == "gain") getEltInt("parms", elt)->setValue(newval, true);



}
void Allsky::checkArchives(void)
{
    getProperty("archives")->clearGrid();
    QStringList folders;

    //check existing folders
    QDirIterator itFolders(getWebroot() + "/" + getModuleName() + "/archives", QDirIterator::Subdirectories);
    while (itFolders.hasNext())
    {
        QString d = itFolders.next();
        d.replace(getWebroot() + "/" + getModuleName() + "/archives", "");
        if (d.endsWith("/.") && !d.contains("images") && d != "/.")
        {
            QString dd = d;
            dd.replace("/.", "");
            if (!folders.contains(dd))
            {
                folders.append(dd);
            }
        }
    }

    folders.sort();
    for (const auto &f : folders)
    {
        QString dd = f;
        OST::ImgData i = getEltImg("archives", "keogram")->value();
        i.mUrlJpeg = getModuleName() + "/archives" + dd + "/keogram.jpeg";
        getEltImg("archives", "keogram")->setValue(i);
        i = getEltImg("archives", "stack")->value();
        i.mUrlJpeg = getModuleName() + "/archives" + dd + "/stacked.jpeg";
        getEltImg("archives", "stack")->setValue(i);
        OST::VideoData v = getEltVideo("archives", "timelapse")->value();
        v.url = getModuleName() + "/archives" + dd + "/timelapse.mp4";
        getEltVideo("archives", "timelapse")->setValue(v);
        getEltString("archives", "date")->setValue(dd.replace("/", ""));
        getProperty("archives")->push();
    }
}
void Allsky::moveCurrentToArchives(void)
{
    QDir dir;
    dir.mkdir(getWebroot() + "/" + getModuleName() + "/archives");
    dir.rename(getWebroot() + "/" + getModuleName() + "/" + mFolder,
               getWebroot() + "/" + getModuleName() + "/archives/" + mFolder);
    if (!getBool("keepimages", "enable"))
    {
        QDir dd(getWebroot() + "/" + getModuleName() + "/archives/" + mFolder + "/images");
        dd.removeRecursively();
    }

    checkArchives();
}
void Allsky::calculateSunset(void)
{
    double JD = ln_get_julian_from_sys();

    ln_rst_time rst;
    ln_zonedate rise, set, transit;
    ln_lnlat_posn observer;
    observer.lat = getFloat("geo", "lat");
    observer.lng = getFloat("geo", "long");
    if (ln_get_solar_rst(JD, &observer, &rst) != 0)
    {
        logError("Sun is circumpolar");
        return;
    }
    else
    {
        ln_get_local_date(rst.rise, &rise);
        ln_get_local_date(rst.transit, &transit);
        ln_get_local_date(rst.set, &set);
        QTime t;
        t.setHMS(rise.hours, rise.minutes, rise.seconds);
        getEltTime("coming", "sunrise")->setValue(t, false);
        t.setHMS(set.hours, set.minutes, set.seconds);
        getEltTime("coming", "sunset")->setValue(t, true);
    }

}
void Allsky::addGPSLocalization(void)
{
    getEltFloat("geo", "lat")->setValue(getFloat("geogps", "lat"), false);
    getEltFloat("geo", "long")->setValue(getFloat("geogps", "long"), false);
    getEltString("geo", "id")->setValue("GPS", false);
    getProperty("geo")->push();
}
void Allsky::enableParms(bool enable)
{
    if (enable)
    {
        getProperty("daily")->enable();
        getProperty("type")->enable();
    }
    else
    {
        getProperty("daily")->disable();
        getProperty("type")->disable();
    }
}
