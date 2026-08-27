#ifndef INSPECTOR_MODULE_h_
#define INSPECTOR_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>
#include <QPainter>
#include <opencv2/core.hpp>
#include <vector>

#if defined(INSPECTOR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

// Image analysis module. Two independent, non-exclusive analyses can be run on
// each captured (or reloaded) frame, selected by the "inspector" / "collimator"
// toggle actions:
//  - inspector : StellarSolver on a focused frame -> HFR / shape / corners maps
//                (tilt, field curvature, aberrations)
//  - collimator: OpenCV on a defocused frame -> per-donut deformation vectors,
//                collimation vector C by field-convergence fit, per-screw
//                corrections (cf. collimator-spec.md)
class MODULE_INIT Inspector : public IndiModule
{
        Q_OBJECT

    public:
        Inspector(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Inspector();
        void onNewDevice      (INDI::BaseDevice dp) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice dp) override     {} ;
        void onNewProperty    (INDI::Property property) override {} ;
        void onRemoveProperty (INDI::Property property) override {} ;
        void onUpdateProperty (INDI::Property property) override;

    signals:
        void FrameResetDone();
        void FindStarsDone();
        void cameraAlert();
        void Abort();
        void newImage();

    public slots:
        void OnSucessSEP();
        void OnNewImage();

    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    private:
        void newBLOB(INDI::PropertyBlob pblob);

        void initIndi(void);
        void Shoot(void);
        void finishSingle(void);

        // inspector analysis (focused frame, StellarSolver) -- see OnSucessSEP()
        void runInspection(void);

        // collimator analysis (defocused frame, OpenCV). For every donut in the
        // frame, compute a local deformation vector, then fit the linear field
        // D(P) = C - k*P across all of them. C (the collimation vector) feeds
        // the per-screw corrections. cf. collimator-spec.md.
        void analyzeFrame(void);
        void drawStarOverlay(QPainter &painter, const std::vector<cv::Point> &contour,
                             const cv::Point2d &center, const cv::Point2d &deform, double scale,
                             double sx, double sy);
        void drawConvergenceOverlay(QPainter &painter, double cx0, double cy0, double convX, double convY,
                                    bool hasConvergence, double scale, double sx, double sy);
        void publishCollimationImages(const QImage &overlay);

        // focuser helpers (manual defocus for the collimation workflow).
        // goHome/goIntra/goExtra return true when a move command was actually
        // sent to the focuser, so the caller can keep the action button pressed
        // until ABS_FOCUS_POSITION settles (see onUpdateProperty()).
        void setHome(void);
        bool goHome(void);
        bool goIntra(void);
        bool goExtra(void);
        bool hasFocuser(void);

        void publishRawImage(void);

        // Reset an image element to a blank URL so the frontend stops showing it.
        void blankImage(const QString &prop);

        QPointer<fileio> _image;
        Solver _solver;
        FITSImage::Statistic stats;

        QString mState = "idle";
        // Previous frame's analysis selection, to detect a mid-loop change and
        // blank the images of an analysis that has just been switched off.
        bool mPrevInspect = false;
        bool mPrevCollim = false;
        // Focuser "home" position, set explicitly by the user via "Set home".
        // Not read at start-up (see setHome()).
        bool mHomeDefined = false;
        double mHomePosition = 0;
        // "gohome" / "gointra" / "goextra" while a focuser move it triggered is
        // still running; empty otherwise. The button stays pressed until
        // ABS_FOCUS_POSITION leaves the BUSY state.
        QString mFocuserButton;
        double upperLeftHFR;
        double lowerLeftHFR;
        double upperRightHFR;
        double lowerRightHFR;
};

extern "C" MODULE_INIT Inspector *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
