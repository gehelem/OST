#ifndef COLLIMATOR_MODULE_h_
#define COLLIMATOR_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <QPainter>
#include <opencv2/core.hpp>
#include <vector>

#if defined(COLLIMATOR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Collimator : public IndiModule
{
        Q_OBJECT

    public:
        Collimator(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Collimator();
        void onNewDevice      (INDI::BaseDevice ) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice ) override     {} ;
        void onNewProperty    (INDI::Property ) override {} ;
        void onRemoveProperty (INDI::Property ) override {} ;
        void onUpdateProperty (INDI::Property property) override;

    signals:
        void cameraAlert();
        void newImage();

    public slots:
        void OnNewImage();

    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    private:
        void newBLOB(INDI::PropertyBlob pblob);

        void initIndi(void);
        void Shoot(void);

        void startLoop(void);
        void stopLoop(void);
        void goHome(void);
        void goIntra(void);
        void goExtra(void);

        bool hasMount(void);
        bool hasFocuser(void);

        // Single-image, multi-star analysis (OpenCV, cf. collimator-spec.md):
        // for every donut detected in the frame, compute a local deformation
        // vector, then fit the linear field D(P) = C - k*P across all of them.
        // C (the collimation vector) is what feeds the per-screw corrections.
        // Not implemented yet -- this is the next step after the UI data model.
        void analyzeFrame(void);
        void drawStarOverlay(QPainter &painter, const std::vector<cv::Point> &contour,
                              const cv::Point2d &center, const cv::Point2d &deform, double scale,
                              double sx, double sy);
        void drawConvergenceOverlay(QPainter &painter, double cx0, double cy0, double convX, double convY,
                                     bool hasConvergence, double scale, double sx, double sy);

        QString mState = "idle";

        QPointer<fileio> mImage;
        QImage mRawImage;
};

extern "C" MODULE_INIT Collimator *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
