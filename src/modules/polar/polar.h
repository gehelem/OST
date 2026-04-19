#ifndef POLAR_MODULE_h_
#define POLAR_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>

#if defined(POLAR_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

#include <QtCore>
#include <QtConcurrent>
#include <QStateMachine>
#include <libastro.h>
#include <libnova/julian_day.h>
#include <libnova/sidereal_time.h>


class MODULE_INIT Polar : public IndiModule
{
        Q_OBJECT

    public:
        Polar(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Polar();

    signals:
        void InitDone();
        void AbortDone();
        void Abort();
        void RequestFrameResetDone();
        void FrameResetDone();
        void RequestExposureDone();
        void ExposureDone();
        void FindStarsDone();
        void ComputeDone();
        void ComputeFinalDone();
        void PolarDone();
        void RequestMoveDone();
        void MoveDone();

    protected:
        void onExternalEvent(OST::ExtEvent event) override;

    public slots:
        void OnSucessSolve();
        void OnSolverLog(QString &text);
    private:
        void updateProperty(INDI::Property property) override;
        void newBLOB(INDI::PropertyBlob pblob);

        /*SwitchProperty* _actions;
        NumberProperty* _commonParams;
        NumberProperty* _calParams;
        NumberProperty* _guideParams;
        NumberProperty* _values;
        NumberProperty* _errors;
        ImageProperty*  _img;
        LightProperty*  _states;*/


        //std::unique_ptr<Image> image =nullptr;
        QPointer<fileio> image;
        FITSImage::Statistic mStats;

        double _exposure = 2;
        double _mountDEC;
        double _mountRA;
        bool   _mountPointingWest = false;
        double _ccdOrientation;
        double _aperture;
        double _focalLength;
        double _ccdX;
        double _ccdY;
        double _ccdSize;
        double _ccdFov;
        double _pixelSize;
        double _ccdSampling = 206 * 5.2 / 800;
        double _ra0 = 0;
        double _de0 = 0;
        double _t0 = 0;
        double _ra1 = 0;
        double _de1 = 0;
        double _t1 = 0;
        double _ra2 = 0;
        double _de2 = 0;
        double _t2 = 0;
        double _erraz = 0;
        double _erralt = 0;
        double _errtot = 0;
        int _itt = 0;

        double _observerLat = 0.0;
        double _observerLon = 0.0;


        QString _camera  = "CCD Simulator";
        QString _mount  = "Telescope Simulator";
        QStateMachine _machine;
        Solver _solver;
        double square(double value)
        {
            return value * value;
        }

        QPointF solverToAzAlt(double ra_j2000_deg, double dec_j2000_deg, double jd);
        void buildStateMachine(void);
        void SMInit();
        void SMRequestExposure();
        void SMFindStars();
        void SMCompute();
        void SMComputeFinal();
        void SMRequestFrameReset();
        void SMRequestMove();
        void SMAbort();
};

extern "C" MODULE_INIT Polar *initialize(QString name, QString label, QString profile,
        QVariantMap availableModuleLibs);

#endif
