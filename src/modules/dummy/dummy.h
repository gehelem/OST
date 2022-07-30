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
        void OnExternalEvent(const QString &eventType, const QString &eventData) override;

};

extern "C" MODULE_INIT Dummy *initialize(QString name,QString label,QString profile);

#endif
