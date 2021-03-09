#ifndef CONTROLLER_h_
#define CONTROLLER_h_

#include <model/setup.h>
#include <utils/newpropertylogger.h>
#include <utils/updatedpropertylogger.h>
#include "focus.h"
#include "wshandler.h"
#include "properties.h"


/*!
 * This class is the heart of OST
 * It dispatches events/orders/datas  from one layer to each other
 * - indiclient
 * - websocket traffic
 * - functional Modules
 */
class Controller : public QObject
{
    Q_OBJECT
public:
    Controller(QObject *parent, bool saveAllBlobs, const QString& host, int port);
    ~Controller() override;
    IndiCLient    *indiclient;
    Properties  *properties;
    FocusModule    *focuser;
    FocusModule    *focuser2;
    WShandler   *wshandler;
public slots:
    void valueChanged(Prop prop);
    void AppendGraph (Prop prop,OGraph gra,OGraphValue val);
    void propCreated (Prop prop);
    void propDeleted (Prop prop);

signals:
    void closed();
private slots:
    void onNewDeviceSeen(const std::string& deviceName);
    void onDeviceRemoved(const std::string& deviceName);
    void onNewBlobReveived(const QByteArray& data, const QString& format) const;
    void onNewPropertyReceived(Property* pProperty);
    void onPropertyUpdated(Property* pProperty);
private:
    Setup _setup;
    bool _appSettingsSaveEveryBlob;
    QString _appSettingsHostName;
    int _appSettingsServerPort;
    NewPropertyLogger _newPropertyLogger;
    UpdatedPropertyLogger _updatedPropertyLogger;
};
#endif
