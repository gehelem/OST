#ifndef JOB_h_
#define JOB_h_
#include <QtCore>
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include "client.h"
#include "image.h"
enum Tasktype
{
    TT_WAIT_PROP,
    TT_WAIT_NUMBER,
    TT_WAIT_TEXT,
    TT_WAIT_SWITCH,
    TT_WAIT_SEP,
    TT_WAIT_SOLVE,
    TT_WAIT_BLOB,
    TT_WAIT_LIGHT,
    TT_SEND_NUMBER,
    TT_SEND_TEXT,
    TT_SEND_SWITCH,
    TT_ANALYSE_SEP,
    TT_ANALYSE_SOLVE,
    TT_SPEC
};

struct Ttask
{
    Tasktype tasktype;
    QString taskname;
    QString tasklabel;
    bool specific;              // when true : calling **spec overloaded methods within inherited jobs
    const char *devicename;
    const char *propertyname;
    const char *elementname;
    double value;
    const char *text;
    ISState sw;
};
class Job : public QObject
{
    Q_OBJECT
    public:
        Job(MyClient *cli);
        ~Job();

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
