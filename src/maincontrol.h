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
        //void OnSetPropertyText(TextProperty* prop) override;

    signals:
        void valueChanged(const double &newValue);
        void loadModule(QString lib, QString label);

    private:


};


#endif
