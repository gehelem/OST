#ifndef FOCUS_MODULE_h_
#define FOCUS_MODULE_h_

#include "module.h"
#include "properties.h"
#include <QScxmlStateMachine>

class FocusModule : public Module
{
    Q_OBJECT
    Q_PROPERTY(double  focuserPosition MEMBER _focuserPosition NOTIFY focuserPositionChanged)
    Q_PROPERTY(MapStringStr    test    MEMBER _test)
    Q_PROPERTY(QMap<QString,bool>    test2    MEMBER _test2)
    Q_PROPERTY(QString camera   MEMBER _camera)
    Q_PROPERTY(QString focuser  MEMBER _focuser)

    public:
        FocusModule();
        ~FocusModule();
    signals:
        void focuserPositionChanged(const double &newFocuserPosition);
        //void expdone(IBLOB *bp);
        void expdone();
        void exprequestdone();
        void focuserReachedPosition();
        void frameresetdone();
        void blobloaded();
        void cameraAlert();
        void abort();
    public slots:
        void test0(QString txt);
        void IndiNewNumber(INumberVectorProperty *nvp) override;
        void IndiNewBLOB(IBLOB *bp) override;
        void IndiNewSwitch(ISwitchVectorProperty *svp) override;
    private:
        void SMFrameReset();
        void SMExpRequest();
        void SMAlert();
        //void SMLoadblob(IBLOB *bp);
        void SMLoadblob();
        void SMAbort();
        void startCoarse();
        MapStringStr _test;
        QMap<QString,bool> _test2;
        QString _camera = "CCD Simulator";
        QString _focuser = "Focuser Simulator";
        double  _focuserPosition;
        bool    _newblob;
        QStateMachine *_machine;

};
#endif
