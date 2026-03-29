#ifndef ALLSKY_MODULE_h_
#define ALLSKY_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>


#if defined(ALLSKY_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Allsky : public IndiModule
{
        Q_OBJECT

    public:
        Allsky(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Allsky();

    public slots:
        void onExternalEvent(OST::ExtEvent event) override;
        void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    private slots:
        void OnTimer(void);
        void OnScheduleTimer(void);
    private:
        void newBLOB(INDI::PropertyBlob pblob);
        void updateProperty(INDI::Property property) override;
        void startLoop();
        void stopLoop();
        void startTimelapseBatch();
        void processOutput();
        void processError();
        void computeExposureOrGain(double fromValue);
        void checkArchives(void);
        void moveCurrentToArchives(void);
        void calculateSunset(void);
        void addGPSLocalization(void);
        void enableParms(bool enable);

        QPointer<fileio> _image;
        QImage imageStacked;
        bool firstStack = true;
        Solver _solver;
        FITSImage::Statistic stats;
        long _index;
        QProcess *_process;
        QImage mKeog;
        QTimer mTimer;
        bool mIsLooping = false;
        QString mFolder;
        QTimer mScheduleTimer;

};

extern "C" MODULE_INIT Allsky *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs);

#endif
