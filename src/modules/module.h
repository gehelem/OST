#ifndef MODULE_h_
#define MODULE_h_
#include <QtCore>
#include "indiclient.h"
#include "image.h"
#include "properties.h"
/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/


class Module : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString          status      MEMBER _status NOTIFY statusChanged)
    Q_PROPERTY(QString          modulename  MEMBER _modulename)
    //Q_PROPERTY(QMap<QString,bool>     actions   )
    //Q_PROPERTY(QMap<QString,QString>    devices READ getDevices WRITE setDevices)

    public:
        Module();
        ~Module();
        QMap<QString,QString> getDevices(void);
        void setDevices(QMap<QString,QString>);
        void setAction(QString);

    protected:
        IndiCLient *indiclient;
        std::unique_ptr<Image> image =nullptr;
        //QList<FITSImage::Star> stars;
        bool connectIndi(void);
        bool disconnectIndi(void);
        void connectAllDevices(void);
        void disconnectAllDevices(void);
        void loadDevicesConfs(void);
        bool sendNewText  (QString deviceName, QString propertyName,QString elementName, QString text);
        bool sendNewSwitch(QString deviceName, QString propertyName,QString elementName, ISState sw);
        bool sendNewNumber(const QString& deviceName, const QString& propertyName, const QString& elementName, const double& value);

        bool frameSet(QString devicename,double x,double y,double width,double height);
        bool frameReset(QString devicename);
        void sendMessage(QString message);

        QString     _status;
        QString     _modulename;
        QMap<QString,bool>        _actions;
        QMap<QString,QString>     _devices;


    public slots:
        virtual void OnIndiServerConnected    (){}
        virtual void OnIndiServerDisconnected () {}
        virtual void OnIndiNewDevice          (INDI::BaseDevice *dp)      {Q_UNUSED(dp);}
        virtual void OnIndiRemoveDevice       (INDI::BaseDevice *dp)      {Q_UNUSED(dp);}
        virtual void OnIndiNewProperty        (INDI::Property *property)  {Q_UNUSED(property);}
        virtual void OnIndiRemoveProperty     (INDI::Property *property)  {Q_UNUSED(property);}
        virtual void OnIndiNewText            (ITextVectorProperty *tvp)  {Q_UNUSED(tvp);}
        virtual void OnIndiNewSwitch          (ISwitchVectorProperty *svp){Q_UNUSED(svp);}
        virtual void OnIndiNewLight           (ILightVectorProperty *lvp) {Q_UNUSED(lvp);}
        virtual void OnIndiNewMessage         (INDI::BaseDevice *dp, int messageID){Q_UNUSED(dp);Q_UNUSED(messageID);}
        virtual void OnIndiNewNumber          (INumberVectorProperty *nvp){Q_UNUSED(nvp);}
        virtual void OnIndiNewBLOB            (IBLOB *bp)                 {Q_UNUSED(bp);}
    signals:
        void finished();
        void newMessage(QString message);
        void statusChanged(const QString &newStatus);
        void askedFrameReset(QString devicename);
    private:

}
;
#endif
