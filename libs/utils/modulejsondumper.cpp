#include "modulejsondumper.h"
namespace  OST
{

QJsonValue ModuleJsonDumper(EvType evt, QVariant data, ElementBase *elt, PropertyBase *prp, LovBase *lov, Datastore *mod)
{
    QJsonObject result, properties, globlovs;
    if (evt == EvType::dp)
    {
        properties[data.toString()] = "";
        result["p"] = properties;
        return result;
    }

    if (evt == EvType::av || evt == EvType::pr)
    {
        foreach(const QString &key, mod->getStore().keys())
        {
            if (evt == EvType::av || mod->getStore()[key]->hasProfile())
            {
                OST::PropertyJsonDumper d(evt, data, elt, prp);
                mod->getStore()[key]->accept(&d);
                properties[key] = d.getResult();
            }
        }
        result["p"] = properties;
        return result;
    }

    if (prp)
    {
        OST::PropertyJsonDumper d(evt, data, elt, prp);
        mod->getStore()[prp->key()]->accept(&d);
        properties[prp->key()] = d.getResult();
        result["p"] = properties;
    }
    else
    {
        if (!lov && (evt == EvType::aa || evt == EvType::am))
        {
            foreach(const QString &key, mod->getStore().keys())
            {
                OST::PropertyJsonDumper d(evt, data, elt, prp);
                mod->getStore()[key]->accept(&d);
                properties[key] = d.getResult();
            }
            result["p"] = properties;
        }
    }

    if (lov)
    {
        OST::LovJsonDumper d;
        lov->accept(&d);
        globlovs[lov->getKey()] = d.getResult();
        result["l"] = globlovs;
    }
    else
    {
        if (!prp && (evt == EvType::aa || evt == EvType::am))
        {
            foreach(const QString &key, mod->getGlobLovs().keys())
            {
                OST::LovJsonDumper d;
                mod->getGlobLovs()[key]->accept(&d);
                globlovs[key] = d.getResult();
            }
            result["l"] = globlovs;
        }
    }
    //result["infos"] = QJsonObject::fromVariantMap(mod->getAllMetadata());;

    return result;
}


}
