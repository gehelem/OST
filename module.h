#ifndef JOB_h_
#define JOB_h_
#include <QtCore>
#include "client.h"
#include "image.h"
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
    const char *devicename;
    const char *propertyname;
    const char *elementname;
    double value;
    const char *text;
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

        MyClient *client;
        QQueue<Ttask> tasks;
        std::unique_ptr<Image> image =nullptr;
        QList<FITSImage::Star> stars;

        void addnewtask (Tasktype tasktype,  QString taskname, QString tasklabel,bool specific,
                        const char *devicename,const char *propertyname,const char *elementname);
        void addnewtask (Tasktype tasktype,  QString taskname, QString tasklabel,bool specific,
                        const char *devicename,const char *propertyname,const char *elementname,
                        double value,const char *text,ISState sw);
        bool taskSendNewNumber(const char *deviceName, const char *propertyName, const char *elementName, double value);
        bool taskSendNewText  (const char *deviceName, const char *propertyName, const char *elementName, const char *text);
        bool taskSendNewSwitch(const char *deviceName, const char *propertyName, const char *elementName, ISState sw);
        void executeTask(Ttask task);
        virtual void executeTaskSpec(Ttask task) {Q_UNUSED(task)}
        virtual void executeTaskSpec(Ttask task,IBLOB *bp) {Q_UNUSED(task);Q_UNUSED(bp);}
        virtual void executeTaskSpec(Ttask task,INumberVectorProperty *nvp) {Q_UNUSED(task);Q_UNUSED(nvp);}
        virtual void executeTaskSpec(Ttask task,ISwitchVectorProperty *svp) {Q_UNUSED(task);Q_UNUSED(svp);}
        virtual void executeTaskSpec(Ttask task,ITextVectorProperty *tvp) {Q_UNUSED(task);Q_UNUSED(tvp);}
        virtual void executeTaskSpec(Ttask task,ILightVectorProperty *lvp) {Q_UNUSED(task);Q_UNUSED(lvp);}
        void popnext(void);
        void setProperties(void);
        QObject getProperties(void);
        QObject props;
    public slots:
        virtual void gotserverConnected();
        virtual void gotserverDisconnected(int exit_code);
        virtual void gotnewDevice(INDI::BaseDevice *dp);
        virtual void gotremoveDevice(INDI::BaseDevice *dp);
        virtual void gotnewProperty(INDI::Property *property);
        virtual void gotremoveProperty(INDI::Property *property);
        virtual void gotnewText(ITextVectorProperty *tvp);
        virtual void gotnewSwitch(ISwitchVectorProperty *svp);
        virtual void gotnewLight(ILightVectorProperty *lvp);
        virtual void gotnewBLOB(IBLOB *bp);
        virtual void gotnewNumber(INumberVectorProperty *nvp);
        virtual void gotnewMessage(INDI::BaseDevice *dp, int messageID);
        void finishedSEP(void);
        void finishedSolve(void);
    signals:
        virtual void finished();
        //virtual void taskblob();
}
;
#endif
