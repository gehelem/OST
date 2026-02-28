#include "modulejsondumper.h"
namespace  OST
{

QJsonValue ModuleJsonDumper(EvType evt, QVariant data, ElementBase *elt, PropertyBase *prp, LovBase *lov, Datastore *mod)
{
    QJsonObject result, properties, globlovs;

    foreach(const QString &key, mod->getStore().keys())
    {
        OST::PropertyJsonDumper d(evt, data, elt, prp);
        mod->getStore()[key]->accept(&d);
        properties[key] = d.getResult();
    }

    foreach(const QString &key, mod->getGlobLovs().keys())
    {
        OST::LovJsonDumper d;
        mod->getGlobLovs()[key]->accept(&d);
        globlovs[key] = d.getResult();
    }

    result["p"] = properties;
    result["l"] = globlovs;
    result["infos"] = QJsonObject::fromVariantMap(mod->getAllMetadata());;
    return result;
}


}
