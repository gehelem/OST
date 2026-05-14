#ifndef OST_MODULEJSONDUMPER_H
#define OST_MODULEJSONDUMPER_H

#include "basemodule.h"
#include <propertyjsondumper.h>

namespace  OST
{

QJsonValue ModuleJsonDumper(OST::EvType evt, QVariant data, OST::ElementBase* elt, OST::PropertyBase* prp,
                            OST::LovBase* lov,
                            Basemodule* mod);

}
#endif //OST_MODULEJSONDUMPER_H
