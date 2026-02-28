#include "modulejsondumper.h"
namespace  OST
{

QJsonValue ModuleJsonDumper(EvType evt, QVariant data, ElementBase *elt, PropertyBase *prp, LovBase *lov, Datastore *mod)
{
    QJsonObject result, properties, globlovs;

    if (prp)
    {
        OST::PropertyJsonDumper d(evt, data, elt, prp);
        mod->getStore()[prp->key()]->accept(&d);
        properties[prp->key()] = d.getResult();
    }
    else
    {
        foreach(const QString &key, mod->getStore().keys())
        {
            OST::PropertyJsonDumper d(evt, data, elt, prp);
            mod->getStore()[key]->accept(&d);
            properties[key] = d.getResult();
        }
    }

    if (lov)
    {
        foreach(const QString &key, mod->getGlobLovs().keys())
        {
            OST::LovJsonDumper d;
            mod->getGlobLovs()[key]->accept(&d);
            globlovs[key] = d.getResult();
        }
    }
    result["p"] = properties;
    //result["l"] = globlovs;
    //result["infos"] = QJsonObject::fromVariantMap(mod->getAllMetadata());;
    return result;
}


}
