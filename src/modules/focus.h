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
        void IndiNewNumber(INumberVectorProperty *nvp);
        void IndiNewBLOB(IBLOB *bp);

    private:
        void CallStartCoarse(void);
        void startCoarse(void);
        QString _camera;
        double  _focuserPosition;
        bool    _newblob;

};
#endif
