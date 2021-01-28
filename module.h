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
        Module(MyClient *cli,Properties *properties);
        ~Module();
        QString modulename;
        MyClient *indiclient;
        Properties *props;
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
        void    createMyModule(QString modulelabel);
        void    deleteMyModule(void);

        void    createMyCateg(QString categname,  QString categlabel);
        void    deleteMyCateg(QString categname);

        void    createMyGroup(QString categname, QString groupname,  QString grouplabel);
        void    deleteMyGroup(QString categname, QString groupname);

        void    createMyProp (QString propname,Prop    prop);
        void    createMyProp (QString propname,QString label,propType typ,QString categoryname,QString groupname,OPerm perm,OSRule rule,double timeout,OPState state,QString aux0,QString aux1);
        void    deleteMyProp (QString propname);
        Prop    getMyProp    (QString propname);
        void    setMyProp    (QString propname,Prop    prop);

        void    appendMyElt  (QString propname,  QString txtname, QString text     , QString label, QString aux0,QString aux1);
        void    appendMyElt  (QString propname,  QString numname, double  num      , QString label, QString aux0,QString aux1);
        void    appendMyElt  (QString propname,  QString swtname, OSState swt      , QString label, QString aux0,QString aux1);
        void    appendMyElt  (QString propname,  QString lgtname, OPState lgt      , QString label, QString aux0,QString aux1);
        void    deleteMyElt  (QString propname,  QString eltname);
        void    setMyElt     (QString propname,  QString txtname, QString text);
        void    setMyElt     (QString propname,  QString numname, double  num);
        void    setMyElt     (QString propname,  QString swtname, OSState swt);
        void    setMyElt     (QString propname,  QString lgtname, OPState lgt);
        QString getMyTxt     (QString propname,  QString txtname);
        double  getMyNum     (QString propname,  QString numname);
        OSState getMySwt     (QString propname,  QString swtname);
        OPState getMyLgt     (QString propname,  QString lgtname);
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
        void slotvalueChanged(Prop prop);
        void slotpropCreated(Prop prop);
        void slotpropDeleted(Prop prop);
        virtual void slotvalueChangedFromCtl(Prop prop) {Q_UNUSED(prop);}
    signals:
        void finished();
        void signalpropCreated(Prop prop);
        void signalpropDeleted(Prop prop);
        void signalvalueChanged(Prop prop);
        //virtual void taskblob();
    protected:

}
;
#endif
