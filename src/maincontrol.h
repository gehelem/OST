#ifndef MAINCONTROL_MODULE_h_
#define MAINCONTROL_MODULE_h_
#include <basemodule.h>

class MainControl : public Basemodule
{
    Q_OBJECT

    public:
        MainControl(QString name,QString label);
        ~MainControl();
    signals:
        void valueChanged(const double &newValue);
    private:


};


#endif
