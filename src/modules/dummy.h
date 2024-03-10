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

    public slots:
        void OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey,
                               const QVariantMap &eventData) override;
        void OnSucessSEP();
        void OnSucessSolve();
        void OnSolverLog(QString &text);
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


};

extern "C" MODULE_INIT Dummy *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs);

#endif
