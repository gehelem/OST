#ifndef MAINCONTROL_MODULE_h_
#define MAINCONTROL_MODULE_h_
#include <basemodule.h>

#if defined(MAINCONTROL_MODULE_h_)
#  define MODULE_INIT Q_DECL_EXPORT
#else
#  define MODULE_INIT Q_DECL_IMPORT
#endif

class MODULE_INIT MainControl : public Basemodule
{
    Q_OBJECT

    public:
        MainControl(QString name,QString label);
        ~MainControl();
    signals:
        void valueChanged(const double &newValue);
    private:

};

extern "C" MODULE_INIT MainControl *initialize(QString name,QString label);

#endif
