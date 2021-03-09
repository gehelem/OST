#ifndef CLIENT_h_
#define CLIENT_h_
#include <baseclient.h>
#include <qobject.h>

class Property;

Q_DECLARE_METATYPE(std::string)

/*!
 * indiclient class, overloaded with some basic functions to help us
 * it should be instanciated only once, to keep only one active connection to indiserver :
 * This class is declared as a singleton https://gist.github.com/pazdera/1098119
 * To make sure to keep a unique instance shared by each module
*/
class IndiCLient : public QObject, public INDI::BaseClient
{
    Q_OBJECT
  public:
    /* Singleton : Static access method. */
    static IndiCLient* getInstance();
    void serverConnected() override;
    void serverDisconnected(int exit_code) override;
    void newDevice(INDI::BaseDevice *dp) override;
    void removeDevice(INDI::BaseDevice *dp) override;
    void newProperty(INDI::Property*) override;
    void removeProperty(INDI::Property *property) override;
    void newText(ITextVectorProperty *tvp) override;
    void newSwitch(ISwitchVectorProperty *svp) override;
    void newLight(ILightVectorProperty *lvp) override;
    void newBLOB(IBLOB *bp) override;
    void newNumber(INumberVectorProperty *nvp) override;
    void newMessage(INDI::BaseDevice *dp, int messageID) override;

  signals:
    void SigServerConnected();
    void SigServerDisconnected(int exit_code);
    void newDeviceSeen(std::string);
    void deviceRemoved(std::string);
    void newPropertyReceived(Property* pProperty);
    void propertyUpdated(Property* pProperty);
    void SigRemoveProperty(INDI::Property *property);
    void SigNewText(ITextVectorProperty *tvp);
    void SigNewSwitch(ISwitchVectorProperty *svp);
    void SigNewLight(ILightVectorProperty *lvp);
    void newBlobReceived(const QByteArray& data, QString format);
    void SigNewMessage(INDI::BaseDevice *dp, int messageID);

    protected:

private:
    /* Private constructor to prevent instancing. */
    IndiCLient();
    ~IndiCLient() override = default;
    /* Singleton instance storage. */
    static IndiCLient* instance;

    std::map<int, std::string> _propertyTypesToNamesMap;
    std::map<int, std::string> _switchRuleToNamesMap;

    std::string extract(ITextVectorProperty* pVector);
    std::string extract(ISwitchVectorProperty* pVector);
    std::string extract(ILightVectorProperty* pVector);
};
#endif
