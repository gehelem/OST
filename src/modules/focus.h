#ifndef FOCUS_MODULE_h_
#define FOCUS_MODULE_h_

#include "module.h"

class FocusModule : public Module
{
    Q_OBJECT
    Q_PROPERTY(double  focuserPosition MEMBER _focuserPosition NOTIFY focuserPositionChanged)
    Q_PROPERTY(QString camera   MEMBER _camera)
    Q_PROPERTY(QString focuser  MEMBER _focuser)

    public:
        FocusModule();
        ~FocusModule();
    signals:
        void focuserPositionChanged(const double &newFocuserPosition);
        void blobReceived();
        void focuserReachedPosition();
        void frameResetDone();
        void cameraAlert();
        void abort();
    public slots:
        void test0(QString txt);
        void IndiNewNumber(INumberVectorProperty *nvp) override;
        void IndiNewBLOB(IBLOB *bp) override;
        void IndiNewSwitch(ISwitchVectorProperty *svp) override;
    private:
        void SMFrameReset(void);
        void SMExpRequest(void);
        void SMAlert(void);
        void CallStartCoarse(void);
        void startCoarse(void);
        QString _camera;
        QString _focuser;
        double  _focuserPosition;
        bool    _newblob;
        QStateMachine machine;

};
#endif
