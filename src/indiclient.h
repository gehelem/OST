#ifndef CLIENT_h_
#define CLIENT_h_
#include <baseclientqt.h>
/*!
 * indiclient class, overloaded with some basic functions to help us
 * it should be instanciated only once, to keep only one active connection to indiserver :
 * This class is declared as a singleton https://gist.github.com/pazdera/1098119
 * To make sure to keep a unique instance shared by each module
*/
class IndiCLient : public INDI::BaseClientQt
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
    void SigNewProperty(INDI::Property *property);
    void SigRemoveProperty(INDI::Property *property);
    void SigNewText(ITextVectorProperty *tvp);
    void SigNewSwitch(ISwitchVectorProperty *svp);
    void SigNewLight(ILightVectorProperty *lvp);
    void newBlobReceived(const QByteArray& data, QString format);
    void SigNewNumber(INumberVectorProperty *nvp);
    void SigNewMessage(INDI::BaseDevice *dp, int messageID);

    protected:

private:
    /* Private constructor to prevent instancing. */
    IndiCLient();
    ~IndiCLient() = default;
    /* Singleton instance storage. */
    static IndiCLient* instance;

    std::map<int, std::string> _propertyTypesToNamesMap;
    std::map<int, std::string> _propertyStatesToNamesMap;
    std::map<int, std::string> _propertyPermsToNamesMap;
    std::map<int, std::string> _switchRuleToNamesMap;

    std::string extract(ITextVectorProperty* pVector);
    std::string extract(INumberVectorProperty* pVector);
    std::string extract(ISwitchVectorProperty* pVector);
};
#endif
