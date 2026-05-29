#include "focus.h"
#include "polynomialfit.h"
#include "version.cc"
#include <cmath>
#include <algorithm>
#include <limits>
#include <QPainter>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>

Focus *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Focus *basemodule = new Focus(name, label, profile, availableModuleLibs);
    return basemodule;
}

Focus::Focus(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)

{
    Q_INIT_RESOURCE(focus);

    loadOstPropertiesFromFile(":focus.json");

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Focus (scxml)");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "focus");

    pMachine = QScxmlStateMachine::fromFile(":focus.scxml");
    setupMachineConnections();



    _startpos =          getEltInt("parameters", "startpos")->value();
    _steps =             getEltInt("parameters", "steps")->value();
    _iterations =        getEltInt("parameters", "iterations")->value();
    _loopIterations =    getEltInt("parameters", "loopIterations")->value();
    _backlash =          getEltInt("parameters", "backlash")->value();

    defineMeAsFocuser();
    giveMeADevice("camera", "Camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("focuser", "Focuser", INDI::BaseDevice::FOCUSER_INTERFACE);
    giveMeADevice("filter", "Filter wheel", INDI::BaseDevice::FILTER_INTERFACE);

}

Focus::~Focus()
{

}
void Focus::onExternalEvent(OST::ExtEvent event)
{

    // Handle external autofocus request from other modules (e.g., sequencer)
    //if (e.type == "requestautofocus" && getModuleName() == e.module)
    //{
    //    logInfo("Autofocus requested by another module - starting");
    //    getProperty("actions")->setState(OST::Busy, true);
    //    startCoarse();
    //    return;
    //}

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "autofocus")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                getEltBool("actions", "abortfocus")->setValue(false, true);
                getProperty(event.prpkey)->setState(OST::Busy, true);
                startCoarse();
            }
        }
        if (event.eltkey == "abortfocus")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                getEltBool("actions", "autofocus")->setValue(false, true);
                getProperty(event.prpkey)->setState(OST::Ok, true);
                getEltPrg("progress", "global")->setPrgValue(0, true);
                getEltPrg("progress", "global")->setDynLabel("Aborted", true);
                getProperty("parms")->enable();
                getProperty("devices")->enable();
                getProperty("parameters")->enable();
                pMachine->submitEvent("abort");
                pMachine->stop();
            }
        }
        if (event.eltkey == "loop")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(false, true))
            {
                getProperty(event.prpkey)->setState(OST::Ok, true);
            }
        }
    }

}

void Focus::updateProperty(INDI::Property p)
{
    if (
        (QString(p.getDeviceName()) == getString("devices", "camera") )
        &&  (p.getState() == IPS_ALERT)
    )
    {
        emit cameraAlert();
        pMachine->submitEvent("cameraAlert");
    }
    if (
        (QString(p.getDeviceName()) == getString("devices", "focuser"))
        &&  (QString(p.getName())   == "ABS_FOCUS_POSITION")
    )
    {
        INDI::PropertyNumber n = p;
        getEltInt("values", "focpos")->setValue(n[0].value, true);

        if (n.getState() == IPS_OK && pMachine->isRunning())
        {
            pMachine->submitEvent("GotoBestDone");
            pMachine->submitEvent("BacklashBestDone");
            pMachine->submitEvent("BacklashDone");
            pMachine->submitEvent("GotoNextDone");
            pMachine->submitEvent("GotoStartDone");
            getEltFloat("results", "pos")->setValue(n[0].value, true);
        }
    }
    if (
        (QString(p.getDeviceName()) == getString("devices", "camera"))
        &&  (QString(p.getName())   == "CCD_FRAME_RESET")
        &&  (p.getState() == IPS_OK)
    )
    {
        INDI::PropertySwitch n = p;
        //sendMessage("FrameResetDone");
        if (pMachine->isRunning())
        {
            pMachine->submitEvent("FrameResetDone");
        }
    }
    if (QString(p.getName()) == "CCD1")
    {
        newBLOB(p);
    }

}

void Focus::newBLOB(INDI::PropertyBlob b)
{
    if
    (
        (QString(b.getDeviceName()) == getString("devices", "camera"))
    )
    {
        delete _image;
        _image = new fileio();
        _image->loadBlob(b, 64);
        getProperty("image")->setState(OST::Ok, true);

        QImage rawImage = _image->getRawQImage();
        rawImage.save( getWebroot() + "/" + getModuleName() + QString(b.getDeviceName()) + ".jpeg", "JPG", 100);
        OST::ImgData dta = _image->ImgStats();
        dta.mUrlJpeg = getModuleName() + QString(b.getDeviceName()) + ".jpeg";
        dta.mUrlFits = getModuleName() + QString(b.getDeviceName()) + ".FITS";
        getEltImg("image", "image")->setValue(dta, true);

        if (pMachine->isRunning())
        {
            pMachine->submitEvent("ExposureDone");
            pMachine->submitEvent("ExposureBestDone");
        }

    }
}

void Focus::SMAbort()
{
    pMachine->stop();
    getEltBool("actions", "autofocus")->setValue(false, false);
    getEltBool("actions", "abortfocus")->setValue(false, true);
}

void Focus::setupMachineConnections()
{
    pMachine->connectToState("RequestFrameReset",  QScxmlStateMachine::onEntry(this, &Focus::SMRequestFrameReset));
    pMachine->connectToState("RequestBacklash",    QScxmlStateMachine::onEntry(this, &Focus::SMRequestBacklash));
    pMachine->connectToState("RequestGotoStart",   QScxmlStateMachine::onEntry(this, &Focus::SMRequestGotoStart));
    pMachine->connectToState("RequestExposure",    QScxmlStateMachine::onEntry(this, &Focus::SMRequestExposure));
    pMachine->connectToState("FindStars",          QScxmlStateMachine::onEntry(this, &Focus::SMFindStars));
    pMachine->connectToState("Compute",            QScxmlStateMachine::onEntry(this, &Focus::SMCompute));
    pMachine->connectToState("RequestGotoNext",    QScxmlStateMachine::onEntry(this, &Focus::SMRequestGotoNext));
    pMachine->connectToState("RequestBacklashBest", QScxmlStateMachine::onEntry(this, &Focus::SMRequestBacklashBest));
    pMachine->connectToState("RequestGotoBest",    QScxmlStateMachine::onEntry(this, &Focus::SMRequestGotoBest));
    pMachine->connectToState("RequestExposureBest", QScxmlStateMachine::onEntry(this, &Focus::SMRequestExposureBest));
    pMachine->connectToState("ComputeResult",      QScxmlStateMachine::onEntry(this, &Focus::SMComputeResult));
    pMachine->connectToState("ComputeResult",      QScxmlStateMachine::onExit(this, &Focus::SMFocusDone));
    pMachine->connectToState("ComputeLoopFrame",   QScxmlStateMachine::onEntry(this, &Focus::SMComputeLoopFrame));
    pMachine->connectToState("InitLoopFrame",      QScxmlStateMachine::onEntry(this, &Focus::SMInitLoopFrame));
    pMachine->connectToState("FindStarsFinal",     QScxmlStateMachine::onEntry(this, &Focus::SMFindStars));
    pMachine->connectToState("Final_1",            QScxmlStateMachine::onEntry(this, &Focus::SMAbort));
}

void Focus::startCoarse()
{
    logInfo("Start focus");
    getEltBool("actions", "autofocus")->setValue(true, false);
    getEltBool("actions", "abortfocus")->setValue(false, true);
    getProperty("values")->clearGrid();
    getProperty("parms")->disable();
    getProperty("devices")->disable();
    getProperty("parameters")->disable();
    if (!isServerConnected()) connectIndi();
    connectDevice(getString("devices", "camera"));
    connectDevice(getString("devices", "focuser"));
    connectDevice(getString("devices", "filter"));
    setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
    if (getString("devices", "camera") == "CCD Simulator")
    {
        //sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
    }
    enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);

    _posvector.clear();
    _hfdvector.clear();
    _coefficients.clear();

    _zonePosvector.clear();
    _zoneHfdvector.clear();
    _zoneCoefficients.clear();


    _iteration = 0;
    _besthfr = 99;
    _bestposfit = 0;
    _khi = 0;
    _r2 = 0;
    _zoneBestposfit.clear();

    mZoning =  getInt("parameters", "zoning");
    getProperty("zones")->clearGrid();


    for (int i = 0; i < mZoning * mZoning; i++)
    {
        _zoneBestposfit.append(0);
    }

    _steps =             getEltInt("parameters", "steps")->value();
    _iterations =        getEltInt("parameters", "iterations")->value();
    if (getBool("parameters", "aroundinitial"))
    {
        double p = 0;
        if (!getModNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION", p))
        {
            pMachine->submitEvent("abort");
            return;
        }
        _startpos = p -  _steps * _iterations / 2;
    }
    else
    {
        _startpos =          getEltInt("parameters", "startpos")->value();
    }
    _loopIterations =    getEltInt("parameters", "loopIterations")->value();
    _backlash =          getEltInt("parameters", "backlash")->value();


    setStateEvent(OST::Busy, "focusing", "startfocus", "start focusing");
    delete pMachine;
    pMachine = QScxmlStateMachine::fromFile(":focus.scxml");
    setupMachineConnections();
    pMachine->init();
    pMachine->start();
    getEltPrg("progress", "global")->setPrgValue(0, true);
    getEltPrg("progress", "global")->setDynLabel("0/" + QString::number(_iterations), true);
}

void Focus::SMRequestFrameReset()
{
    //sendMessage("SMRequestFrameReset");

    if (!frameReset(getString("devices", "camera")))
    {
        usleep(1000);
        pMachine->submitEvent("FrameResetDone");
        return;
    }

}

void Focus::SMRequestBacklash()
{
    logInfo("Moving to start position - backlash %1", {_startpos - _backlash});
    if (!sendModNewNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION",
                          _startpos - _backlash))
    {
        pMachine->submitEvent("abort");
        return;
    }
    pMachine->submitEvent("RequestBacklashDone");

}

void Focus::SMRequestGotoStart()
{
    logInfo("Moving to start position %1", {_startpos});
    if (!sendModNewNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION", _startpos))
    {
        pMachine->submitEvent("abort");
        return;
    }
    pMachine->submitEvent("RequestGotoStartDone");

}

void Focus::SMRequestExposure()
{
    logInfo("Exposure %1", {getFloat("parms", "exposure")});
    if (!requestCapture(getString("devices", "camera"), getFloat("parms", "exposure"), getInt("parms", "gain"), getInt("parms",
                        "offset")))
    {
        emit abort();
        return;
    }
    pMachine->submitEvent("RequestExposureDone");

}

void Focus::SMFindStars()
{
    stats = _image->getStats();
    _solver.ResetSolver(stats, _image->getImageBuffer(), getInt("parameters", "zoning"));
    connect(&_solver, &Solver::successSEP, this, &Focus::OnSucessSEP);
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void Focus::OnSucessSEP()
{
    disconnect(&_solver, &Solver::successSEP, this, &Focus::OnSucessSEP);
    OST::ImgData dta = getEltImg("image", "image")->value();
    double ech = getSampling();
    dta.HFRavg = _solver.HFRavg * ech;
    dta.starsCount = _solver.stars.size();
    getEltImg("image", "image")->setValue(dta, true);
    getEltFloat("results", "hfr")->setValue(dta.HFRavg, true);

    if (_solver.stars.size() < 1 )
    {
        logError("We need at least 1 stars to  focus - abort");
        pMachine->submitEvent("abort");
        setStateEvent(OST::Error, "error", "nostars", "no stars");

        return;
    }
    logInfo("%1 stars found - HFR: %2", {dta.starsCount, dta.HFRavg });

    pMachine->submitEvent("FindStarsDone");
}

void Focus::SMCompute()
{
    //sendMessage("SMCompute");

    _posvector.push_back(_startpos + _iteration * _steps);
    _hfdvector.push_back(_loopHFRavg);

    for (int i = 0; i < mZoning * mZoning; i++)
    {
        if (_zoneloopHFRavg[i] != 99)
        {
            _zonePosvector[i].push_back(_startpos + _iteration * _steps);
            _zoneHfdvector[i].push_back(_zoneloopHFRavg[i]);
        }
    }

    double coeff[3], khi = 0;
    if (_posvector.size() > 2)
    {
        polynomialfit(_posvector.size(), 3, _posvector.data(), _hfdvector.data(), coeff, &khi);
        _bestposfit = -coeff[1] / (2 * coeff[2]);
        _khi = khi;

        int n = _posvector.size();
        double meanHFR = 0;
        for (double v : _hfdvector) meanHFR += v;
        meanHFR /= n;
        double tss = 0;
        for (double v : _hfdvector) tss += (v - meanHFR) * (v - meanHFR);
        _r2 = (tss > 0) ? (1.0 - khi / tss) : 1.0;
    }

    for (int i = 0; i < mZoning * mZoning; i++)
    {
        if (_zoneHfdvector[i].size() > 2)
        {
            double coeff[3], zkhi;
            polynomialfit(_posvector.size(), 3, _posvector.data(), _zoneHfdvector[i].data(), coeff, &zkhi);
            _zoneBestposfit[i] = -coeff[1] / (2 * coeff[2]);
        }
    }

    if ( _loopHFRavg < _besthfr )
    {
        _besthfr = _loopHFRavg;
        _bestpos = _startpos + _iteration * _steps;
    }

    getEltFloat("values", "loopHFRavg")->setValue(_loopHFRavg, false);
    getEltInt("values", "bestpos")->setValue(_bestpos, false);
    getEltFloat("values", "bestposfit")->setValue(_bestposfit, false);
    getEltInt("values", "focpos")->setValue(_startpos + _iteration * _steps, false);
    getEltInt("values", "iteration")->setValue(_iteration, false);
    getEltFloat("values", "khi")->setValue(khi, false);
    getEltFloat("values", "r2")->setValue(_r2, false);
    getEltFloat("values", "fitk0")->setValue(coeff[0], false);
    getEltFloat("values", "fitk1")->setValue(coeff[1], false);
    getEltFloat("values", "fitk2")->setValue(coeff[2], true);

    getStore()["values"]->push();
    getEltPrg("progress", "global")->setPrgValue(100 * _iteration / _iterations, false);
    getEltPrg("progress", "global")->setDynLabel(QString::number(_iteration + 1) + "/" + QString::number(_iterations), true);

    if (_iteration + 1 < _iterations )
    {
        _iteration++;
        pMachine->submitEvent("NextLoop");
    }
    else
    {
        pMachine->submitEvent("LoopFinished");
    }
}

void Focus::SMRequestGotoNext()
{
    int pos = _startpos + _iteration * _steps;
    logInfo("Moving to position  1 %1", {pos});
    if (!sendModNewNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION", pos))
    {
        pMachine->submitEvent("abort");
        return;
    }
    pMachine->submitEvent("RequestGotoNextDone");
}

void Focus::SMRequestBacklashBest()
{
    int pos = _bestpos - _backlash;
    logInfo("Moving to position 2 %1", {pos});
    if (!sendModNewNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION",
                          pos))
    {
        pMachine->submitEvent("abort");
        return;
    }
    pMachine->submitEvent("RequestBacklashBestDone");
}

void Focus::SMRequestGotoBest()
{
    int n = _posvector.size();

    // Validate polynomial fit quality before using _bestposfit
    bool fitValid = (n > 2);

    if (fitValid)
    {
        // Reject if R² is too low (poor parabolic fit)
        const double r2Threshold = 0.95;
        if (_r2 < r2Threshold)
        {
            logWarning("Polynomial fit unreliable (R²=%1 < %2) - using measured minimum", {_r2, r2Threshold});
            fitValid = false;
        }

        // Reject if vertex is outside the measured range (extrapolation is unreliable)
        double rangeMin = std::min(_startpos + 0.0, _startpos + (n - 1.0) * _steps);
        double rangeMax = std::max(_startpos + 0.0, _startpos + (n - 1.0) * _steps);
        if (_bestposfit < rangeMin || _bestposfit > rangeMax)
        {
            logWarning("Fit vertex %1 outside measured range [%2, %3] - using measured minimum", {_bestposfit, rangeMin, rangeMax});
            fitValid = false;
        }
        logInfo("Polynomial fit reliability (bad / 0.95 / good / 0.99 / excellent)  : R²=%1", {_r2});
    }

    if (!fitValid)
    {
        logWarning("Falling back to measured minimum position %1", {_bestpos});
        _bestposfit = _bestpos;
    }

    logInfo("Moving to best position %1", {(int)_bestposfit});
    if (!sendModNewNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION", int(_bestposfit)))
    {
        pMachine->submitEvent("abort");
        return;
    }
    pMachine->submitEvent("RequestGotoBestDone");
}

void Focus::SMRequestExposureBest()
{
    logInfo("Exposure %1", {getFloat("parms", "exposure")});
    if (!sendModNewNumber(getString("devices", "camera"), "CCD_EXPOSURE", "CCD_EXPOSURE_VALUE", getEltFloat("parms",
                          "exposure")->value()))
    {
        pMachine->submitEvent("abort");
        return;
    }
    double mFinalPos = 0;
    if (!getModNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION", mFinalPos))
    {
        pMachine->submitEvent("abort");
        return;
    }
    //getEltFloat("results", "pos")->setValue(mFinalPos, true);
    pMachine->submitEvent("RequestExposureBestDone");
}

void Focus::SMComputeResult()
{
    //sendMessage("SMComputeResult");
    double ech = getSampling();
    getEltFloat("values", "imgHFR")->setValue(_solver.HFRavg * ech, false);
    getEltFloat("values", "imgHFRdev")->setValue(_solver.HFRavgDev * ech, true);
    getEltFloat("results", "hfr")->setValue(_solver.HFRavg * ech, true);
    //getEltFloat("results", "pos")->setValue(int(_bestposfit), true);

    OST::ImgData dta = getEltImg("image", "image")->value();
    dta.HFRavg = _solver.HFRavg * ech;
    dta.HFRavgDev = _solver.HFRavgDev * ech;
    dta.starsCount = _solver.stars.size();
    getEltImg("image", "image")->setValue(dta, true);

    // what should i do here ?
    pMachine->submitEvent("ComputeResultDone");
    getEltPrg("progress", "global")->setPrgValue(100, true);
    getEltPrg("progress", "global")->setDynLabel("Finished", true);

    getProperty("zones")->clearGrid();

    for (int i = 0; i < mZoning * mZoning; i++)
    {
        if ((mZoning != 2) && (mZoning != 3))
        {
            getEltString("zones", "zone")->setValue(QString::number(i + 1), false);
        }
        if (mZoning == 2)
        {
            if (i == 0) getEltString("zones", "zone")->setValue("Upper left", false);
            if (i == 1) getEltString("zones", "zone")->setValue("Upper right", false);
            if (i == 2) getEltString("zones", "zone")->setValue("Lower left", false);
            if (i == 3) getEltString("zones", "zone")->setValue("Lower right", false);
        }
        if (mZoning == 3)
        {
            if (i == 0) getEltString("zones", "zone")->setValue("Upper left", false);
            if (i == 1) getEltString("zones", "zone")->setValue("Upper middle", false);
            if (i == 2) getEltString("zones", "zone")->setValue("Upper right", false);
            if (i == 3) getEltString("zones", "zone")->setValue("Midle left", false);
            if (i == 4) getEltString("zones", "zone")->setValue("Center", false);
            if (i == 5) getEltString("zones", "zone")->setValue("Middle right", false);
            if (i == 6) getEltString("zones", "zone")->setValue("Lower left", false);
            if (i == 7) getEltString("zones", "zone")->setValue("Lower middle", false);
            if (i == 8) getEltString("zones", "zone")->setValue("Lower right", false);
        }


        getEltFloat("zones", "bestpos")->setValue(_zoneBestposfit[i], false);
        getProperty("zones")->push();
    }

    getProperty("parms")->enable();
    getProperty("devices")->enable();
    getProperty("parameters")->enable();

    // Tilt heatmap image
    if (mZoning >= 2) {
        double minPos =  std::numeric_limits<double>::max();
        double maxPos = -std::numeric_limits<double>::max();
        for (int i = 0; i < mZoning * mZoning; i++) {
            if (_zoneBestposfit[i] != 0) {
                minPos = std::min(minPos, _zoneBestposfit[i]);
                maxPos = std::max(maxPos, _zoneBestposfit[i]);
            }
        }

        const int cellSize = 120;
        QImage tiltImage(mZoning * cellSize, mZoning * cellSize, QImage::Format_RGB32);
        tiltImage.fill(Qt::black);
        QPainter painter(&tiltImage);
        painter.setRenderHint(QPainter::Antialiasing, false);

        for (int row = 0; row < mZoning; row++) {
            for (int col = 0; col < mZoning; col++) {
                double val = _zoneBestposfit[row * mZoning + col];
                QRect cell(col * cellSize, row * cellSize, cellSize, cellSize);

                if (val == 0 || maxPos <= minPos) {
                    painter.fillRect(cell, QColor(80, 80, 80));
                } else {
                    double t = (val - minPos) / (maxPos - minPos);
                    // Blue (min) → Cyan → Green → Yellow → Red (max)
                    QColor color = QColor::fromHsvF(0.666 * (1.0 - t), 1.0, 1.0);
                    painter.fillRect(cell, color);
                }

                // Zone value label
                painter.setPen(Qt::white);
                QFont font = painter.font();
                font.setPixelSize(14);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(cell, Qt::AlignCenter, QString::number((int)val));

                // Cell border
                painter.setPen(QPen(Qt::black, 2));
                painter.drawRect(cell);
            }
        }
        painter.end();

        tiltImage.save(getWebroot() + "/" + getModuleName() + getString("devices", "camera") + "rawTilt.jpeg", "JPG", 100);
        logInfo("Tilt heatmap saved (range: %1 - %2 steps)", {(int)minPos, (int)maxPos});

        // Bilinear interpolation image
        // Zone (col, row) is treated as a control point at pixel center (col*cellSize + cellSize/2)
        // Invalid zones (bestposfit==0) fall back to _bestposfit
        auto zoneVal = [&](int col, int row) -> double {
            col = std::clamp(col, 0, mZoning - 1);
            row = std::clamp(row, 0, mZoning - 1);
            double v = _zoneBestposfit[row * mZoning + col];
            return (v != 0) ? v : _bestposfit;
        };

        const int iw = mZoning * cellSize;
        const int ih = mZoning * cellSize;
        QImage tiltInterp(iw, ih, QImage::Format_RGB32);

        for (int py = 0; py < ih; py++) {
            for (int px = 0; px < iw; px++) {
                // Map pixel center to zone-grid coordinates (zone centers at 0,1,...,mZoning-1)
                double zx = (px + 0.5) / cellSize - 0.5;
                double zy = (py + 0.5) / cellSize - 0.5;

                int c0 = (int)std::floor(zx);
                int r0 = (int)std::floor(zy);
                double fx = zx - c0;
                double fy = zy - r0;

                double v = (1-fx)*(1-fy) * zoneVal(c0,   r0)
                         +    fx *(1-fy) * zoneVal(c0+1, r0)
                         + (1-fx)*   fy  * zoneVal(c0,   r0+1)
                         +    fx *   fy  * zoneVal(c0+1, r0+1);

                double t = (maxPos > minPos) ? std::clamp((v - minPos) / (maxPos - minPos), 0.0, 1.0) : 0.5;
                tiltInterp.setPixel(px, py, QColor::fromHsvF(0.666 * (1.0 - t), 1.0, 1.0).rgb());
            }
        }

        // Overlay zone borders and values on the interpolated image
        QPainter painterInterp(&tiltInterp);
        painterInterp.setRenderHint(QPainter::Antialiasing, false);
        QFont font = painterInterp.font();
        font.setPixelSize(14);
        font.setBold(true);
        painterInterp.setFont(font);
        for (int row = 0; row < mZoning; row++) {
            for (int col = 0; col < mZoning; col++) {
                QRect cell(col * cellSize, row * cellSize, cellSize, cellSize);
                double val = _zoneBestposfit[row * mZoning + col];
                painterInterp.setPen(Qt::white);
                painterInterp.drawText(cell, Qt::AlignCenter, QString::number((int)val));
                painterInterp.setPen(QPen(QColor(0, 0, 0, 120), 1));
                painterInterp.drawRect(cell);
            }
        }
        painterInterp.end();

        tiltInterp.save(getWebroot() + "/" + getModuleName() + getString("devices", "camera") + "localTiltLinear.jpeg", "JPG", 100);

        // Global plane fit: z = c[0] + c[1]*col + c[2]*row  (least squares over all valid zones)
        QList<int> validIdx;
        for (int i = 0; i < mZoning * mZoning; i++)
            if (_zoneBestposfit[i] != 0) validIdx.append(i);

        if (validIdx.size() >= 3) {
            int n = validIdx.size();
            gsl_matrix *X   = gsl_matrix_alloc(n, 3);
            gsl_vector *zv  = gsl_vector_alloc(n);
            gsl_vector *c   = gsl_vector_alloc(3);
            gsl_matrix *cov = gsl_matrix_alloc(3, 3);
            double chisq;

            for (int k = 0; k < n; k++) {
                int col = validIdx[k] % mZoning;
                int row = validIdx[k] / mZoning;
                gsl_matrix_set(X, k, 0, 1.0);
                gsl_matrix_set(X, k, 1, col);
                gsl_matrix_set(X, k, 2, row);
                gsl_vector_set(zv, k, _zoneBestposfit[validIdx[k]]);
            }

            gsl_multifit_linear_workspace *ws = gsl_multifit_linear_alloc(n, 3);
            gsl_multifit_linear(X, zv, c, cov, &chisq, ws);

            double c0 = gsl_vector_get(c, 0);
            double c1 = gsl_vector_get(c, 1); // gradient X (steps/zone)
            double c2 = gsl_vector_get(c, 2); // gradient Y (steps/zone)
            logInfo("Global tilt plane: gradX=%1 steps/zone  gradY=%2 steps/zone", {c1, c2});

            // Color scale = plane range over the full image extent (corners)
            double zxMin = -0.5,               zxMax = mZoning - 0.5;
            double zyMin = -0.5,               zyMax = mZoning - 0.5;
            double planMin = std::min({c0 + c1*zxMin + c2*zyMin,
                                       c0 + c1*zxMax + c2*zyMin,
                                       c0 + c1*zxMin + c2*zyMax,
                                       c0 + c1*zxMax + c2*zyMax});
            double planMax = std::max({c0 + c1*zxMin + c2*zyMin,
                                       c0 + c1*zxMax + c2*zyMin,
                                       c0 + c1*zxMin + c2*zyMax,
                                       c0 + c1*zxMax + c2*zyMax});

            QImage globalTilt(iw, ih, QImage::Format_RGB32);
            for (int py = 0; py < ih; py++) {
                for (int px = 0; px < iw; px++) {
                    double zx = (px + 0.5) / cellSize - 0.5;
                    double zy = (py + 0.5) / cellSize - 0.5;
                    double v  = c0 + c1 * zx + c2 * zy;
                    double t  = (planMax > planMin) ? std::clamp((v - planMin) / (planMax - planMin), 0.0, 1.0) : 0.5;
                    globalTilt.setPixel(px, py, QColor::fromHsvF(0.666 * (1.0 - t), 1.0, 1.0).rgb());
                }
            }

            // Overlay zone borders, measured values and residuals
            QPainter painterGT(&globalTilt);
            font.setPixelSize(12);
            painterGT.setFont(font);
            for (int row = 0; row < mZoning; row++) {
                for (int col = 0; col < mZoning; col++) {
                    QRect cell(col * cellSize, row * cellSize, cellSize, cellSize);
                    double val      = _zoneBestposfit[row * mZoning + col];
                    double fitted   = c0 + c1 * col + c2 * row;
                    double residual = (val != 0) ? val - fitted : 0;
                    painterGT.setPen(Qt::white);
                    painterGT.drawText(cell.adjusted(0, 4, 0, -cell.height()/2),
                                       Qt::AlignCenter, QString::number((int)val));
                    painterGT.setPen(QColor(255, 220, 100));
                    painterGT.drawText(cell.adjusted(0, cell.height()/2, 0, -4),
                                       Qt::AlignCenter, (residual >= 0 ? "+" : "") + QString::number((int)residual));
                    painterGT.setPen(QPen(QColor(0, 0, 0, 120), 1));
                    painterGT.drawRect(cell);
                }
            }
            painterGT.end();

            globalTilt.save(getWebroot() + "/" + getModuleName() + getString("devices", "camera") + "globalTiltLinear.jpeg", "JPG", 100);

            gsl_multifit_linear_free(ws);
            gsl_matrix_free(X);
            gsl_vector_free(zv);
            gsl_vector_free(c);
            gsl_matrix_free(cov);
        }

        // Quadratic surface fit: z = c0 + c1*x + c2*y + c3*x² + c4*y² + c5*x*y
        if (validIdx.size() >= 6) {
            int n = validIdx.size();
            gsl_matrix *X   = gsl_matrix_alloc(n, 6);
            gsl_vector *zv  = gsl_vector_alloc(n);
            gsl_vector *c   = gsl_vector_alloc(6);
            gsl_matrix *cov = gsl_matrix_alloc(6, 6);
            double chisq;

            for (int k = 0; k < n; k++) {
                double x = validIdx[k] % mZoning;
                double y = validIdx[k] / mZoning;
                gsl_matrix_set(X, k, 0, 1.0);
                gsl_matrix_set(X, k, 1, x);
                gsl_matrix_set(X, k, 2, y);
                gsl_matrix_set(X, k, 3, x * x);
                gsl_matrix_set(X, k, 4, y * y);
                gsl_matrix_set(X, k, 5, x * y);
                gsl_vector_set(zv, k, _zoneBestposfit[validIdx[k]]);
            }

            gsl_multifit_linear_workspace *ws = gsl_multifit_linear_alloc(n, 6);
            gsl_multifit_linear(X, zv, c, cov, &chisq, ws);

            double c0 = gsl_vector_get(c, 0);
            double c1 = gsl_vector_get(c, 1);
            double c2 = gsl_vector_get(c, 2);
            double c3 = gsl_vector_get(c, 3);
            double c4 = gsl_vector_get(c, 4);
            double c5 = gsl_vector_get(c, 5);

            auto quadVal = [&](double x, double y) {
                return c0 + c1*x + c2*y + c3*x*x + c4*y*y + c5*x*y;
            };

            // Two-pass: first find min/max of the surface over all pixels
            double qMin =  std::numeric_limits<double>::max();
            double qMax = -std::numeric_limits<double>::max();
            for (int py = 0; py < ih; py++) {
                for (int px = 0; px < iw; px++) {
                    double v = quadVal((px + 0.5) / cellSize - 0.5, (py + 0.5) / cellSize - 0.5);
                    qMin = std::min(qMin, v);
                    qMax = std::max(qMax, v);
                }
            }

            QImage quadTilt(iw, ih, QImage::Format_RGB32);
            for (int py = 0; py < ih; py++) {
                for (int px = 0; px < iw; px++) {
                    double v = quadVal((px + 0.5) / cellSize - 0.5, (py + 0.5) / cellSize - 0.5);
                    double t = (qMax > qMin) ? std::clamp((v - qMin) / (qMax - qMin), 0.0, 1.0) : 0.5;
                    quadTilt.setPixel(px, py, QColor::fromHsvF(0.666 * (1.0 - t), 1.0, 1.0).rgb());
                }
            }

            // Overlay zone borders, measured values and residuals
            QPainter painterQ(&quadTilt);
            font.setPixelSize(12);
            painterQ.setFont(font);
            for (int row = 0; row < mZoning; row++) {
                for (int col = 0; col < mZoning; col++) {
                    QRect cell(col * cellSize, row * cellSize, cellSize, cellSize);
                    double val      = _zoneBestposfit[row * mZoning + col];
                    double fitted   = quadVal(col, row);
                    double residual = (val != 0) ? val - fitted : 0;
                    painterQ.setPen(Qt::white);
                    painterQ.drawText(cell.adjusted(0, 4, 0, -cell.height()/2),
                                      Qt::AlignCenter, QString::number((int)val));
                    painterQ.setPen(QColor(255, 220, 100));
                    painterQ.drawText(cell.adjusted(0, cell.height()/2, 0, -4),
                                      Qt::AlignCenter, (residual >= 0 ? "+" : "") + QString::number((int)residual));
                    painterQ.setPen(QPen(QColor(0, 0, 0, 120), 1));
                    painterQ.drawRect(cell);
                }
            }
            painterQ.end();

            quadTilt.save(getWebroot() + "/" + getModuleName() + getString("devices", "camera") + "globalTiltQuadratic.jpeg", "JPG", 100);

            gsl_multifit_linear_free(ws);
            gsl_matrix_free(X);
            gsl_vector_free(zv);
            gsl_vector_free(c);
            gsl_matrix_free(cov);
        }
    }
}




void Focus::SMInitLoopFrame()
{
    //sendMessage("SMInitLoopFrame");
    _loopIteration = 0;
    _loopHFRavg = 99;
    _zoneloopHFRavg.clear();
    _zoneLoopIteration.clear();
    for (int i = 0; i < mZoning * mZoning; i++)
    {
        _zoneloopHFRavg.append(99);
        _zoneLoopIteration.append(0);
        _zonePosvector.append(std::vector<double>());
        _zoneHfdvector.append(std::vector<double>());
    }
    getEltFloat("values", "loopHFRavg")->setValue(_loopHFRavg, true);

    pMachine->submitEvent("InitLoopFrameDone");

}

void Focus::SMComputeLoopFrame()
{
    //sendMessage("SMComputeLoopFrame");
    double ech = getSampling();
    _loopIteration++;
    _loopHFRavg = ((_loopIteration - 1) * _loopHFRavg + _solver.HFRavg * ech) / _loopIteration;
    for (int i = 0; i < mZoning * mZoning; i++)
    {
        if (_solver.HFRavgZone[i] != 99)
        {
            _zoneloopHFRavg[i] = (_zoneLoopIteration[i] * _zoneloopHFRavg[i] + _solver.HFRavgZone[i] * ech) /
                                 (_zoneLoopIteration[i] + 1);
            _zoneLoopIteration[i]++;

        }
    }
    getEltFloat("values", "loopHFRavg")->setValue(_loopHFRavg, true);
    getEltFloat("values", "imgHFR")->setValue(_solver.HFRavg, false);
    getEltFloat("values", "imgHFRdev")->setValue(_solver.HFRavgDev * ech, true);

    //qDebug() << "Loop    " << _loopIteration << "/" << _loopIterations << " = " <<  _solver.HFRavg;


    if (_loopIteration < _loopIterations )
    {
        pMachine->submitEvent("NextFrame");
    }
    else
    {
        pMachine->submitEvent("LoopFrameDone");
    }
}

void Focus::SMAlert()
{
    logError("SMAlert");
    setStateEvent(OST::Error, "error", "SMAlert", "SMAlert");
    pMachine->submitEvent("abort");
}

void Focus::SMFocusDone()
{
    double ech = getSampling();
    logInfo("Focus done");
    getEltFloat("results", "hfr")->setValue(_solver.HFRavg * ech, true);

    getProperty("actions")->setState(OST::Ok, true); // this will inform other modules
    getEltBool("actions", "autofocus")->setValue(false, true);
    pMachine->stop();
    setStateEvent(OST::Ok, "ready", "focusdone", "focus finished");

}
