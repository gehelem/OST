#include "inspector.h"
#include <QPainter>
#include <opencv2/opencv.hpp>
#include <cmath>
#include "version.cc"

static void atomicSaveJpeg(const QImage &img, const QString &finalPath)
{
    const QString tmp = finalPath + ".tmp";
    if (img.save(tmp, "JPG", 100))
        ::rename(tmp.toLocal8Bit().constData(), finalPath.toLocal8Bit().constData());
}

// Robust circle fit (Kasa's algebraic method) through a set of boundary
// points. Unlike a filled-area centroid/moments, this tolerates a partial
// arc -- it still recovers something close to the true center when the ring
// is reduced to a crescent (badly decollimated donut whose shadow eats into
// the outer boundary, breaking the usual "hole inside a ring" topology).
static bool fitCircleKasa(const std::vector<cv::Point> &pts, cv::Point2d &center)
{
    int const n = static_cast<int>(pts.size());
    if (n < 8)
        return false;

    cv::Mat A(n, 3, CV_64F);
    cv::Mat b(n, 1, CV_64F);
    for (int i = 0; i < n; i++)
    {
        double x = pts[i].x;
        double y = pts[i].y;
        A.at<double>(i, 0) = 2.0 * x;
        A.at<double>(i, 1) = 2.0 * y;
        A.at<double>(i, 2) = 1.0;
        b.at<double>(i, 0) = x * x + y * y;
    }
    cv::Mat sol;
    if (!cv::solve(A, b, sol, cv::DECOMP_SVD))
        return false;

    center.x = sol.at<double>(0, 0);
    center.y = sol.at<double>(1, 0);
    return true;
}

Inspector *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Inspector *basemodule = new Inspector(name, label, profile, availableModuleLibs);
    return basemodule;
}

Inspector::Inspector(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)

{
    loadOstPropertiesFromFile(":inspector.json");
    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Image inspector and collimation assistant module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "inspector");

    giveMeADevice("camera", "Camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("focuser", "Focuser", INDI::BaseDevice::FOCUSER_INTERFACE);

    giveMeAnActions();
    defineMeAsImager();

    OST::PropertyMulti *pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::Any);

    // Capture drivers.
    OST::ElementBool *b = new OST::ElementBool("shoot", "Shoot", "10", "");
    b->setValue(false, false);
    b->setPreIcon("photo_camera");
    pm->addElt(b);

    b = new OST::ElementBool("loop", "Loop", "20", "");
    b->setValue(false, false);
    b->setPreIcon("loop");
    pm->addElt(b);

    b = new OST::ElementBool("stop", "Stop", "30", "");
    b->setValue(false, false);
    b->setPreIcon("stop");
    pm->addElt(b);

    b = new OST::ElementBool("reload", "Reload", "40", "");
    b->setValue(false, false);
    pm->addElt(b);

    // Analysis selectors -- non-exclusive: each captured (or reloaded) frame is
    // run through the inspection pipeline and/or the collimation pipeline
    // depending on which of these is set.
    b = new OST::ElementBool("inspector", "Inspector analysis", "50", "");
    b->setValue(false, false);
    pm->addElt(b);

    b = new OST::ElementBool("collimator", "Collimator analysis", "60", "");
    b->setValue(false, false);
    pm->addElt(b);

    // Focuser helpers -- manual defocus for the collimation workflow.
    b = new OST::ElementBool("sethome", "Set home", "65", "");
    pm->addElt(b);

    b = new OST::ElementBool("gohome", "Go home", "70", "");
    pm->addElt(b);

    b = new OST::ElementBool("gointra", "Go intra", "80", "");
    pm->addElt(b);

    b = new OST::ElementBool("goextra", "Go extra", "90", "");
    pm->addElt(b);

    auto *i = new OST::ElementInt("cornersize", "Corner size (pixels)", "50", "");
    i->setAutoUpdate(true);
    i->setDirectEdit(true);
    i->setMinMax(0, 2000, false);
    i->setValue(200);
    i->setSlider(OST::SliderAndValue, false);
    getProperty("parms")->addElt(i);

    getEltLight("states", "idle")->setValue(OST::Ok, true);
    getEltLight("focushome", "defined")->setValue(OST::Idle, true);

    connectIndi();
    connectAllDevices();

    connect(this, &Inspector::newImage, this, &Inspector::OnNewImage);
}

Inspector::~Inspector()
{

}

bool Inspector::hasFocuser(void)
{
    return !getString("devices", "focuser").isEmpty();
}

void Inspector::onExternalEvent(OST::ExtEvent event)
{
    //qDebug() << "Inspector::onExternalEventBase event = " << OST::ExtEvToString(event.ev) << " p=" << event.prpkey << " e=" <<
    //         event.eltkey << " l=" << event.lovkey << " i=" << event.line;

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "shoot")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                mState = "single";
                getEltLight("states", "idle")->setValue(OST::Idle, false);
                initIndi();
                Shoot();
            }
        }
        if (event.eltkey == "loop")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                mState = "loop";
                getEltLight("states", "idle")->setValue(OST::Idle, false);
                initIndi();
                Shoot();
            }
        }
        if (event.eltkey == "reload")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                getEltBool("actions", "reload")->setValue(false, true);
                getProperty(event.prpkey)->setState(OST::Ok, true);
            }
        }
        if (event.eltkey == "stop")
        {
            getEltBool("actions", "loop")->setValue(false, false);
            getEltBool("actions", "stop")->setValue(false, true);
            emit Abort();
            mState = "idle";
            getEltLight("states", "shooting")->setValue(OST::Idle, false);
            getEltLight("states", "analyzing")->setValue(OST::Idle, false);
            getEltLight("states", "idle")->setValue(OST::Ok, true);
            getProperty("actions")->setState(OST::Ok, true);
        }
        // Persistent toggles: flip on every press.
        /*if (event.eltkey == "inspector")
            getEltBool("actions", "inspector")->setValue(!getBool("actions", "inspector"), true);
        if (event.eltkey == "collimator")
            getEltBool("actions", "collimator")->setValue(!getBool("actions", "collimator"), true);*/

        if (event.eltkey == "sethome")
        {
            getEltBool(event.prpkey, event.eltkey)->setValue(true, true);
            setHome();
            getEltBool(event.prpkey, event.eltkey)->setValue(false, true);
        }
        // Focuser move buttons: press now, and either release immediately (no
        // command sent) or keep pressed until ABS_FOCUS_POSITION settles, which
        // onUpdateProperty() watches for.
        if (event.eltkey == "gohome" || event.eltkey == "gointra" || event.eltkey == "goextra")
        {
            getEltBool(event.prpkey, event.eltkey)->setValue(true, true);
            bool sent = false;
            if (event.eltkey == "gohome")
                sent = goHome();
            else if (event.eltkey == "gointra")
                sent = goIntra();
            else
                sent = goExtra();

            if (sent)
                mFocuserButton = event.eltkey;
            else
                getEltBool(event.prpkey, event.eltkey)->setValue(false, true);
        }
    }

    if (event.ev == OST::ExtEvType::J2 && event.prpkey == "fileselect" && event.eltkey == "name")
    {
        getProperty("actions")->setState(OST::Ok, true);
        delete _image;
        _image = new fileio();
        logDebug(" file = %1", {getString("fileselect", "name")});
        _image->loadFits(getString("fileselect", "name"));
        _image->generateQImage();
        stats = _image->getStats();
        mState = "single";
        emit newImage();
    }
}

void Inspector::onUpdateProperty(INDI::Property property)
{
    // Focuser move tracking. Watch ONLY ABS_FOCUS_POSITION: while the focuser
    // is moving it stays BUSY, then goes back to OK (or ALERT on failure).
    // Release the pressed gohome/gointra/goextra button on that transition.
    // Done before the mState guard: the focuser buttons work while idle.
    if (!mFocuserButton.isEmpty()
            && QString(property.getDeviceName()) == getString("devices", "focuser")
            && QString(property.getName()) == "ABS_FOCUS_POSITION")
    {
        IPState st = property.getState();
        if (st == IPS_OK || st == IPS_ALERT)
        {
            getEltBool("actions", mFocuserButton)->setValue(false, true);
            getProperty("actions")->setState(st == IPS_OK ? OST::Ok : OST::Error, true);
            mFocuserButton.clear();
        }
    }

    if (mState == "idle") return;

    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
    }
    if (
        (property.getDeviceName() == getString("devices", "camera"))
        &&  (property.getState() == IPS_ALERT)
    )
    {
        logWarning("cameraAlert");
        emit cameraAlert();
    }

    if (
        (property.getDeviceName() == getString("devices", "camera"))
        &&  (property.getName()   == std::string("CCD_FRAME_RESET"))
        &&  (property.getState() == IPS_OK)
    )
    {
        //logInfo("FrameResetDone");
        emit FrameResetDone();
    }
}

void Inspector::newBLOB(INDI::PropertyBlob pblob)
{
    if (
        (QString(pblob.getDeviceName()) == getString("devices", "camera")) && (mState != "idle")
    )
    {
        getProperty("actions")->setState(OST::Ok, true);
        delete _image;
        _image = new fileio();
        _image->loadBlob(pblob, 64);
        // testing : load fits, comment previous and uncomment **2** lines below
        //_image->loadFits("/pathoftheimage/Light_LLL_008.fits");
        //_image->generateQImage();
        stats = _image->getStats();
        emit newImage();
    }
}

void Inspector::initIndi(void)
{
    connectIndi();
    connectDevice(getString("devices", "camera"));
    if (hasFocuser())
        connectDevice(getString("devices", "focuser"));
    setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
    if (getString("devices", "camera") == "CCD Simulator")
    {
        sendModNewNumber(getString("devices", "camera"), "SIMULATOR_SETTINGS", "SIM_TIME_FACTOR", 0.01 );
    }
    enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);
}

void Inspector::Shoot(void)
{
    if (!setFocalLengthAndDiameter())
    {
        getProperty("actions")->setState(OST::Error, true);
        finishSingle();
        return;
    }
    if (!requestCapture(getString("devices", "camera"), getFloat("parms", "exposure"), getInt("parms", "gain"),
                        getInt("parms", "offset")))
    {
        getProperty("actions")->setState(OST::Error, true);
        finishSingle();
        return;
    }
    getEltLight("states", "shooting")->setValue(OST::Busy, true);
    getProperty("actions")->setState(OST::Busy, true);
}

void Inspector::finishSingle(void)
{
    getEltBool("actions", "shoot")->setValue(false, true);
    getEltBool("actions", "stop")->setValue(false, true);
    getEltBool("actions", "loop")->setValue(false, true);
    getEltBool("actions", "reload")->setValue(false, true);
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "analyzing")->setValue(OST::Idle, false);
    getEltLight("states", "idle")->setValue(OST::Ok, true);
    getProperty("actions")->setState(OST::Ok, true);
    mState = "idle";
}

void Inspector::publishRawImage(void)
{
    if (!_image)
        return;
    QImage rawImage = _image->getRawQImage();
    QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
    im.setColorTable(rawImage.colorTable());
    atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + ".jpeg");

    OST::ImgData dta = _image->ImgStats();
    dta.mUrlJpeg = getModuleName() + ".jpeg";
    getEltImg("image", "image")->setValue(dta, true);
}

void Inspector::blankImage(const QString &prop)
{
    getEltImg(prop, "image")->setValue(OST::ImgData(), true);
}

void Inspector::OnNewImage()
{
    bool const doInspect = getBool("actions", "inspector");
    bool const doCollim  = getBool("actions", "collimator");

    // A selection change mid-loop must not leave a stale map on screen next to
    // a fresh one built from a different source frame: as soon as an analysis
    // is switched off, blank its result images.
    if (mPrevInspect && !doInspect)
    {
        blankImage("hfr");
        blankImage("shape");
        blankImage("corners");
    }
    if (mPrevCollim && !doCollim)
        blankImage("collim");
    mPrevInspect = doInspect;
    mPrevCollim = doCollim;

    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "analyzing")->setValue(OST::Busy, true);

    if (doCollim)
        analyzeFrame();

    if (doInspect)
    {
        // Asynchronous: StellarSolver emits successSEP -> OnSucessSEP(), which
        // publishes the maps and then continues the loop / ends the single shot.
        runInspection();
        return;
    }

    getEltLight("states", "analyzing")->setValue(OST::Idle, true);

    if (!doCollim)
        publishRawImage();

    if (mState == "loop")
        Shoot();
    else
        finishSingle();
}

void Inspector::runInspection(void)
{
    _solver.ResetSolver(stats, _image->getImageBuffer(), getInt("parameters", "zoning"));
    connect(&_solver, &Solver::successSEP, this, &Inspector::OnSucessSEP);
    _solver.FindStars(_solver.stellarSolverProfiles[0]);
}

void Inspector::OnSucessSEP()
{
    //qDebug() << "OnSucessSEP";

    getProperty("actions")->setState(OST::Ok, true);

    disconnect(&_solver, &Solver::successSEP, this, &Inspector::OnSucessSEP);


    //image->saveMapToJpeg(_webroot+"/"+_modulename+".jpeg",100,_solver.stars);
    QList<fileio::Record> rec = _image->getRecords();
    QImage rawImage = _image->getRawQImage();
    QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
    im.setColorTable(rawImage.colorTable());
    QImage imHFR = rawImage.convertToFormat(QImage::Format_ARGB32);
    //imHFR.setColorTable(rawImage.colorTable());
    QImage imShape = rawImage.convertToFormat(QImage::Format_RGB32);
    imShape.setColorTable(rawImage.colorTable());
    int imgWidth = _image->getStats().width;
    int imgHeight = _image->getStats().height;

    double ech = getSampling();

    /****************************************************************** HFR */
    /****************************************************************** HFR */
    /****************************************************************** HFR */
    /****************************************************************** HFR */

    /* smoothing avg 8 surround cell */
    QList<float> smoothedHFR = _solver.HFRavgZone;
    for (int line = 1; line < _solver.HFRZones - 1 ; line++)
    {
        for (int column = 1; column < _solver.HFRZones - 1; column++)
        {
            int i = _solver.HFRZones * line + column;
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 1];
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 0];
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column + 1];
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column - 1];
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column + 1];
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 1];
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 0];
            smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column + 1];
            smoothedHFR[i] = smoothedHFR[i] / 9;
        }
    }
    /* first row */
    for (int column = 1; column < _solver.HFRZones - 1; column++)
    {
        int line = 0;
        int i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] / 6;
    }
    /* last row */
    for (int column = 1; column < _solver.HFRZones - 1; column++)
    {
        int line = _solver.HFRZones - 1;
        int i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] / 6;
    }
    /* first column */
    for (int line = 1; line < _solver.HFRZones - 1 ; line++)
    {
        int column = 0;
        int i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] / 6;
    }
    /* last column */
    for (int line = 1; line < _solver.HFRZones - 1 ; line++)
    {
        int column = _solver.HFRZones - 1;
        int i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 0];
        smoothedHFR[i] = smoothedHFR[i] / 6;
    }
    /*corners*/
    {
        /* upper left */
        int line = 0;
        int column = 0;
        int i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column + 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] / 4;
        /* upper right */
        line = 0;
        column = _solver.HFRZones - 1;
        i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 0) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column + 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 1) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] / 4;
        /* lower left */
        line = _solver.HFRZones - 1;
        column = 0;
        i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column + 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 0) + column + 1];
        smoothedHFR[i] = smoothedHFR[i] / 4;
        /* lower right */
        line = _solver.HFRZones - 1;
        column = _solver.HFRZones - 1;
        i = _solver.HFRZones * line + column;
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column + 0];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line - 1) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] + _solver.HFRavgZone[_solver.HFRZones * (line + 0) + column - 1];
        smoothedHFR[i] = smoothedHFR[i] / 4;
    }
    /* calculate min & max HFR */
    float minHFR = 99;
    float maxHFR = 0;
    for (int i = 0; i < _solver.HFRZones * _solver.HFRZones ; i++)
    {
        if (smoothedHFR[i] < minHFR) minHFR = smoothedHFR[i];
        if ((smoothedHFR[i] > maxHFR) && (smoothedHFR[i] < 99)) maxHFR = smoothedHFR[i];
    }
    /*zoning*/
    QPainter p2;
    p2.begin(&imHFR);
    p2.setOpacity(0.5);
    for (int line = 0; line < _solver.HFRZones ; line++)
    {
        for (int column = 0; column < _solver.HFRZones; column++)
        {
            int zone = _solver.HFRZones * line + column;

            int x = (imgWidth / _solver.HFRZones) * (column) ;
            int y = (imgHeight / _solver.HFRZones) * (line) ;
            int dx = imgWidth / _solver.HFRZones;
            int dy = imgHeight / _solver.HFRZones;

            qreal alpha = 1 * (smoothedHFR[zone] - minHFR) / (maxHFR - minHFR);
            unsigned int r = 255 * (smoothedHFR[zone] - minHFR) / (maxHFR - minHFR);
            unsigned int g = 255 * (maxHFR - smoothedHFR[zone]) / (maxHFR - minHFR);
            p2.fillRect(QRect(x / 2, y / 2, dx / 2, dy / 2), qRgb(r, 0, 0));


        }
    }
    p2.end();

    /*surround stars*/
    p2.begin(&imHFR);
    p2.setPen(QPen(Qt::blue, 10));
    for( FITSImage::Star s : _solver.stars )
    {
        QPainter p2;
        int x = s.x;
        int y = s.y;
        int hfr = s.HFR;
        p2.drawEllipse(QPoint(x / 2, y / 2), hfr, hfr);
    }
    p2.end();




    /* HFR rectangle*/
    QPainter p;
    p.begin(&imHFR);
    p.setPen(QPen(Qt::red));
    int upperLeftI = 0;
    int lowerLeftI = 0;
    int upperRightI = 0;
    int lowerRightI = 0;
    upperLeftHFR = 0;
    lowerLeftHFR = 0;
    upperRightHFR = 0;
    lowerRightHFR = 0;
    for( FITSImage::Star s : _solver.stars )
    {
        if ( (s.x < (im.width() / 2)) && (s.y < (im.height() / 2) ))
        {
            upperLeftHFR = ( upperLeftI * upperLeftHFR + s.HFR * ech) / (upperLeftI + 1);
            upperLeftI++;
        }
        if ( (s.x > (im.width() / 2)) && (s.y < (im.height() / 2) ))
        {
            upperRightHFR = (upperRightI * upperRightHFR + s.HFR * ech ) / (upperRightI + 1);
            upperRightI++;
        }
        if ( (s.x < (im.width() / 2)) && (s.y > (im.height() / 2) ))
        {
            lowerLeftHFR = (lowerLeftI * lowerLeftHFR + s.HFR * ech) / (lowerLeftI + 1);
            lowerLeftI++;
        }
        if ( (s.x > (im.width() / 2)) && (s.y > (im.height() / 2) ))
        {
            lowerRightHFR = (lowerRightI * lowerRightHFR + s.HFR * ech) / (lowerRightI + 1);
            lowerRightI++;
        }
    };

    p.setPen(QPen(Qt::white));
    int mul = 200;
    QVector<QPointF> hexPoints;
    hexPoints << QPointF(1 * im.width() / 4 - mul * (upperLeftHFR - _solver.HFRavg * ech),
                         1 * im.height() / 4 - mul * (upperLeftHFR - _solver.HFRavg * ech));
    hexPoints << QPointF(3 * im.width() / 4 + mul * (upperRightHFR - _solver.HFRavg * ech),
                         1 * im.height() / 4 - mul * (upperRightHFR - _solver.HFRavg * ech));
    hexPoints << QPointF(3 * im.width() / 4 - mul * (lowerRightHFR - _solver.HFRavg * ech),
                         3 * im.height() / 4 + mul * (lowerRightHFR - _solver.HFRavg * ech));
    hexPoints << QPointF(1 * im.width() / 4 + mul * (lowerLeftHFR - _solver.HFRavg * ech),
                         3 * im.height() / 4 + mul * (lowerLeftHFR - _solver.HFRavg * ech));
    p.drawPolygon(hexPoints);
    p.setFont(QFont("Courrier", im.width() / 50, QFont::Normal));
    p.drawText(  QRect(0, 0, im.width(), im.height()), Qt::AlignCenter, QString::number(_solver.HFRavg * ech, 'f', 3) + "''");
    p.drawText(1 * im.width() / 4, 1 * im.height() / 4, QString::number(upperLeftHFR, 'f', 3) + "''");
    p.drawText(3 * im.width() / 4, 1 * im.height() / 4, QString::number(upperRightHFR, 'f', 3) + "''");
    p.drawText(1 * im.width() / 4, 3 * im.height() / 4, QString::number(lowerLeftHFR, 'f', 3) + "''");
    p.drawText(3 * im.width() / 4, 3 * im.height() / 4, QString::number(lowerRightHFR, 'f', 3) + "''");

    p.end();

    atomicSaveJpeg(imHFR, getWebroot() + "/" + getModuleName() + "HFR.jpeg");

    /****************************************************************** corners */
    /****************************************************************** corners */
    /****************************************************************** corners */
    /****************************************************************** corners */

    int h = rawImage.height();
    int w = rawImage.width();
    int sw = getInt("parms", "cornersize");
    int sh = sw * h / w;

    //QImage corners = QImage(3 * s, 3 * s, QImage::Format_RGB32);
    QImage corners = QImage(sw * 3, sh * 3, QImage::Format_RGB32);
    corners.setColorTable(rawImage.colorTable());
    corners.fill(Qt::green);
    QPainter painter(&corners);

    // drawImage(target, image,source)
    painter.drawImage(QRect(0 * sw, 0 * sh, sw, sh), rawImage, QRect(0, 0, sw, sh));               //upper left
    painter.drawImage(QRect(1 * sw, 0 * sh, sw, sh), rawImage, QRect(w / 2 - sw / 2, 0, sw, sh));  //upper middle
    painter.drawImage(QRect(2 * sw, 0 * sh, sw, sh), rawImage, QRect(w - sw, 0, sw, sh));          //upper right

    painter.drawImage(QRect(0 * sw, 1 * sh, sw, sh), rawImage, QRect(0, h / 2 - sw / 2, sw, sh)); //middle left
    painter.drawImage(QRect(1 * sw, 1 * sh, sw, sh), rawImage, QRect(w / 2 - sw / 2, h / 2 - sh / 2, sw, sh)); //middle middle
    painter.drawImage(QRect(2 * sw, 1 * sh, sw, sh), rawImage, QRect(w - sw, h / 2 - sh / 2, sw, sh)); //middle right

    painter.drawImage(QRect(0 * sw, 2 * sh, sw, sh), rawImage, QRect(0, h - sh, sw, sh)); //lower left
    painter.drawImage(QRect(1 * sw, 2 * sh, sw, sh), rawImage, QRect(w / 2 - sw / 2, h - sh, sw, sh)); //lower middle
    painter.drawImage(QRect(2 * sw, 2 * sh, sw, sh), rawImage, QRect(w - sw, h - sh, sw, sh)); //lower right

    painter.setPen(QPen(Qt::red));
    painter.drawRect(QRect(sw, 0, sw, 3 * sh - 1));
    painter.drawRect(QRect(0, sh, 3 * sw - 1, sh));
    painter.drawRect(QRect(0, 0, 3 * sw - 1, 3 * sh - 1));

    painter.end();
    atomicSaveJpeg(corners, getWebroot() + "/" + getModuleName() + "corners.jpeg");
    atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + ".jpeg");

    /****************************************************************** aberations */
    /****************************************************************** aberations */
    /****************************************************************** aberations */
    /****************************************************************** aberations */

    int ellipseSize = 0.20 * imgHeight / _solver.HFRZones;

    /* min and max aberations */
    float eMin = 99;
    float eMax = 0;
    for (int line = 0; line < _solver.HFRZones ; line++)
    {
        for (int column = 0; column < _solver.HFRZones; column++)
        {
            int zone = _solver.HFRZones * line + column;
            float e = _solver.aAxeAvgZone[zone] / _solver.bAxeAvgZone[zone] - 1;
            if (e < eMin) eMin = e;
            if (e > eMax) eMax = e;
        }
    }

    /* draw aberations */
    p2.begin(&imShape);
    p2.setOpacity(0.5);
    p2.setPen(QPen(Qt::red, 10));
    for (int line = 0; line < _solver.HFRZones ; line++)
    {
        for (int column = 0; column < _solver.HFRZones; column++)
        {
            int zone = _solver.HFRZones * line + column;

            int x = (imgWidth / _solver.HFRZones) * (column + 0.5) ;
            int y = (imgHeight / _solver.HFRZones) * (line + 0.5) ;
            float e = _solver.aAxeAvgZone[zone] / _solver.bAxeAvgZone[zone] - 1;
            float dx = (0.5 * imgWidth / _solver.HFRZones) * ((e - eMin) / (eMax - eMin)) * cos(_solver.thetaAvgZone[zone] * 3.14159 /
                       360);
            float dy = (0.5 * imgWidth / _solver.HFRZones) * ((e - eMin) / (eMax - eMin)) * sin(_solver.thetaAvgZone[zone] * 3.14159 /
                       360);

            p2.drawLine(x / 2 - dx, y / 2 - dy, x / 2 + dx, y / 2 + dy);
        }
    }
    p2.end();
    atomicSaveJpeg(imShape, getWebroot() + "/" + getModuleName() + "shape.jpeg");


    OST::ImgData dta = _image->ImgStats();
    dta.mUrlJpeg = getModuleName() + ".jpeg";
    dta.HFRavg = ech * _solver.HFRavg;
    dta.starsCount = _solver.stars.size();
    getEltImg("image", "image")->setValue(dta, true);

    dta = _image->ImgStats();
    dta.mUrlJpeg = getModuleName() + "corners.jpeg";
    getEltImg("corners", "image")->setValue(dta, true);
    dta.mUrlJpeg = getModuleName() + "HFR.jpeg";
    getEltImg("hfr", "image")->setValue(dta, true);
    dta.mUrlJpeg = getModuleName() + "shape.jpeg";
    getEltImg("shape", "image")->setValue(dta, true);



    emit FindStarsDone();

    getEltLight("states", "analyzing")->setValue(OST::Idle, true);

    if (mState == "loop")
        Shoot();
    else
        finishSingle();
}

// -------------------------------------------------------------------------
// Collimation analysis (defocused donuts, OpenCV). cf. collimator-spec.md.
// -------------------------------------------------------------------------
void Inspector::analyzeFrame(void)
{
    if (!_image)
        return;

    FITSImage::Statistic stats = _image->getStats();
    uint8_t *buffer = _image->getImageBuffer();
    if (!buffer || stats.width == 0 || stats.height == 0)
        return;

    // Wrap the raw buffer as an OpenCV Mat. v1 assumes a mono camera (the
    // usual case for collimation/guide cameras) -- color/Bayer is not
    // handled here.
    cv::Mat raw;
    if (stats.bytesPerPixel == 2)
        raw = cv::Mat(stats.height, stats.width, CV_16UC1, buffer);
    else
        raw = cv::Mat(stats.height, stats.width, CV_8UC1, buffer);

    // Stretch to 8-bit for thresholding/contour work.
    double lo = stats.min[0];
    double hi = stats.max[0];
    if (hi <= lo)
        hi = lo + 1;
    cv::Mat img8;
    raw.convertTo(img8, CV_8UC1, 255.0 / (hi - lo), -255.0 * lo / (hi - lo));

    // Coarse candidate detection. A single global Otsu threshold picks one
    // brightness level for the whole frame, so a field with donuts of very
    // different brightness loses the faint ones under the bright one's
    // threshold. Adaptive threshold compares each pixel to its own local
    // neighborhood instead, so it catches both. Morphological close/open
    // bridges small gaps in a ring's edge and removes speckle noise before
    // contour extraction.
    int const blockSize = std::max(21, (std::min(stats.width, stats.height) / 8) | 1); // odd
    cv::Mat coarseBin;
    cv::adaptiveThreshold(img8, coarseBin, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, blockSize, -5);
    cv::Mat const morphKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(coarseBin, coarseBin, cv::MORPH_CLOSE, morphKernel);
    cv::morphologyEx(coarseBin, coarseBin, cv::MORPH_OPEN, morphKernel);

    std::vector<std::vector<cv::Point>> candidates;
    cv::findContours(coarseBin, candidates, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    getProperty("stars")->clearGrid();

    double const minArea = 30.0; // reject noise speckles
    double const cx0 = stats.width / 2.0;
    double const cy0 = stats.height / 2.0;

    // Pixels -> arcsec, same convention as the rest of OST (getSampling()).
    double scale = getSampling();
    if (!std::isfinite(scale) || scale <= 0)
        scale = 0;

    QImage rawImage = _image->getRawQImage();
    QImage overlay = rawImage.convertToFormat(QImage::Format_RGB32);
    overlay.setColorTable(rawImage.colorTable());
    QPainter painter(&overlay);
    painter.setRenderHint(QPainter::Antialiasing);

    // The display image (via fileio::generateQImage()) may be downsampled
    // relative to the full-resolution analysis buffer -- scale every drawn
    // coordinate down to match, while keeping all the reported property values
    // (stars grid, correction, convergence point) in true full-resolution px.
    double const overlaySx = overlay.width() > 0 ? overlay.width() / static_cast<double>(stats.width) : 1.0;
    double const overlaySy = overlay.height() > 0 ? overlay.height() / static_cast<double>(stats.height) : 1.0;

    std::vector<cv::Point2d> starPos;  // donut center, absolute image px
    std::vector<cv::Point2d> deform;   // deformation vector, px

    for (const std::vector<cv::Point> &candidate : candidates)
    {
        if (cv::contourArea(candidate) < minArea)
            continue;

        // Local refinement: re-threshold (Otsu) just this candidate's own
        // neighborhood, expanded a bit around its coarse bounding box. This
        // gives each star its own brightness-appropriate binary mask instead
        // of inheriting a one-size-fits-all threshold, and RETR_CCOMP on that
        // local patch recovers the outer/hole contour pair cleanly.
        cv::Rect coarseBox = cv::boundingRect(candidate);
        int const marginX = coarseBox.width / 4 + 4;
        int const marginY = coarseBox.height / 4 + 4;
        cv::Rect box(coarseBox.x - marginX, coarseBox.y - marginY,
                     coarseBox.width + 2 * marginX, coarseBox.height + 2 * marginY);
        box &= cv::Rect(0, 0, img8.cols, img8.rows);
        if (box.width <= 0 || box.height <= 0)
            continue;

        cv::Mat roi = img8(box);
        cv::Mat localBin;
        cv::threshold(roi, localBin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

        std::vector<std::vector<cv::Point>> localContours;
        std::vector<cv::Vec4i> localHierarchy;
        cv::findContours(localBin, localContours, localHierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

        int bestIdx = -1;
        double bestArea = 0;
        for (size_t j = 0; j < localContours.size(); j++)
        {
            if (localHierarchy[j][3] != -1)
                continue; // top-level only
            double a = cv::contourArea(localContours[j]);
            if (a > bestArea)
            {
                bestArea = a;
                bestIdx = static_cast<int>(j);
            }
        }
        if (bestIdx < 0 || bestArea < minArea)
            continue;

        // Circle fit through the outer boundary rather than its filled-area
        // centroid: tolerates a partial arc, so it still recovers something
        // close to the star's true position even when the shadow has eaten
        // into the outer boundary (crescent case, no separate child/hole
        // contour) -- see fitCircleKasa() above.
        std::vector<cv::Point> outerFull = localContours[bestIdx];
        for (cv::Point &pt : outerFull)
            pt += box.tl();

        cv::Point2d center;
        if (!fitCircleKasa(outerFull, center))
            continue;

        // Ring mask: filled outer contour minus every hole found inside it,
        // in the local ROI's own coordinate space.
        cv::Mat mask = cv::Mat::zeros(box.size(), CV_8UC1);
        cv::drawContours(mask, localContours, bestIdx, cv::Scalar(255), cv::FILLED);
        for (int child = localHierarchy[bestIdx][2]; child != -1; child = localHierarchy[child][0])
            cv::drawContours(mask, localContours, child, cv::Scalar(0), cv::FILLED);

        // Deformation vector: intensity-weighted centroid of the ring alone
        // (excluding the shadow, when one was detected) versus the fitted
        // circle center above. This stays meaningful both for a strongly
        // comatic/"V"-shaped ring and for a crescent (shadow eating into the
        // boundary), unlike comparing two fitted ellipse centers.
        double sumI = 0, sumX = 0, sumY = 0;
        for (int y = 0; y < mask.rows; y++)
        {
            const uchar *mrow = mask.ptr<uchar>(y);
            const uchar *irow = roi.ptr<uchar>(y);
            for (int x = 0; x < mask.cols; x++)
            {
                if (mrow[x])
                {
                    double v = irow[x];
                    sumI += v;
                    sumX += v * (x + box.x);
                    sumY += v * (y + box.y);
                }
            }
        }
        if (sumI <= 0)
            continue;

        cv::Point2d weighted(sumX / sumI, sumY / sumI);
        cv::Point2d d = weighted - center;

        starPos.push_back(center);
        deform.push_back(d);

        getProperty("stars")->newLine(
        {
            {"x", center.x}, {"y", center.y}, {"dx", d.x}, {"dy", d.y}
        }, true);

        drawStarOverlay(painter, outerFull, center, d, scale, overlaySx, overlaySy);
    }
    getProperty("stars")->emitAll();

    if (starPos.size() < 2)
    {
        getEltLight("correction", "quality")->setValue(OST::Error, true);
        drawConvergenceOverlay(painter, cx0, cy0, 0, 0, false, scale, overlaySx, overlaySy);
        painter.end();
        publishCollimationImages(overlay);
        return;
    }

    // Fit D(P) = C - k*P by least squares (P relative to the image center),
    // cf. collimator-spec.md "Principe retenu pour l'algo d'analyse". SVD
    // handles the few-stars / ill-conditioned cases gracefully rather than
    // failing outright.
    int const n = static_cast<int>(starPos.size());
    cv::Mat A(2 * n, 3, CV_64F);
    cv::Mat b(2 * n, 1, CV_64F);
    for (int i = 0; i < n; i++)
    {
        double px = starPos[i].x - cx0;
        double py = starPos[i].y - cy0;
        A.at<double>(2 * i, 0) = 1;
        A.at<double>(2 * i, 1) = 0;
        A.at<double>(2 * i, 2) = -px;
        b.at<double>(2 * i, 0) = deform[i].x;
        A.at<double>(2 * i + 1, 0) = 0;
        A.at<double>(2 * i + 1, 1) = 1;
        A.at<double>(2 * i + 1, 2) = -py;
        b.at<double>(2 * i + 1, 0) = deform[i].y;
    }
    cv::Mat sol;
    cv::solve(A, b, sol, cv::DECOMP_SVD);
    double Cx = sol.at<double>(0, 0);
    double Cy = sol.at<double>(1, 0);
    double k  = sol.at<double>(2, 0);

    double amplitudeArcsec = std::hypot(Cx, Cy) * scale;

    getEltFloat("correction", "cx")->setValue(Cx, false);
    getEltFloat("correction", "cy")->setValue(Cy, false);
    getEltFloat("correction", "amplitude")->setValue(amplitudeArcsec, false);

    bool hasConvergence = std::fabs(k) > 1e-9;
    double convX = cx0;
    double convY = cy0;
    if (hasConvergence)
    {
        convX = cx0 + Cx / k;
        convY = cy0 + Cy / k;
        getEltFloat("correction", "convergencex")->setValue(convX, false);
        getEltFloat("correction", "convergencey")->setValue(convY, false);
    }

    OST::State quality = OST::Ok;
    if (amplitudeArcsec > 120)
        quality = OST::Error;
    else if (amplitudeArcsec > 30)
        quality = OST::Busy;
    getEltLight("correction", "quality")->setValue(quality, true);

    // Project the collimation vector onto the 3 screws. v1 simplification
    // (cf. collimator-spec.md "Convention mecanique"): fixed 120 deg spacing,
    // no auto-detection of the screws' actual orientation in the image, so
    // screw 1 is arbitrarily "up" (+Y). Correction is -C projected on each
    // screw's axis: dialing in that amount should null the observed error.
    for (int s = 0; s < 3; s++)
    {
        double angle = (M_PI / 2.0) + s * (2.0 * M_PI / 3.0);
        double ux = std::cos(angle);
        double uy = std::sin(angle);
        double proj = -(Cx * ux + Cy * uy) * scale;
        getEltFloat("screws", QString("screw%1").arg(s + 1))->setValue(proj, s == 2);
    }

    drawConvergenceOverlay(painter, cx0, cy0, convX, convY, hasConvergence, scale, overlaySx, overlaySy);
    painter.end();

    publishCollimationImages(overlay);
}

// The main image must stay virgin (raw frame, no drawing) -- like the
// inspection pipeline, which keeps its overlays in dedicated images. The
// annotated collimation overlay goes to its own "collim" image.
void Inspector::publishCollimationImages(const QImage &overlay)
{
    if (!_image)
        return;

    QImage rawImage = _image->getRawQImage();
    QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
    im.setColorTable(rawImage.colorTable());

    atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + ".jpeg");
    atomicSaveJpeg(overlay, getWebroot() + "/" + getModuleName() + "collim.jpeg");

    OST::ImgData dta = _image->ImgStats();
    dta.mUrlJpeg = getModuleName() + ".jpeg";
    dta.isSolved = false;
    getEltImg("image", "image")->setValue(dta, true);

    OST::ImgData cdta = _image->ImgStats();
    cdta.mUrlJpeg = getModuleName() + "collim.jpeg";
    cdta.isSolved = false;
    getEltImg("collim", "image")->setValue(cdta, true);
}

void Inspector::drawStarOverlay(QPainter &painter, const std::vector<cv::Point> &contour,
                                const cv::Point2d &center, const cv::Point2d &deform, double scale,
                                double sx, double sy)
{
    // All coordinates below are in full-resolution analysis pixels; sx/sy
    // convert them down to the (possibly subsampled) overlay image's own
    // pixel grid -- see the comment in analyzeFrame().
    QPolygon poly;
    for (const cv::Point &p : contour)
        poly << QPoint(qRound(p.x * sx), qRound(p.y * sy));
    painter.setPen(QPen(QColor(80, 160, 255), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawPolygon(poly);

    // Deformation arrow, exaggerated for visibility -- raw offsets are a few
    // pixels at most, invisible if drawn at true scale.
    double const arrowScale = 8.0;
    QPointF from(center.x * sx, center.y * sy);
    QPointF to((center.x + deform.x * arrowScale) * sx, (center.y + deform.y * arrowScale) * sy);

    double amplitudeArcsec = std::hypot(deform.x, deform.y) * scale;
    QColor color(0, 220, 0);
    if (amplitudeArcsec > 5)
        color = QColor(230, 0, 0);
    else if (amplitudeArcsec > 1)
        color = QColor(255, 160, 0);

    painter.setPen(QPen(color, 2));
    painter.drawLine(from, to);

    double angle = std::atan2(to.y() - from.y(), to.x() - from.x());
    double const headLen = 6.0;
    QPointF h1 = to - QPointF(headLen * std::cos(angle - M_PI / 6.0), headLen * std::sin(angle - M_PI / 6.0));
    QPointF h2 = to - QPointF(headLen * std::cos(angle + M_PI / 6.0), headLen * std::sin(angle + M_PI / 6.0));
    painter.drawLine(to, h1);
    painter.drawLine(to, h2);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(from, 3, 3);
}

void Inspector::drawConvergenceOverlay(QPainter &painter, double cx0, double cy0, double convX, double convY,
                                       bool hasConvergence, double scale, double sx, double sy)
{
    QPointF center(cx0 * sx, cy0 * sy);
    double const radiusScale = (sx + sy) / 2.0;

    // Bullseye: tolerance rings at the same amplitude thresholds used for the
    // "quality" light (30"/120").
    painter.setPen(QPen(QColor(255, 255, 255), 1, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    if (scale > 0)
    {
        painter.drawEllipse(center, 30.0 / scale * radiusScale, 30.0 / scale * radiusScale);
        painter.drawEllipse(center, 120.0 / scale * radiusScale, 120.0 / scale * radiusScale);
    }
    painter.setPen(QPen(QColor(255, 255, 255), 1));
    painter.drawLine(center - QPointF(10, 0), center + QPointF(10, 0));
    painter.drawLine(center - QPointF(0, 10), center + QPointF(0, 10));

    if (!hasConvergence)
        return;

    QPointF conv(convX * sx, convY * sy);
    painter.setPen(QPen(QColor(255, 0, 255), 2));
    painter.drawLine(center, conv);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 0, 255));
    painter.drawEllipse(conv, 5, 5);
}

// Record the focuser's current absolute position as the "home" to return to.
// There is deliberately no auto-read at module start: nothing tells us the
// focus is even roughly right, so the user must set it explicitly.
void Inspector::setHome(void)
{
    if (!hasFocuser())
    {
        getProperty("actions")->setState(OST::Error, true);
        return;
    }
    double pos = 0;
    if (!getModNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION", pos))
    {
        logWarning("setHome - could not read the focuser position");
        getProperty("actions")->setState(OST::Error, true);
        return;
    }
    mHomePosition = pos;
    mHomeDefined = true;
    getEltFloat("focushome", "position")->setValue(pos, true);
    getEltLight("focushome", "defined")->setValue(OST::Ok, true);
    logInfo("setHome - focuser home set to %1", {QString::number(pos)});
    getProperty("actions")->setState(OST::Ok, true);
}

bool Inspector::goHome(void)
{
    if (!hasFocuser())
    {
        getProperty("actions")->setState(OST::Error, true);
        return false;
    }
    if (!mHomeDefined)
    {
        logWarning("goHome - home undefined, use 'Set home' first");
        getProperty("actions")->setState(OST::Ok, true);
        return false;
    }
    sendModNewNumber(getString("devices", "focuser"), "ABS_FOCUS_POSITION", "FOCUS_ABSOLUTE_POSITION", mHomePosition);
    getProperty("actions")->setState(OST::Busy, true);
    return true;
}

bool Inspector::goIntra(void)
{
    if (!hasFocuser())
    {
        getProperty("actions")->setState(OST::Error, true);
        return false;
    }
    sendModNewSwitch(getString("devices", "focuser"), "FOCUS_MOTION", "FOCUS_INWARD", ISS_ON);
    sendModNewNumber(getString("devices", "focuser"), "REL_FOCUS_POSITION", "FOCUS_RELATIVE_POSITION",
                     getInt("focusparams", "offset"));
    getProperty("actions")->setState(OST::Busy, true);
    return true;
}

bool Inspector::goExtra(void)
{
    if (!hasFocuser())
    {
        getProperty("actions")->setState(OST::Error, true);
        return false;
    }
    sendModNewSwitch(getString("devices", "focuser"), "FOCUS_MOTION", "FOCUS_OUTWARD", ISS_ON);
    sendModNewNumber(getString("devices", "focuser"), "REL_FOCUS_POSITION", "FOCUS_RELATIVE_POSITION",
                     getInt("focusparams", "offset"));
    getProperty("actions")->setState(OST::Busy, true);
    return true;
}
