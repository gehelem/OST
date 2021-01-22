#ifndef MODULE_h_
#define MODULE_h_
#include <QtCore>
#include "client.h"
#include "image.h"
#include "properties.h"

enum Tasktype
{
    /*! Wait new property event from indiserver */
    TT_WAIT_PROP,
    /*! Wait new number event from indiserver */
    TT_WAIT_NUMBER,
    /*! Wait new text event from indiserver */
    TT_WAIT_TEXT,
    /*! Wait new switch event from indiserver */
    TT_WAIT_SWITCH,
    /*! Wait Stellarsolver SEP job to finish */
    TT_WAIT_SEP,
    /*! Wait Stellarsolver Solve job to finish */
    TT_WAIT_SOLVE,
    /*! Wait new Blob event from indiserver */
    TT_WAIT_BLOB,
    /*! Wait new light event from indiserver */
    TT_WAIT_LIGHT,
    /*! Send new number to indiserver */
    TT_SEND_NUMBER,
    /*! Send new text to indiserver */
    TT_SEND_TEXT,
    /*! Send new switch to indiserver */
    TT_SEND_SWITCH,
    /*! Asking Stellarsolver to find stars*/
    TT_ANALYSE_SEP,
    /*! Asking Stellarsolver to solve field */
    TT_ANALYSE_SOLVE,
    /*! calling specific task, to be defined in inherited module */
    TT_SPEC
};

struct Ttask
{
    Tasktype tasktype;
    QString taskname;
    QString tasklabel;
    /*! when true : calling **spec overloaded methods within inherited jobs */
    bool specific;
    QString devicename;
    QString propertyname;
    QString elementname;
    double value;
    QString text;
    ISState sw;
};
/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
 * It mainly provides a task queue skeleton
*/
class Module : public QObject
{
    Q_OBJECT
    public:
        Module(MyClient *cli);
        ~Module();

        MyClient *indiclient;
        QQueue<Ttask> tasks;
        std::unique_ptr<Image> image =nullptr;
        QList<FITSImage::Star> stars;

        void addnewtask (Tasktype tasktype,  QString taskname, QString tasklabel,bool specific,
                         QString devicename,QString propertyname,QString elementname);
        void addnewtask (Tasktype tasktype,  QString taskname, QString tasklabel,bool specific,
                        QString devicename,QString propertyname,QString elementname,
                        double value,QString text,ISState sw);

        bool taskSendNewNumber(QString deviceName, QString propertyName, QString elementName, double value);
        bool taskSendNewText  (QString deviceName, QString propertyName, QString elementName, QString text);
        bool taskSendNewSwitch(QString deviceName, QString propertyName, QString elementName, ISState sw);
        void executeTask(Ttask task);
        void dumpTasks(void);

        virtual void executeTaskSpec(Ttask task) {Q_UNUSED(task);}
        virtual void executeTaskSpec(Ttask task,IBLOB *bp) {Q_UNUSED(task);Q_UNUSED(bp);}
        virtual void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) {Q_UNUSED(task);Q_UNUSED(nvp);}
        virtual void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) {Q_UNUSED(task);Q_UNUSED(svp);}
        virtual void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) {Q_UNUSED(task);Q_UNUSED(tvp);}
        virtual void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) {Q_UNUSED(task);Q_UNUSED(lvp);}
        void popnext(void);
        void initProperties(void);
        OSTProperties props;
    public slots:
        void gotserverConnected();
        void gotserverDisconnected(int exit_code);
        void gotnewDevice(INDI::BaseDevice *dp);
        void gotremoveDevice(INDI::BaseDevice *dp);
        void gotnewProperty(INDI::Property *property);
        void gotremoveProperty(INDI::Property *property);
        void gotnewText(ITextVectorProperty *tvp);
        void gotnewSwitch(ISwitchVectorProperty *svp);
        void gotnewLight(ILightVectorProperty *lvp);
        void gotnewBLOB(IBLOB *bp);
        void gotnewNumber(INumberVectorProperty *nvp);
        void gotnewMessage(INDI::BaseDevice *dp, int messageID);
        void finishedSEP(void);
        void finishedSolve(void);
        void slotvalueChanged(elem prop);
        void slotpropCreated(elem prop);
        void slotpropDeleted(elem prop);
        virtual void slotvalueChangedFromCtl(elem prop) {Q_UNUSED(prop);}
    signals:
        void finished();
        void signalpropCreated(elem prop);
        void signalpropDeleted(elem prop);
        void signalvalueChanged(elem prop);
        //virtual void taskblob();
}
;
#endif
