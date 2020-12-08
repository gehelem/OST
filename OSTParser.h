#ifndef OSTParser_h_
#define OSTParser_h_
#pragma once

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <baseclientqt.h>

/*QJsonDocument  nINDItoJSON(INumberVectorProperty *prop);
QJsonDocument  tINDItoJSON(ITextVectorProperty *prop);
QJsonDocument  sINDItoJSON(ISwitchVectorProperty *prop);
QJsonDocument  lINDItoJSON(ILightVectorProperty *prop);
QJsonDocument  dINDItoJSON(INDI::BaseDevice *d);*/
QJsonObject IProToJson(INDI::Property *Iprop);
std::set<std::string> GroupsArray(INDI::BaseDevice *dev);
QJsonArray IDevToJson(INDI::BaseDevice *dev);

#endif
