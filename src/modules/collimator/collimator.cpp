#include "collimator.h"
#include "version.cc"
#include <opencv2/opencv.hpp>
#include <cmath>

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
        mRawImage = rawImage.convertToFormat(QImage::Format_RGB32);
        mRawImage.setColorTable(rawImage.colorTable());

        // Published once analysis is done, with the deformation overlay drawn
        // on top -- see analyzeFrame().
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
    if (!mImage)
        return;

    FITSImage::Statistic stats = mImage->getStats();
    uint8_t *buffer = mImage->getImageBuffer();
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

    // Pixels -> arcsec, same convention as polar/navigator. Computed up front:
    // needed both for the per-star overlay color coding and the final fit.
    double pixelSize = 0;
    getModNumber(getString("devices", "camera"), "CCD_INFO", "CCD_PIXEL_SIZE", pixelSize);
    double focalLength = getFloat("optic", "fl");
    double scale = (focalLength > 0) ? 206.3 * pixelSize / focalLength : 0; // arcsec/px

    QImage overlay = mRawImage.copy();
    QPainter painter(&overlay);
    painter.setRenderHint(QPainter::Antialiasing);

    // The display image (mRawImage, via fileio::generateQImage()) is
    // downsampled relative to the full-resolution analysis buffer -- scale
    // every drawn coordinate down to match, while keeping all the reported
    // property values (stars grid, correction, convergence point) in true
    // full-resolution pixels.
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
        atomicSaveJpeg(overlay, getWebroot() + "/" + getModuleName() + ".jpeg");
        OST::ImgData dta = mImage->ImgStats();
        dta.mUrlJpeg = getModuleName() + ".jpeg";
        dta.isSolved = false;
        getEltImg("image", "image")->setValue(dta, true);
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

    atomicSaveJpeg(overlay, getWebroot() + "/" + getModuleName() + ".jpeg");
    OST::ImgData dta = mImage->ImgStats();
    dta.mUrlJpeg = getModuleName() + ".jpeg";
    dta.isSolved = false;
    getEltImg("image", "image")->setValue(dta, true);
}

void Collimator::drawStarOverlay(QPainter &painter, const std::vector<cv::Point> &contour,
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

void Collimator::drawConvergenceOverlay(QPainter &painter, double cx0, double cy0, double convX, double convY,
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
