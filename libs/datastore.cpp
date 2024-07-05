#include <QtCore>
#include "datastore.h"

Datastore::Datastore()
{
    //sendMessage("DataStore init");
}
Datastore::~Datastore()
{
}
OST::PropertyMulti* Datastore::getProperty(QString pProperty)
{
    if (!mStore.contains(pProperty))
    {
        sendWarning(" getProperty - property " + pProperty + " not found");
        //return nullptr;
    }

    return mStore[pProperty];
}
OST::ElementBase* Datastore::getEltBase(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getEltString - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltString - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    return p->getElt(pElement);
}

OST::ElementString* Datastore::getEltString(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getEltString - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltString - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "string")
    {
        sendWarning("getEltString - property " + pProperty + " : element " + pElement + " is not string");
        return nullptr;
    }
    return static_cast<OST::ElementString*>(p->getElt(pElement));
}
OST::ElementPrg* Datastore::getEltPrg(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getEltPrg - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltPrg - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "prg")
    {
        sendWarning("getEltPrg - property " + pProperty + " : element " + pElement + " is not progress");
        return nullptr;
    }
    return static_cast<OST::ElementPrg*>(p->getElt(pElement));
}
OST::ElementMessage* Datastore::getEltMsg(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getEltMsg - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltMsg - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "prg")
    {
        sendWarning("getEltMsg - property " + pProperty + " : element " + pElement + " is not message");
        return nullptr;
    }
    return static_cast<OST::ElementMessage*>(p->getElt(pElement));
}

QString Datastore::getString(QString pProperty, QString pElement)
{
    return getEltString(pProperty, pElement)->value();
}
QString Datastore::getString(QString pProperty, QString pElement, long line)
{
    if (getProperty(pProperty)->getGrid().size() < line + 1)
    {
        sendWarning("getString - property " + pProperty + " line : " + line + " does not exist" );
        return QString();
    }
    OST::ValueString* v = static_cast<OST::ValueString*>(getProperty(pProperty)->getGrid()[line][pElement]);
    return v->value;
}
OST::ElementInt* Datastore::getEltInt(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getEltInt - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltInt - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "int")
    {
        sendWarning("getEltInt - property " + pProperty + " : element " + pElement + " is not int");
        return nullptr;
    }
    return static_cast<OST::ElementInt*>(p->getElt(pElement));
}
long Datastore::getInt(QString pProperty, QString pElement)
{
    return getEltInt(pProperty, pElement)->value();
}
long Datastore::getInt(QString pProperty, QString pElement, long line)
{
    if (getProperty(pProperty)->getGrid().size() < line + 1)
    {
        sendWarning("getInt- property " + pProperty + " line : " + line + " does not exist" );
        return 0;
    }
    OST::ValueInt* v = static_cast<OST::ValueInt*>(getProperty(pProperty)->getGrid()[line][pElement]);
    return v->value;
}
OST::ElementFloat* Datastore::getEltFloat(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getEltFloat - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltFloat - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "float")
    {
        sendWarning("getEltFloat - property " + pProperty + " : element " + pElement + " is not float");
        return nullptr;
    }
    return static_cast<OST::ElementFloat*>(p->getElt(pElement));
}
double  Datastore::getFloat(QString pProperty, QString pElement)
{
    return getEltFloat(pProperty, pElement)->value();
}
double Datastore::getFloat(QString pProperty, QString pElement, long line)
{
    if (getProperty(pProperty)->getGrid().size() < line + 1)
    {
        sendWarning("getFloat - property " + pProperty + " line : " + line + " does not exist" );
        return 0;
    }
    OST::ValueFloat* v = static_cast<OST::ValueFloat*>(getProperty(pProperty)->getGrid()[line][pElement]);
    return v->value;
}
OST::ElementLight* Datastore::getEltLight(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getEltLight - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltLight - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "light")
    {
        sendWarning("getEltLight - property " + pProperty + " : element " + pElement + " is not light");
        return nullptr;
    }
    return static_cast<OST::ElementLight*>(p->getElt(pElement));
}

OST::ElementImg* Datastore::getEltImg(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getEltImg - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltImg - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "img")
    {
        sendWarning("getEltImg - property " + pProperty + " : element " + pElement + " is not img");
        return nullptr;
    }
    return static_cast<OST::ElementImg*>(p->getElt(pElement));
}
OST::ElementVideo* Datastore::getEltVideo(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getEltVideo - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("getEltVideo - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "video")
    {
        sendWarning("getEltVideo - property " + pProperty + " : element " + pElement + " is not video");
        return nullptr;
    }
    return static_cast<OST::ElementVideo*>(p->getElt(pElement));
}
OST::ElementBool* Datastore::getEltBool(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("ValueBool - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        sendWarning("ValueBool - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "bool")
    {
        sendWarning("ValueBool - property " + pProperty + " : element " + pElement + " is not bool");
        return nullptr;
    }
    return static_cast<OST::ElementBool*>(p->getElt(pElement));
}
bool Datastore::getBool(QString pProperty, QString pElement)
{
    return getEltBool(pProperty, pElement)->value();
}
bool Datastore::getBool(QString pProperty, QString pElement, long line)
{
    if (getProperty(pProperty)->getGrid().size() < line + 1)
    {
        sendWarning("getBool - property " + pProperty + " line : " + line + " does not exist" );
        return 0;
    }
    OST::ValueBool* v = static_cast<OST::ValueBool*>(getProperty(pProperty)->getGrid()[line][pElement]);
    return v->value;
}

bool Datastore::createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel,
                                  const int &pPropertyPermission,
                                  const  QString &pPropertyDevcat, const QString &pPropertyGroup)
{
    if (mStore.contains(pPropertyName))
    {
        sendWarning("createOstProperty - property " + pPropertyName + " already exists.");
        return false;
    }
    QVariantMap prop;
    prop["label"] = pPropertyLabel;
    prop["permission"] = pPropertyPermission;
    prop["devcat"] = pPropertyDevcat;
    prop["group"] = pPropertyGroup;
    prop["name"] = pPropertyName;
    prop["elements"] = QVariantMap();

    OST::PropertyMulti *pm = OST::PropertyFactory::createProperty(pPropertyName, prop);
    mStore[pPropertyName] = pm;
    OST::PropertyJsonDumper d;
    pm->accept(&d);
    connect(mStore[pPropertyName], &OST::PropertyMulti::valueSet, this, &Datastore::onValueSet);
    connect(mStore[pPropertyName], &OST::PropertyMulti::eltChanged, this, &Datastore::onEltChanged);
    connect(mStore[pPropertyName], &OST::PropertyMulti::propertyEvent, this, &Datastore::onPropertyEvent);
    connect(mStore[pPropertyName], &OST::PropertyMulti::sendMessage, this, &Datastore::onPropertyMessage);
    OnModuleEvent("cp", QString(), pPropertyName, d.getResult().toVariantMap());
    return true;
}
void Datastore::emitPropertyCreation(const QString &pPropertyName)
{
    OST::PropertyJsonDumper d;
    QVariantMap result;
    mStore[pPropertyName]->accept(&d, result);
    emit OnModuleEvent("cp", QString(), pPropertyName, result);
}
void Datastore::onValueSet(void)
{
    OST::PropertyBase* obj = qobject_cast<OST::PropertyBase*>(sender());
    OST::PropertyJsonDumper d;
    obj->accept(&d);
    OnModuleEvent("se", QString(), obj->key(), d.getResult().toVariantMap());

}
void Datastore::onEltChanged(void)
{
    OST::PropertyBase* obj = qobject_cast<OST::PropertyBase*>(sender());
    OST::PropertyJsonDumper d;
    obj->accept(&d);
    OnModuleEvent("ap", QString(), obj->key(), d.getResult().toVariantMap());

}
void Datastore::onPropertyEvent(QString event, QString key, OST::PropertyBase* prop)
{
    Q_UNUSED(event) Q_UNUSED(key) Q_UNUSED(prop)
    OST::PropertyBase* obj = qobject_cast<OST::PropertyBase*>(sender());
    OST::PropertyJsonDumper d;
    obj->accept(&d);
    OnModuleEvent("ap", QString(), obj->key(), d.getResult().toVariantMap());
}
void Datastore::onLovChanged()
{
    OST::LovBase* lov = qobject_cast<OST::LovBase*>(sender());
    OST::LovJsonDumper d;
    lov->accept(&d);
    OnModuleEvent("lc", QString(), lov->getKey(), d.getResult().toVariantMap());
}

bool Datastore::createOstElementText(const QString &pPropertyName, const QString &pElementName,
                                     const QString &pElementLabel,
                                     bool mEmitEvent)
{
    if (!mStore.contains(pPropertyName) )
    {
        sendWarning(" createOstElementText - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap pData;
    pData["label"] = pElementLabel;
    pData["type"] = "string";
    OST::ElementBase *el = OST::ElementFactory::createElement(pData);

    mStore[pPropertyName]->addElt(pElementName, el);
    OST::PropertyJsonDumper d;
    mStore[pPropertyName]->accept(&d);
    //qDebug() << "createOstElementText el(" << pElementName << ")=" << d.getResult();
    if (mEmitEvent) OnModuleEvent("ce", QString(), pPropertyName, d.getResult().toVariantMap());
    return true;

}
bool Datastore::createOstElementBool(const QString &pPropertyName, const QString &pElementName,
                                     const QString &pElementLabel,
                                     bool mEmitEvent)
{
    if (!mStore.contains(pPropertyName) )
    {
        sendWarning(" createOstElementBool - property " + pPropertyName + " not found");
        return false;
    }
    QVariantMap pData;
    pData["label"] = pElementLabel;
    pData["type"] = "bool";

    OST::ElementBase *el = OST::ElementFactory::createElement(pData);

    mStore[pPropertyName]->addElt(pElementName, el);
    OST::PropertyJsonDumper d;
    mStore[pPropertyName]->accept(&d);
    //qDebug() << "createOstElementBool el(" << pElementName << ")=" << d.getResult();
    if (mEmitEvent) OnModuleEvent("ce", QString(), pPropertyName, d.getResult().toVariantMap());
    return true;

}
bool Datastore::setOstElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                   bool mEmitEvent)
{
    Q_UNUSED(mEmitEvent)
    if (!mStore.contains(pPropertyName))
    {
        sendWarning("setElementValue - property2 " + pPropertyName + " not found");
        return false;
    }
    OST::PropertyMulti *pb = mStore[pPropertyName];
    if (!mStore[pPropertyName]->getElts()->contains(pElementName))
    {
        sendWarning("setElementValue - property2 " + pPropertyName + " : element " + pElementName + " not found.");
        return false;
    }
    return pb->setElt(pElementName, pElementValue);
}
QVariantList Datastore::getOstElementGrid(const QString &pPropertyName, const QString &pElementName)
{
    if (!mStore.contains(pPropertyName))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " not found.");
        return QVariantList();
    }
    if (mStore[pPropertyName]->getElts()->size() == 0)
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " contains no elements.");
        return QVariantList();
    }
    if (!mStore[pPropertyName]->hasGrid())
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " has no grid.");
        return QVariantList();
    }
    OST::ElementUpdate v;
    QString action = "getgrid";
    QVariantMap m;
    mStore[pPropertyName]->getElt(pElementName)->accept(&v, action, m);
    return v.getGrid();


}

void Datastore::loadOstPropertiesFromFile(const QString &pFileName)
{
    QString content;
    QFile file;
    file.setFileName(pFileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    content = file.readAll();
    file.close();
    QJsonParseError parseError;
    QJsonDocument d = QJsonDocument::fromJson(content.toUtf8(), &parseError);
    if (!(parseError.error == QJsonParseError::NoError))
    {
        qDebug() << "Json parse error " << pFileName << "-" << parseError.errorString();
    }

    QJsonObject data = d.object();
    if (data.contains("properties"))
    {
        QJsonObject props = data["properties"].toObject();
        foreach(const QString &key, props.keys())
        {
            //qDebug() << "***** create property from file " << key;
            QVariantMap tt = props[key].toVariant().toMap();
            OST::PropertyMulti *rp = OST::PropertyFactory::createProperty(key, tt);
            if (rp != nullptr)
            {
                mStore[key] = rp;
                connect(rp, &OST::PropertyMulti::valueSet, this, &Datastore::onValueSet);
                connect(rp, &OST::PropertyMulti::eltChanged, this, &Datastore::onEltChanged);
                connect(rp, &OST::PropertyMulti::propertyEvent, this, &Datastore::onPropertyEvent);
                connect(rp, &OST::PropertyMulti::sendMessage, this, &Datastore::onPropertyMessage);
                mStore[key]->setState(OST::State::Idle);

            }
            else
            {
                qDebug() << "***** can't create property " << key;
            }

        }


    }
    if (data.contains("globallov"))
    {
        QJsonObject lovs = data["globallov"].toObject();
        foreach(const QString &key, lovs.keys())
        {
            QVariantMap tt = lovs[key].toVariant().toMap();
            OST::LovBase *lb = OST::LovFactory::createLov(tt);
            if (lb != nullptr)
            {
                mGlobLov[key] = lb;
                OST::LovJsonDumper d;
                mGlobLov[key]->accept(&d);
            }
            else
            {
                qDebug() << "***** can't create globlov " << key;
            }

        }


    }


}

void Datastore::saveOstPropertiesToFile(const QString &pFileName)
{
    QJsonObject obj = QJsonObject::fromVariantMap(getPropertiesDump().toVariantMap());
    QJsonDocument doc(obj);

    QFile jsonFile(pFileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();
}

QVariantMap Datastore::getProfile(void)
{
    QVariantMap result;
    QVariantMap propertiesResult;
    QVariantMap props = getPropertiesDump().toVariantMap();
    result["globallovs"] = getGlobalLovsDump().toVariantHash();


    foreach(const QString &keyprop, props.keys())
    {
        if (props[keyprop].toMap().contains("hasprofile"))
        {
            if (props[keyprop].toMap()["hasprofile"].toBool())
            {
                QVariantMap property;
                if (props[keyprop].toMap().contains("elements"))
                {
                    QVariantMap element, elements;
                    foreach(const QString &keyelt, props[keyprop].toMap()["elements"].toMap().keys())
                    {
                        if (props[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("value"))
                        {
                            element["value"] = props[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                        }
                        if (props[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("min"))
                        {
                            element["min"] = props[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["min"];
                        }
                        if (props[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("max"))
                        {
                            element["max"] = props[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["max"];
                        }
                        if (props[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("step"))
                        {
                            element["step"] = props[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["step"];
                        }
                        if (props[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("gridvalues"))
                        {
                            element["gridvalues"] = props[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["gridvalues"];
                        }
                        elements[keyelt] = element;
                    }
                    property["elements"] = elements;
                }
                propertiesResult[keyprop] = property;
            }

        }
    }

    result["properties"] = propertiesResult;
    return result;
}

void Datastore::deleteOstProperty(const QString &pPropertyName)
{
    if (!mStore.contains(pPropertyName))
    {
        sendWarning("Can't delete inexistant property " + pPropertyName);
        return;
    }
    mStore.remove(pPropertyName);
    emit OnModuleEvent("delprop", QString(), pPropertyName, QVariantMap());

}



QJsonObject Datastore::getPropertiesDump(void)
{
    QJsonObject properties;
    foreach(const QString &key, mStore.keys())
    {
        OST::PropertyJsonDumper d;
        mStore[key]->accept(&d);
        properties[key] = d.getResult();
    }
    return properties;
}
QJsonObject Datastore::getGlobalLovsDump(void)
{
    QJsonObject globallov;
    foreach(const QString &key, mGlobLov.keys())
    {
        OST::LovJsonDumper d;
        mGlobLov[key]->accept(&d);
        globallov[key] = d.getResult();
    }
    return globallov;
}
OST::LovString* Datastore::getGlovString(QString pLov)
{
    if (!getGlobLovs().contains(pLov))
    {
        sendWarning(" getGlovString - Globallob " + pLov + " not found");
        return nullptr;
    }
    if (getGlobLovs()[pLov]->getType() != "string")
    {
        sendWarning(" getGlovString - Globallob " + pLov + " wrong type " + getGlobLovs()[pLov]->getType());
        return nullptr;
    }

    return static_cast<OST::LovString*>(getGlobLovs()[pLov]);
}
void Datastore::onPropertyMessage(OST::MsgLevel l, QString m)
{
    switch (l)
    {
        case OST::Info:
            sendMessage(m);
            break;
        case OST::Warn:
            sendWarning(m);
            break;
        case OST::Err:
            sendError(m);
            break;
        default:
            sendError(m);
            break;
    }

}
