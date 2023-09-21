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

OST::ValueString* Datastore::getValueString(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getValueString - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getValueString - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getValue(pElement)->getType() != "string")
    {
        sendWarning("getValueString - property " + pProperty + " : element " + pElement + " is not string");
        return nullptr;
    }
    return static_cast<OST::ValueString*>(p->getValue(pElement));
}
QString Datastore::getString(QString pProperty, QString pElement)
{
    return getValueString(pProperty, pElement)->value();
}
QString Datastore::getString(QString pProperty, QString pElement, long line)
{
    return getValueString(pProperty, pElement)->getGrid()[line];
}
OST::ValueInt* Datastore::getValueInt(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getValueInt - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getValueInt - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getValue(pElement)->getType() != "int")
    {
        sendWarning("getValueInt - property " + pProperty + " : element " + pElement + " is not int");
        return nullptr;
    }
    return static_cast<OST::ValueInt*>(p->getValue(pElement));
}
long Datastore::getInt(QString pProperty, QString pElement)
{
    return getValueInt(pProperty, pElement)->value();
}
long Datastore::getInt(QString pProperty, QString pElement, long line)
{
    return getValueInt(pProperty, pElement)->getGrid()[line];
}
OST::ValueFloat* Datastore::getValueFloat(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getValueFloat - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getValueFloat - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getValue(pElement)->getType() != "float")
    {
        sendWarning("getValueFloat - property " + pProperty + " : element " + pElement + " is not float");
        return nullptr;
    }
    return static_cast<OST::ValueFloat*>(p->getValue(pElement));
}
double  Datastore::getFloat(QString pProperty, QString pElement)
{
    return getValueFloat(pProperty, pElement)->value();
}
double Datastore::getFloat(QString pProperty, QString pElement, long line)
{
    return getValueFloat(pProperty, pElement)->getGrid()[line];
}
OST::ValueLight* Datastore::getValueLight(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getValueLight - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getValueLight - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getValue(pElement)->getType() != "light")
    {
        sendWarning("getValueLight - property " + pProperty + " : element " + pElement + " is not light");
        return nullptr;
    }
    return static_cast<OST::ValueLight*>(p->getValue(pElement));
}

OST::ValueImg* Datastore::getValueImg(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getValueImg - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getValueImg - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getValue(pElement)->getType() != "img")
    {
        sendWarning("getValueImg - property " + pProperty + " : element " + pElement + " is not img");
        return nullptr;
    }
    return static_cast<OST::ValueImg*>(p->getValue(pElement));
}
OST::ValueVideo* Datastore::getValueVideo(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getValueVideo - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getValueVideo - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getValue(pElement)->getType() != "video")
    {
        sendWarning("getValueVideo - property " + pProperty + " : element " + pElement + " is not video");
        return nullptr;
    }
    return static_cast<OST::ValueVideo*>(p->getValue(pElement));
}
OST::ValueBool* Datastore::getValueBool(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("ValueBool - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("ValueBool - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    if (p->getValue(pElement)->getType() != "bool")
    {
        sendWarning("ValueBool - property " + pProperty + " : element " + pElement + " is not bool");
        return nullptr;
    }
    return static_cast<OST::ValueBool*>(p->getValue(pElement));
}
bool Datastore::getBool(QString pProperty, QString pElement)
{
    return getValueBool(pProperty, pElement)->value();
}
bool Datastore::getBool(QString pProperty, QString pElement, long line)
{
    return getValueBool(pProperty, pElement)->getGrid()[line];
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
    connect(mStore[pPropertyName], &OST::PropertyMulti::valueChanged, this, &Datastore::onValueChanged);
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
void Datastore::onValueChanged(void)
{
    OST::PropertyBase* obj = qobject_cast<OST::PropertyBase*>(sender());
    OST::PropertyJsonDumper d;
    obj->accept(&d);
    OnModuleEvent("se", QString(), obj->key(), d.getResult().toVariantMap());

}
void Datastore::onPropertyEvent(QString event, QString key, OST::PropertyBase* prop)
{
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
    OST::ValueBase *el = OST::ValueFactory::createValue(pData);

    mStore[pPropertyName]->addValue(pElementName, el);
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

    OST::ValueBase *el = OST::ValueFactory::createValue(pData);

    mStore[pPropertyName]->addValue(pElementName, el);
    OST::PropertyJsonDumper d;
    mStore[pPropertyName]->accept(&d);
    //qDebug() << "createOstElementBool el(" << pElementName << ")=" << d.getResult();
    if (mEmitEvent) OnModuleEvent("ce", QString(), pPropertyName, d.getResult().toVariantMap());
    return true;

}
bool Datastore::setOstElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                   bool mEmitEvent)
{
    if (!mStore.contains(pPropertyName))
    {
        sendWarning("setElementValue - property2 " + pPropertyName + " not found");
        return false;
    }
    OST::PropertyMulti *pb = mStore[pPropertyName];
    if (!mStore[pPropertyName]->getValues().contains(pElementName))
    {
        sendWarning("setElementValue - property2 " + pPropertyName + " : element " + pElementName + " not found.");
        return false;
    }
    return pb->setValue(pElementName, pElementValue);
}
QVariantList Datastore::getOstElementGrid(const QString &pPropertyName, const QString &pElementName)
{
    if (!mStore.contains(pPropertyName))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " not found.");
        return QVariantList();
    }
    if (mStore[pPropertyName]->getValues().size() == 0)
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " contains no elements.");
        return QVariantList();
    }
    if (!mStore[pPropertyName]->hasArray())
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " has no grid.");
        return QVariantList();
    }
    OST::ValueUpdate v;
    QString action = "getgrid";
    QVariantMap m;
    mStore[pPropertyName]->getValue(pElementName)->accept(&v, action, m);
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
                connect(rp, &OST::PropertyMulti::valueChanged, this, &Datastore::onValueChanged);
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
