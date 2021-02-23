#ifndef MODULE_h_
#define MODULE_h_
#include <QtCore>
#include "indiclient.h"
#include "image.h"
/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Module : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString  status MEMBER _status NOTIFY statusChanged)
    Q_PROPERTY(QString  modulename MEMBER _modulename)

    public:
        Module();
        ~Module();
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
        bool sendNewSwitch(QString deviceName,QString propertyName,QString elementName, ISState sw);
        bool sendNewNumber(QString deviceName, QString propertyName,QString elementName, double value);

        bool frameSet(QString devicename,double x,double y,double width,double height);
        bool frameReset(QString devicename);
        void sendMessage(QString message);

        QString _status;
        QString _modulename;


    public slots:
        virtual void IndiServerConnected    (){}
        virtual void IndiServerDisconnected (int exit_code)             {Q_UNUSED(exit_code);}
        virtual void IndiNewDevice          (INDI::BaseDevice *dp)      {Q_UNUSED(dp);}
        virtual void IndiRemoveDevice       (INDI::BaseDevice *dp)      {Q_UNUSED(dp);}
        virtual void IndiNewProperty        (INDI::Property *property)  {Q_UNUSED(property);}
        virtual void IndiRemoveProperty     (INDI::Property *property)  {Q_UNUSED(property);}
        virtual void IndiNewText            (ITextVectorProperty *tvp)  {Q_UNUSED(tvp);}
        virtual void IndiNewSwitch          (ISwitchVectorProperty *svp){Q_UNUSED(svp);}
        virtual void IndiNewLight           (ILightVectorProperty *lvp) {Q_UNUSED(lvp);}
        virtual void IndiNewMessage         (INDI::BaseDevice *dp, int messageID){Q_UNUSED(dp);Q_UNUSED(messageID);}
        virtual void IndiNewNumber          (INumberVectorProperty *nvp){Q_UNUSED(nvp);}
        virtual void IndiNewBLOB            (IBLOB *bp)                 {Q_UNUSED(bp);}
    signals:
        void finished();
        void newMessage(QString message);
        void statusChanged(const QString &newStatus);
        void askedFrameReset(QString devicename);
    private:

}
;
#endif
