#ifndef MFOCUSER_h_
#define MFOCUSER_h_
#include "module.h"

class MFocuser : public Module
{
    Q_OBJECT
    public:
        MFocuser(Properties *properties);
        ~MFocuser();
        void initProperties(void);
        Q_PROPERTY(Prop rrrrr);
        Q_PROPERTY(ONumber nnnn);

    public slots:
        void slotvalueChangedFromCtl(Prop prop);

    private:
        std::vector<double> posvector;
        std::vector<double> hfdvector;
        std::vector<double> coefficients;
        void startFraming(void);
        void startCoarse(void);
        void startFine(void);
        void abort(void);
        void executeTaskSpec(Ttask task);
        void executeTaskSpec(Ttask task,IBLOB *bp);
        void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) ;
        void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) ;
        void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) ;
        void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) ;

};
#endif
