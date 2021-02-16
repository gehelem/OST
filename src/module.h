#ifndef MODULE_h_
#define MODULE_h_
#include <QtCore>
#include "client.h"
#include "image.h"
#include "properties.h"

/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Module : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString  status MEMBER _status NOTIFY statusChanged)

    public:
    protected:
        Module(); /*declared protected to prevent instanciation */
        ~Module();
        MyClient *indiclient;
        std::unique_ptr<Image> image =nullptr;
        QList<FITSImage::Star> stars;
        bool connectIndi(void);
        void connectAllDevices(void);
        void disconnectAllDevices(void);
        void loadDevicesConfs(void);
        bool sendNewNumber(QString deviceName, QString propertyName,QString elementName, double value);
        bool sendNewText  (QString deviceName, QString propertyName,QString elementName, QString text);
        bool sendNewSwitch(QString deviceName,QString propertyName,QString elementName, ISState sw);

        bool frameSet(QString devicename,double x,double y,double width,double height);
        bool frameReset(QString devicename);
        void sendMessage(QString message);

        QString _status;


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
    signals:
        void finished();
        void newMessage(QString message);
        void statusChanged(const QString &newStatus);
    private:
}
;
#endif
