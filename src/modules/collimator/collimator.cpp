#include "collimator.h"
#include "version.cc"

static void atomicSaveJpeg(const QImage &img, const QString &finalPath)
{
    const QString tmp = finalPath + ".tmp";
    if (img.save(tmp, "JPG", 100))
        ::rename(tmp.toLocal8Bit().constData(), finalPath.toLocal8Bit().constData());
}

Collimator *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Collimator *basemodule = new Collimator(name, label, profile, availableModuleLibs);
    return basemodule;
}

Collimator::Collimator(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    loadOstPropertiesFromFile(":collimator.json");

    giveMeAnActions();
    OST::PropertyMulti *pm = getProperty("actions");
    pm->setRule(OST::SwitchsRule::Any);

    OST::ElementBool *b = new OST::ElementBool("start", "Start", "10", "");
    b->setPreIcon("play_arrow");
    pm->addElt(b);

    b = new OST::ElementBool("stop", "Stop", "20", "");
    b->setPreIcon("stop");
    pm->addElt(b);

    b = new OST::ElementBool("gohome", "Go home", "30", "");
    pm->addElt(b);

    b = new OST::ElementBool("gointra", "Go intra", "40", "");
    pm->addElt(b);

    b = new OST::ElementBool("goextra", "Go extra", "50", "");
    pm->addElt(b);

    getEltLight("states", "idle")->setValue(OST::Ok, true);

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Collimation assistant module");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));
    setMetadata("template", "collimator");

    giveMeADevice("camera", "Camera", INDI::BaseDevice::CCD_INTERFACE);
    giveMeADevice("mount", "Mount", INDI::BaseDevice::TELESCOPE_INTERFACE);
    giveMeADevice("focuser", "Focuser", INDI::BaseDevice::FOCUSER_INTERFACE);

    defineMeAsImager();

    connectIndi();
    connectAllDevices();

    connect(this, &Collimator::newImage, this, &Collimator::OnNewImage);
}

Collimator::~Collimator() {}

bool Collimator::hasMount(void)
{
    return !getString("devices", "mount").isEmpty();
}

bool Collimator::hasFocuser(void)
{
    return !getString("devices", "focuser").isEmpty();
}

void Collimator::initIndi(void)
{
    connectIndi();
    connectDevice(getString("devices", "camera"));
    if (hasMount())
        connectDevice(getString("devices", "mount"));
    if (hasFocuser())
        connectDevice(getString("devices", "focuser"));

    setBLOBMode(B_ALSO, getString("devices", "camera").toStdString().c_str(), nullptr);
    enableDirectBlobAccess(getString("devices", "camera").toStdString().c_str(), nullptr);
}

void Collimator::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "start")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
                startLoop();
        }
        if (event.eltkey == "stop")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
                stopLoop();
        }
        if (event.eltkey == "gohome")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
                goHome();
        }
        if (event.eltkey == "gointra")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
                goIntra();
        }
        if (event.eltkey == "goextra")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
                goExtra();
        }
    }
}

void Collimator::onUpdateProperty(INDI::Property property)
{
    if (strcmp(property.getName(), "CCD1") == 0)
    {
        newBLOB(property);
    }
    if (
        (property.getDeviceName() == getString("devices", "camera"))
        && (property.getState() == IPS_ALERT)
    )
    {
        logWarning("cameraAlert");
        emit cameraAlert();
    }
}

void Collimator::newBLOB(INDI::PropertyBlob pblob)
{
    if (
        (QString(pblob.getDeviceName()) == getString("devices", "camera")) && (mState == "running")
    )
    {
        mImage = new fileio();
        mImage->loadBlob(pblob, 64);
        QImage rawImage = mImage->getRawQImage();
        QImage im = rawImage.convertToFormat(QImage::Format_RGB32);
        im.setColorTable(rawImage.colorTable());
        atomicSaveJpeg(im, getWebroot() + "/" + getModuleName() + ".jpeg");
        OST::ImgData dta = mImage->ImgStats();
        dta.mUrlJpeg = getModuleName() + ".jpeg";
        dta.isSolved = false;
        getEltImg("image", "image")->setValue(dta, true);

        emit newImage();
    }
}

void Collimator::OnNewImage(void)
{
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "analyzing")->setValue(OST::Busy, true);

    analyzeFrame();

    getEltLight("states", "analyzing")->setValue(OST::Idle, true);

    // Continuous loop: shoot again right away while running.
    if (mState == "running")
        Shoot();
}

void Collimator::analyzeFrame(void)
{
    // TODO: OpenCV-based multi-star donut detection and deformation-field fit
    // (cf. collimator-spec.md "Principe retenu pour l'algo d'analyse"). Not
    // implemented yet -- this is the step after the UI data model is in place.
    logInfo("Collimator::analyzeFrame not implemented yet");
}

void Collimator::Shoot(void)
{
    if (!setFocalLengthAndDiameter())
    {
        stopLoop();
        return;
    }
    if (!requestCapture(getString("devices", "camera"), getFloat("parms", "exposure"), getInt("parms", "gain"),
                         getInt("parms", "offset")))
    {
        stopLoop();
        return;
    }
    getEltLight("states", "shooting")->setValue(OST::Busy, true);
}

void Collimator::startLoop(void)
{
    mState = "running";
    getEltLight("states", "idle")->setValue(OST::Idle, true);
    initIndi();
    Shoot();
}

void Collimator::stopLoop(void)
{
    mState = "idle";
    getEltLight("states", "shooting")->setValue(OST::Idle, false);
    getEltLight("states", "analyzing")->setValue(OST::Idle, false);
    getEltLight("states", "idle")->setValue(OST::Ok, true);
    getProperty("actions")->setState(OST::Ok, true);
}

void Collimator::goHome(void)
{
    if (!hasFocuser())
    {
        getProperty("actions")->setState(OST::Error, true);
        return;
    }
    // TODO: restore the focuser position recorded before defocusing.
    getProperty("actions")->setState(OST::Ok, true);
}

void Collimator::goIntra(void)
{
    if (!hasFocuser())
    {
        getProperty("actions")->setState(OST::Error, true);
        return;
    }
    sendModNewSwitch(getString("devices", "focuser"), "FOCUS_MOTION", "FOCUS_INWARD", ISS_ON);
    sendModNewNumber(getString("devices", "focuser"), "REL_FOCUS_POSITION", "FOCUS_RELATIVE_POSITION",
                      getInt("focusparams", "offset"));
    getProperty("actions")->setState(OST::Ok, true);
}

void Collimator::goExtra(void)
{
    if (!hasFocuser())
    {
        getProperty("actions")->setState(OST::Error, true);
        return;
    }
    sendModNewSwitch(getString("devices", "focuser"), "FOCUS_MOTION", "FOCUS_OUTWARD", ISS_ON);
    sendModNewNumber(getString("devices", "focuser"), "REL_FOCUS_POSITION", "FOCUS_RELATIVE_POSITION",
                      getInt("focusparams", "offset"));
    getProperty("actions")->setState(OST::Ok, true);
}
