#ifndef DUMMY_MODULE_h_
#define DUMMY_MODULE_h_
#include <indimodule.h>
#include <fileio.h>
#include <solver.h>

#if defined(DUMMY_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Dummy : public IndiModule
{
        Q_OBJECT

    public:
        Dummy(QString name, QString label, QString profile, QVariantMap availableModuleLibs);
        ~Dummy();
        void onNewDevice      (INDI::BaseDevice dp) override     {} ;
        void onRemoveDevice   (INDI::BaseDevice dp) override     {} ;
        void onNewProperty    (INDI::Property property) override {} ;
        void onRemoveProperty (INDI::Property property) override {} ;
        void onUpdateProperty (INDI::Property property) override {} ;


    public slots:
        void OnSucessSEP();
        void OnSucessSolve();
        void OnSolveFinished();
        void OnSolverLog(QString text);
        void OnNewImage();
    private:
        void newBLOB(INDI::PropertyBlob pblob);
        void newDevice(INDI::BaseDevice bd) override;

        QString _camera;
        QPointer<fileio> _image;
        Solver _solver;
        FITSImage::Statistic stats;
        void updateProperty(INDI::Property property) override;
        void updateSearchList(void);

        OST::PropertyMulti *dynprop;
        OST::ElementLight *dynlight;
        OST::ElementString *dyntext;
        OST::ElementBool *dynbool;
        OST::ElementString *dyntext2;
    signals:
        void newImage();
    protected:
        /**
         * @brief Custom module event handler (Hook 3/3)
         *
         * Override of Basemodule's hook for Dummy-specific events.
         * Called automatically after onExternalEventBase() and onExternalEventIndi().
         *
         * NO need to call parent - orchestration is automatic!
         */
        void onExternalEvent(OST::ExtEvent event) override;

    private slots:
        void onTimer(void);


};

extern "C" MODULE_INIT Dummy *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs);

#endif
