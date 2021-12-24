#ifndef MODULE_h_
#define MODULE_h_
#include <QtCore>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QObject>
#include <basedevice.h>
#include <baseclient.h>
#include <boost/log/trivial.hpp>
#include "image.h"
#include "solver.h"
#include <model/setup.h>
#include <model/textproperty.h>
#include <model/numberproperty.h>
#include <model/switchproperty.h>
#include <model/messageproperty.h>
#include <model/propertystore.h>
#include "utils/propertyfactory.h"


class Property;

/*!
 * This Class shouldn't be used as is
 * Every functionnal module should inherit it
*/
class Basemodule : public QObject, public INDI::BaseClient
{
    Q_OBJECT

    public:
        Basemodule(QString name, QString label);
        ~Basemodule() = default;
        void setHostport(QString host, int port);
        bool connectIndi(void);
        void setBlobMode(void);
        QString getDescription(void) {return _moduledescription;}

        //std::unique_ptr<Image> image =nullptr;
        QPointer<Image> image;
        Solver _solver;

        QString _modulename;
        QString _modulelabel;
        QString _moduledescription;

    public slots:
        void OnDumpAsked(void);
        virtual void OnSetPropertyText(TextProperty* prop) {
            if (!(prop->getModuleName()==_modulename)) return;
            BOOST_LOG_TRIVIAL(debug) << _modulename.toStdString() << " : recv setprop text : " << prop->getLabel().toStdString();
        }
        virtual void OnSetPropertyNumber(NumberProperty* prop) {
            if (!(prop->getModuleName()==_modulename)) return;
            BOOST_LOG_TRIVIAL(debug) << _modulename.toStdString() << " : recv setprop number : " <<prop->getLabel().toStdString();
        }
        virtual void OnSetPropertySwitch(SwitchProperty* prop) {
            if (!(prop->getModuleName()==_modulename)) return;
            BOOST_LOG_TRIVIAL(debug) << _modulename.toStdString() << " : recv setprop switch : " <<prop->getLabel().toStdString();
        }

    protected:
        bool disconnectIndi(void);
        void connectAllDevices(void);
        void disconnectAllDevices(void);
        void loadDevicesConfs(void);
        bool sendModNewText  (QString deviceName, QString propertyName,QString elementName, QString text);
        bool sendModNewSwitch(QString deviceName, QString propertyName,QString elementName, ISState sw);
        bool sendModNewNumber(const QString& deviceName, const QString& propertyName, const QString& elementName, const double& value);

        bool frameSet(QString devicename,double x,double y,double width,double height);
        bool frameReset(QString devicename);
        void sendMessage(QString message);

        PropertyStore _propertyStore;

        virtual void serverConnected() {}
        virtual void serverDisconnected(int exit_code)          {Q_UNUSED(exit_code);}
        virtual void newDevice(INDI::BaseDevice *dp)            {Q_UNUSED(dp);}
        virtual void removeDevice(INDI::BaseDevice *dp)         {Q_UNUSED(dp);}
        virtual void newProperty(INDI::Property *property)      {Q_UNUSED(property);}
        virtual void removeProperty(INDI::Property *property)   {Q_UNUSED(property);}
        virtual void newText(ITextVectorProperty *tvp)          {Q_UNUSED(tvp);}
        virtual void newSwitch(ISwitchVectorProperty *svp)      {Q_UNUSED(svp);}
        virtual void newLight(ILightVectorProperty *lvp)        {Q_UNUSED(lvp);}
        virtual void newNumber(INumberVectorProperty *nvp)      {Q_UNUSED(nvp);}
        virtual void newBLOB(IBLOB *bp)                         {Q_UNUSED(bp);}
        virtual void newMessage(INDI::BaseDevice *dp, int messageID) {Q_UNUSED(dp);Q_UNUSED(messageID);}
        virtual void newUniversalMessage(std::string message)   {Q_UNUSED(message);}


    signals:
        void propertyCreated(Property* pProperty, QString* pModulename);
        void propertyUpdated(Property* pProperty, QString* pModulename);
        void propertyRemoved(Property* pProperty, QString* pModulename);
        void moduleDumped(QMap<QString, QMap<QString, QMap<QString, Property*>>> treeList, QString* pModulename, QString* pModulelabel, QString* pModuledescription);
        void newMessageSent(QString message,      QString* pModulename, QString Device);

        void finished();
        void statusChanged(const QString &newStatus);
        void askedFrameReset(QString devicename);
}
;
#endif
