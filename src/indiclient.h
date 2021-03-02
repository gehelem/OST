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
    void serverConnected();
    void serverDisconnected(int exit_code);
    void newDevice(INDI::BaseDevice *dp);
    void removeDevice(INDI::BaseDevice *dp);
    void newProperty(INDI::Property *property);
    void removeProperty(INDI::Property *property);
    void newText(ITextVectorProperty *tvp);
    void newSwitch(ISwitchVectorProperty *svp);
    void newLight(ILightVectorProperty *lvp);
    void newBLOB(IBLOB *bp);
    void newNumber(INumberVectorProperty *nvp);
    void newMessage(INDI::BaseDevice *dp, int messageID);

  signals:
    void SigServerConnected();
    void SigServerDisconnected(int exit_code);
    void SigNewDevice(INDI::BaseDevice *dp);
    void SigRemoveDevice(INDI::BaseDevice *dp);
    void SigNewProperty(INDI::Property *property);
    void SigRemoveProperty(INDI::Property *property);
    void SigNewText(ITextVectorProperty *tvp);
    void SigNewSwitch(ISwitchVectorProperty *svp);
    void SigNewLight(ILightVectorProperty *lvp);
    void SigNewBLOB(IBLOB *bp);
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
    std::map<int, std::string> _switchRuleToNamesMap;

    std::string extract(ITextVectorProperty* pVector);
    std::string extract(INumberVectorProperty* pVector);
    std::string extract(ISwitchVectorProperty* pVector);

}
;
#endif
