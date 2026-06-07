#ifndef NAVIGATOR_MODULE_h_
#define NAVIGATOR_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>

#include <libastro.h>
#include <libnova/julian_day.h>

#define PI 3.14159265

#if defined(NAVIGATOR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Navigator : public IndiModule
{
        Q_OBJECT

    public:
        Navigator(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Navigator();
        void onNewDevice      (INDI::BaseDevice dp) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice dp) override     {} ;
        void onNewProperty    (INDI::Property property) override {} ;
        void onRemoveProperty (INDI::Property property) override {} ;
        void onUpdateProperty (INDI::Property property);

    signals:

        void cameraAlert();
        void newImage();
    public slots:
        void OnSucessSolve();
        void OnSolverLog(QString text);
        void OnNewImage();
    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    private:
        void newBLOB(INDI::PropertyBlob pblob);

        void initIndi(void);

        void Shoot();
        void SMAlert();
        void updateSearchList(void);
        void slewToSelection(void);
        void convertSelection(void);
        void correctOffset(double solvedRA, double solvedDEC);
        bool checkCentering(double solvedRA, double solvedDEC, double targetRA, double targetDEC);
        void syncMountIfNeeded(double solvedRA, double solvedDEC);


        QString mState = "idle";

        QPointer<fileio> pImage;

        StellarSolver stellarSolver;
        QList<FITSImage::Star> stars;

        /**
         * @brief Add current target to planner
         */
        void addTargeToPlanner();


        // Centering iteration parameters
        int mMaxIterations;
        int mCurrentIteration;
        double mToleranceArcsec;
        double mTargetRA;
        double mTargetDEC;
        double mTargetRAnow;
        double mTargetDECnow;
        bool mWaitingSlew;


};

extern "C" MODULE_INIT Navigator *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
