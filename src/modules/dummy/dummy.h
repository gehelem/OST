#ifndef DUMMY_MODULE_h_
#define DUMMY_MODULE_h_
#include <basemodule.h>

#if defined(DUMMY_MODULE)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT Dummy : public Basemodule
{
    Q_OBJECT

    public:
        Dummy(QString name,QString label,QString profile);
        ~Dummy();

    public slots:
        void OnMyExternalEvent(const QString &eventType, const QString  &eventModule, const QString  &eventKey, const QVariantMap &eventData) override;
        void OnSucessSEP();
    private:
        void newBLOB(IBLOB *bp) override;
        QString _camera;
        QPointer<Image> _image;
        Solver _solver;



};

extern "C" MODULE_INIT Dummy *initialize(QString name,QString label,QString profile);

#endif
