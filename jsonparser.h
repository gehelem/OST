#ifndef PARSER_H_
#define PARSER_H_

#include "properties.h"
#include <basedevice.h>


QJsonObject OtextToJ  (OText   txt);
QJsonObject OnumToJ   (ONumber num);
QJsonObject OswitchToJ(OSwitch swt);
QJsonObject OlightToJ (OLight  lgt);
QJsonObject OpropToJ(Prop prop);



#endif
