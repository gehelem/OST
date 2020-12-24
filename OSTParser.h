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

QJsonObject Otext  (QString name, QString label,QString text);
QJsonObject Onumber(QString name, QString label,double value);
QJsonObject Onumber(QString name, QString label,double value,QString format,double min,double max,double step);
QJsonObject Oswitch(QString name, QString label, QString swit);
QJsonObject Oswitch(QString name, QString label,ISState swit);
// QJsonObject Olight( TODO
QJsonObject Oproperty(QString name, QString label,QString type, QString permission,
                      QString state, QString parentType, QString parentName,
                      QString switchrule,QJsonArray details,
                      QString modulename,QString categoryname,QString groupname
                      );

QString OGetText(QString name,QJsonArray properties);
double  OGetNumber(QString name, QJsonArray properties);
QJsonArray OSetNumber(std::string name, QJsonArray properties,double value);


#endif
