#ifndef OSTClientCTL_h_
#define OSTClientCTL_h_
#pragma once

//QT Includes
//QT Includes
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtNetwork>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <QJsonObject>
#include <QJsonDocument>
#include <OSTClient.h>

class OSTClientCTL : public OSTClient
{
  public:
    OSTClientCTL(QObject *parent = Q_NULLPTR);
    virtual ~OSTClientCTL();

private:

signals:


};

#endif
