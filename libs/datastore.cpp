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
    return static_cast<OST::ValueString*>(p->getValue(pElement));
}
QString Datastore::getString(QString pProperty, QString pElement)
{
    return getValueString(pProperty, pElement)->value();
}
QString Datastore::getString(QString pProperty, QString pElement, long line)
{
    return getValueString(pProperty, pElement)->grid.getGrid()[line];
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
    return static_cast<OST::ValueInt*>(p->getValue(pElement));
}
long Datastore::getInt(QString pProperty, QString pElement)
{
    return getValueInt(pProperty, pElement)->value();
}
long Datastore::getInt(QString pProperty, QString pElement, long line)
{
    return getValueInt(pProperty, pElement)->grid.getGrid()[line];
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
    return static_cast<OST::ValueFloat*>(p->getValue(pElement));
}
double  Datastore::getFloat(QString pProperty, QString pElement)
{
    return getValueFloat(pProperty, pElement)->value();
}
double Datastore::getFloat(QString pProperty, QString pElement, long line)
{
    return getValueFloat(pProperty, pElement)->grid.getGrid()[line];
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
    return static_cast<OST::ValueImg*>(p->getValue(pElement));
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
    return static_cast<OST::ValueBool*>(p->getValue(pElement));
}
bool Datastore::getBool(QString pProperty, QString pElement)
{
    return getValueBool(pProperty, pElement)->value();
}
bool Datastore::getBool(QString pProperty, QString pElement, long line)
{
    return getValueBool(pProperty, pElement)->grid.getGrid()[line];
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
    OST::ValueUpdate vu;
    QVariantMap m;
    m["value"] = pElementValue;
    pb->getValue(pElementName)->accept(&vu, m);
    return true;
}
QVariantList Datastore::getOstElementGrid(const QString &pPropertyName, const QString &pElementName)
{
    if (!mStore.contains(pPropertyName))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " not found.");
        return QVariantList();
    }
    if (!mStore[pPropertyName]->getValues().contains("elements"))
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
    QJsonDocument d = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject props = d.object();
    foreach(const QString &key, props.keys())
    {
        QVariantMap tt = props[key].toVariant().toMap();
        //qDebug() << "***** create property from file " << key;
        OST::PropertyMulti *rp = OST::PropertyFactory::createProperty(key, tt);
        if (rp != nullptr)
        {
            mStore[key] = rp;
            connect(rp, &OST::PropertyMulti::valueChanged, this, &Datastore::onValueChanged);
            connect(rp, &OST::PropertyMulti::propertyEvent, this, &Datastore::onPropertyEvent);
            mStore[key]->setState(OST::State::Idle);

        }
        else
        {
            qDebug() << "***** can't create property " << key;
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
    QVariantMap _res;
    QVariantMap m = getPropertiesDump().toVariantMap();
    foreach(const QString &keyprop, m.keys())
    {
        if (m[keyprop].toMap().contains("hasprofile"))
        {
            if (m[keyprop].toMap()["hasprofile"].toBool())
            {
                QVariantMap property;
                if (m[keyprop].toMap().contains("elements"))
                {
                    QVariantMap element, elements;
                    foreach(const QString &keyelt, m[keyprop].toMap()["elements"].toMap().keys())
                    {
                        if (m[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("value"))
                        {
                            element["value"] = m[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["value"];
                        }
                        if (m[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("min"))
                        {
                            element["min"] = m[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["min"];
                        }
                        if (m[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("max"))
                        {
                            element["max"] = m[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["max"];
                        }
                        if (m[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("step"))
                        {
                            element["step"] = m[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["step"];
                        }
                        if (m[keyprop].toMap()["elements"].toMap()[keyelt].toMap().contains("gridvalues"))
                        {
                            element["gridvalues"] = m[keyprop].toMap()["elements"].toMap()[keyelt].toMap()["gridvalues"];
                        }
                        elements[keyelt] = element;
                    }
                    property["elements"] = elements;
                }
                _res[keyprop] = property;
            }

        }
    }

    return _res;
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

QVariant Datastore::getOstElementLineValue(const QString &pPropertyName, const QString &pElementName,
        const double &pLine)
{
    //if (getOstElementGrid(pPropertyName, pElementName).isEmpty())
    //{
    //    sendWarning("Try to read an empty grid (" + pPropertyName + "/" + pElementName + ")");
    //    return QVariant();

    //}
    //else
    //{
    //    if (pLine >= getOstElementGrid(pPropertyName, pElementName).count())
    //    {
    //        sendWarning("Try to access inexistant line grid (" + pPropertyName + "/" + pElementName + ") size=" + getOstElementGrid(
    //                        pPropertyName, pElementName).count() + " vs requested=" + pLine);
    //        return QVariant();
    //    }
    //    else
    //    {
    //        return getOstElementGrid(pPropertyName, pElementName)[pLine];
    //    }
    //}
    return QVariant();
}
bool Datastore::setOstElementLineValue (const QString &pPropertyName, const QString &pElementName, const double &pLine,
                                        const QVariant &pElementValue)
{
    //QVariantList elementList  = getOstElementGrid(pPropertyName, pElementName);
    //if (pLine >= elementList.count())
    //{
    //    sendWarning("invalid line (" + pPropertyName + "/" + pElementName + ") size=" + QString::number(
    //                    elementList.count()) + " vs requested(-1)=" +
    //                QString::number(pLine));
    //    return false;
    //}
    //else
    //{
    //    elementList[pLine] = pElementValue;
    //    OST::ValueUpdate u;
    //    QVariantMap m;
    //    m["i"] = pLine;
    //    m["val"] = pElementValue;
    //    QString action = "updateline";
    //    getStore()[pPropertyName]->getValue(pElementName)->accept(&u, action, m);
    //    OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    //    return true;
    //}
    return true;

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
