#ifndef FOCUS_MODULE_h_
#define FOCUS_MODULE_h_

#include "module.h"

class FocusModule : public Module
{
    Q_OBJECT
    Q_PROPERTY(double  focuserPosition MEMBER _focuserPosition NOTIFY focuserPositionChanged)
    Q_PROPERTY(QString camera MEMBER _camera)

    public:
        FocusModule(void);
        ~FocusModule();
    signals:
        void focuserPositionChanged(const double &newFocuserPosition);
    public slots:
        void test0(QString txt);

    private:
        QString _camera;
        double  _focuserPosition;
}
;
#endif
