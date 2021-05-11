#ifndef FOCUS_MODULE_h_
#define FOCUS_MODULE_h_

#include "module.h"
#include "properties.h"
#include <QScxmlStateMachine>

class FocusModule : public Module
{
    Q_OBJECT
    Q_PROPERTY(double  focuserPosition MEMBER _focuserPosition NOTIFY focuserPositionChanged)
    //Q_PROPERTY(MapStringStr    test    MEMBER _test)
    //Q_PROPERTY(QMap<QString,bool>    test2    MEMBER _test2)
    Q_PROPERTY(QString camera   MEMBER _camera)
    Q_PROPERTY(QString focuser  MEMBER _focuser)

    public:
        FocusModule();
        ~FocusModule();
    signals:
        void focuserPositionChanged(const double &newFocuserPosition);
        //void expdone(IBLOB *bp);
        void focuserReachedPosition();

        void RequestFrameResetDone();
        void FrameResetDone();
        void RequestBacklashDone();
        void BacklashDone();
        void RequestGotoStartDone();
        void GotoStartDone();

        void RequestExposureDone();
        void ExposureDone();
        void FindStarsDone();
        void NextLoop();
        void RequestGotoNextDone();
        void GotoNextDone();
        void LoopFinished();

        void RequestBacklashBestDone();
        void BacklashBestDone();
        void RequestGotoBestDone();
        void GotoBestDone();
        void RequestExposureBestDone();
        void ExposureBestDone();
        void ComputeResultDone();
        void InitLoopFrameDone();
        void LoopFrameDone();
        void NextFrame();

        void blobloaded();
        void cameraAlert();
        void abort();
    public slots:
        void test0(QString txt);
        void OnIndiNewNumber(INumberVectorProperty *nvp) override;
        void OnIndiNewBLOB(IBLOB *bp) override;
        void OnIndiNewSwitch(ISwitchVectorProperty *svp) override;
        void OnSucessSEP();
    private:
        void SMRequestFrameReset();
        void SMRequestBacklash();
        void SMRequestGotoStart();
        void SMRequestExposure();
        void SMFindStars();
        void SMCompute();
        void SMRequestGotoNext();

        void SMRequestBacklashBest();
        void SMRequestGotoBest();
        void SMRequestExposureBest();
        void SMComputeResult();
        void SMInitLoopFrame();
        void SMComputeLoopFrame();


        void SMAlert();
        //void SMLoadblob(IBLOB *bp);
        void SMLoadblob();
        void SMAbort();
        void startCoarse();
        MapStringStr _test;
        QMap<QString,bool> _test2;
        QString _camera  = "CCD Simulator";
        QString _focuser = "Focuser Simulator";
        bool    _newblob;

        int    _startpos;
        int    _backlash;
        int    _iterations;
        int    _steps;
        int    _exposure;
        int    _loopIterations;
        int    _loopIteration;
        double _loopHFRavg;

        int    _iteration;
        double _bestpos;
        double _bestposfit;
        double _besthfr;
        double  _focuserPosition;
        QStateMachine _machine;

        std::vector<double> _posvector;
        std::vector<double> _hfdvector;
        std::vector<double> _coefficients;

};
#endif
