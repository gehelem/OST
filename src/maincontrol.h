#ifndef MAINCONTROL_MODULE_h_
#define MAINCONTROL_MODULE_h_
#include <basemodule.h>

class MainControl : public Basemodule
{
    Q_OBJECT

    public:
        MainControl(QString name,QString label);
        ~MainControl();
        void newProperty(INDI::Property*) override;
    public slots:
        void OnSetPropertyText(TextProperty* prop) override;
        void OnSetPropertyNumber(NumberProperty* prop) override;
        void OnSetPropertySwitch(SwitchProperty* prop) override;

    signals:
        void valueChanged(const double &newValue);
    private:
        SwitchProperty* _chooseModule;
        TextProperty* _giveItaName;
        SwitchProperty* _loadModule;


};


#endif
