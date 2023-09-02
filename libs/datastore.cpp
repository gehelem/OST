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

OST::ValueString* Datastore::getText(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning(" getText - property " + pProperty + " not found");
        return nullptr;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getText - property " + pProperty + " : element " + pElement + " not found");
        return nullptr;
    }
    return static_cast<OST::ValueString*>(p->getValue(pElement));
}
QString Datastore::getString(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getText - property " + pProperty + " not found");
        return "";
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getText - property " + pProperty + " : element " + pElement + " not found");
        return "";
    }
    QString s = static_cast<OST::ValueString*>(p->getValue(pElement))->value();
    return s;

}
QString Datastore::getString(QString pProperty, QString pElement, long line)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getText - property " + pProperty + " not found");
        return "";
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getText - property " + pProperty + " : element " + pElement + " not found");
        return "";
    }
    QString s = static_cast<OST::ValueString*>(p->getValue(pElement))->grid.getGrid()[line];
    return s;

}
long Datastore::getInt(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getInt - property " + pProperty + " not found");
        return 0;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getInt - property " + pProperty + " : element " + pElement + " not found");
        return 0;
    }
    long i = static_cast<OST::ValueInt*>(p->getValue(pElement))->value();
    return i;

}
long Datastore::getInt(QString pProperty, QString pElement, long line)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getInt - property " + pProperty + " not found");
        return 0;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getInt - property " + pProperty + " : element " + pElement + " not found");
        return 0;
    }
    long i = static_cast<OST::ValueInt*>(p->getValue(pElement))->grid.getGrid()[line];
    return i;

}
double Datastore::getFloat(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getFloat - property " + pProperty + " not found");
        return 0;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getFloat - property " + pProperty + " : element " + pElement + " not found");
        return 0;
    }
    double i = static_cast<OST::ValueFloat*>(p->getValue(pElement))->value();
    return i;

}
double Datastore::getFloat(QString pProperty, QString pElement, long line)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        sendWarning("getFloat - property " + pProperty + " not found");
        return 0;
    }
    if (!p->getValues().contains(pElement))
    {
        sendWarning("getFloat - property " + pProperty + " : element " + pElement + " not found");
        return 0;
    }
    double i = static_cast<OST::ValueFloat*>(p->getValue(pElement))->grid.getGrid()[line];
    return i;

}

bool Datastore::createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel,
                                  const int &pPropertyPermission,
                                  const  QString &pPropertyDevcat, const QString &pPropertyGroup)
{
    if (mProperties.contains(pPropertyName))
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
    emit OnModuleEvent("cp", QString(), pPropertyName, mProperties[pPropertyName].toMap());
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
/*QVariant Datastore::getOstPropertyValue(const QString &pPropertyName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("getPropertyValue - property " + pPropertyName + " not found.");
        return QVariant();
    }
    if (!mProperties[pPropertyName].toMap().contains("value"))
    {
        sendWarning("getPropertyValue - property " + pPropertyName + " contains no value.");
        return QVariant();
    }
    return mProperties[pPropertyName].toMap()["value"];
}*/

bool Datastore::createOstElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                 bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName) )
    {
        sendWarning(" createElement - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = mProperties[pPropertyName].toMap();
    QVariantMap _elements = _prop["elements"].toMap();
    if (_elements.contains(pElementName) )
    {
        sendWarning("createElement - property " + pPropertyName + " : element " + pElementName + " already exists.");
        return false ;
    }
    QVariantMap _element;
    _element["label"] = pElementLabel;
    _elements[pElementName] = _element;
    _prop["elements"] = _elements;
    mProperties[pPropertyName] = _prop;

    if (mEmitEvent) OnModuleEvent("ce", QString(), pPropertyName, mProperties[pPropertyName].toMap());

    return true;
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
    OST::PropertyJsonDumper d;
    QVariantMap m;
    m["value"] = pElementValue;
    pb->getValue(pElementName)->accept(&vu, m);
    pb->accept(&d);


    //if (mEmitEvent) OnModuleEvent("se", QString(), pPropertyName, d.getResult().toVariantMap());
    return true;
}
bool Datastore::setOstElementGrid(const QString &pPropertyName, const QString &pElementName,
                                  const QVariantList &pElementGrid,
                                  bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName) )
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = mProperties[pPropertyName].toMap();
    if (!_prop.contains("elements"))
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " has no elemnts");
        return false;
    }
    QVariantMap _elements = _prop["elements"].toMap();
    if (!_elements.contains(pElementName))
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " : element " + pElementName + " not found.");
        return false;
    }

    QVariantMap element = _elements[pElementName].toMap();
    if (!_prop.contains("grid"))
    {
        sendWarning("setOstElementGrid - property " + pPropertyName + " element " + pElementName + " has no grid.");
        return false;
    }
    element["gridvalues"] = pElementGrid;
    _elements[pElementName] = element;
    _prop["elements"] = _elements;
    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("se", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;
}
QVariantList Datastore::getOstElementGrid(const QString &pPropertyName, const QString &pElementName)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " not found.");
        return QVariantList();
    }
    if (!mProperties[pPropertyName].toMap().contains("elements"))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " contains no elements.");
        return QVariantList();
    }
    if (!mProperties[pPropertyName].toMap().contains("grid"))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " element " + pElementName + " has no grid.");
        return QVariantList();
    }

    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
    if (!elts.contains(pElementName))
    {
        sendWarning("getOstElementGrid - property " + pPropertyName + " has no " + pElementName + " element.");
        return QVariantList();
    }
    if (!mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap().contains("gridvalues"))
    {
        return QVariantList();
    }
    return mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["gridvalues"].toList();
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
        mProperties[key] = tt;
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
void Datastore::setOstPropertyAttribute   (const QString &pPropertyName, const QString &pAttributeName,
        const QVariant &AttributeValue,
        bool mEmitEvent)
{
    //sendMessage("setOstPropertyAttribute  - " + pPropertyName);
    QVariantMap _prop = mProperties[pPropertyName].toMap();
    _prop[pAttributeName] = AttributeValue;
    mProperties[pPropertyName] = _prop;
    if (mEmitEvent) OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

}
bool Datastore::setOstElementAttribute(const QString &pPropertyName, const QString &pElementName,
                                       const  QString &pAttributeName,
                                       const QVariant &AttributeValue,
                                       bool mEmitEvent)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("setOstElementAttribute - property " + pPropertyName + " not found.");
        return false;
    }
    QVariantMap prop = mProperties[pPropertyName].toMap();
    if (!prop.contains("elements"))
    {
        sendWarning("setOstElementAttribute - property " + pPropertyName + " contains no elements.");
        return false;
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
    if (!elts.contains(pElementName))
    {
        sendWarning("setOstElementAttribute - property " + pPropertyName + " has no " + pElementName + " element.");
        return false;
    }

    QVariantMap elt = elts[pElementName].toMap();
    elts[pAttributeName] = AttributeValue;
    elts[pElementName] = elt;
    prop["elements"] = elts;
    mProperties[pPropertyName] = prop;
    if (mEmitEvent) OnModuleEvent("ae", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;


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
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("Can't delete inexistant property " + pPropertyName);
        return;
    }
    mProperties.remove(pPropertyName);
    emit OnModuleEvent("delprop", QString(), pPropertyName, QVariantMap());

}

QVariant Datastore::getOstElementLineValue(const QString &pPropertyName, const QString &pElementName,
        const double &pLine)
{
    if (getOstElementGrid(pPropertyName, pElementName).isEmpty())
    {
        sendWarning("Try to read an empty grid (" + pPropertyName + "/" + pElementName + ")");
        return QVariant();

    }
    else
    {
        if (pLine >= getOstElementGrid(pPropertyName, pElementName).count())
        {
            sendWarning("Try to access inexistant line grid (" + pPropertyName + "/" + pElementName + ") size=" + getOstElementGrid(
                            pPropertyName, pElementName).count() + " vs requested=" + pLine);
            return QVariant();
        }
        else
        {
            return getOstElementGrid(pPropertyName, pElementName)[pLine];
        }
    }
}
bool Datastore::setOstElementLineValue (const QString &pPropertyName, const QString &pElementName, const double &pLine,
                                        const QVariant &pElementValue)
{
    QVariantList elementList  = getOstElementGrid(pPropertyName, pElementName);
    if (pLine >= elementList.count())
    {
        sendWarning("invalid line (" + pPropertyName + "/" + pElementName + ") size=" + QString::number(
                        elementList.count()) + " vs requested(-1)=" +
                    QString::number(pLine));
        return false;
    }
    else
    {
        elementList[pLine] = pElementValue;
        setOstElementGrid(pPropertyName, pElementName, elementList, true);
        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        return true;
    }


}
void Datastore::clearOstElementLov(const QString &pPropertyName, const QString &pElementName)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProp["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mElts[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    mElt["listOfValues"] = QVariantMap();
                    mElts[pElementName] = mElt;
                    mProp["elements"] = mElts;
                    mProperties[pPropertyName] = mProp;
                    OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
                }
                else
                {
                    sendWarning("clearOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("clearOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
                return;
            }
        }
        else
        {
            sendWarning("clearOstElementLov - property " + pPropertyName + " contains no elements.");
            return;
        }
    }
    else
    {
        sendWarning("clearOstElementLov - property " + pPropertyName + " not found.");
        return;
    }

}
void Datastore::addOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode,
                                 const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    QVariantMap mLov = mElt["listOfValues"].toMap();
                    if (mLov.contains(pLovCode))
                    {
                        sendWarning("addOstElementLov - property " + pPropertyName + " element " + pElementName + " list of values member " +
                                    pLovCode +
                                    " already exists");
                    }
                    else
                    {
                        mLov[pLovCode] = pLovLabel;
                        mElt["listOfValues"] = mLov;
                        mElts[pElementName] = mElt;
                        mProp["elements"] = mElts;
                        mProperties[pPropertyName] = mProp;
                        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

                    }
                }
                else
                {
                    sendWarning("addOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("addOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
            }
        }
        else
        {
            sendWarning("addOstElementLov - property " + pPropertyName + " contains no elements.");
        }
    }
    else
    {
        sendWarning("addOstElementLov - property " + pPropertyName + " not found.");
    }

}
QVariant Datastore::getOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode)
{
    if (!mProperties.contains(pPropertyName))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " not found.");
        return QVariant();
    }
    if (!mProperties[pPropertyName].toMap().contains("elements"))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " contains no elements.");
        return QVariant();
    }
    QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
    if (!elts.contains(pElementName))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
        return QVariant();
    }
    QVariantMap elt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
    if (!elt.contains("listOfValues"))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
        return QVariant();
    }
    if (!elt["listOfValues"].toMap().contains(pLovCode))
    {
        sendWarning("getOstElementLov - property " + pPropertyName + " element " + pElementName + " has no " + pLovCode +
                    " member in list of values.");
        return QVariant();
    }

    return elt["listOfValues"].toMap()[pLovCode];

}
void Datastore::setOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode,
                                 const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    QVariantMap mLov = mElt["listOfValues"].toMap();
                    if (mLov.contains(pLovCode))
                    {
                        mLov[pLovCode] = pLovLabel;
                        mElt["listOfValues"] = mLov;
                        mElts[pElementName] = mElt;
                        mProp["elements"] = mElts;
                        mProperties[pPropertyName] = mProp;
                        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
                    }
                    else
                    {
                        sendWarning("setOstElementLov - property " + pPropertyName + " element " + pElementName + " has no " + pLovCode +
                                    " member in list of values.");
                    }
                }
                else
                {
                    sendWarning("setOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("setOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
            }
        }
        else
        {
            sendWarning("setOstElementLov - property " + pPropertyName + " contains no elements.");
        }
    }
    else
    {
        sendWarning("setOstElementLov - property " + pPropertyName + " not found.");
    }
}
void Datastore::deleteOstElementLov(const QString &pPropertyName, const QString &pElementName, const QString &pLovCode)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("elements"))
        {
            QVariantMap mElts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (mElts.contains(pElementName))
            {
                QVariantMap mElt =  mProperties[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap();
                if (mElt.contains("listOfValues"))
                {
                    QVariantMap mLov = mElt["listOfValues"].toMap();
                    if (mLov.contains(pLovCode))
                    {
                        mLov[pLovCode].clear();
                        mElt["listOfValues"] = mLov;
                        mElts[pElementName] = mElt;
                        mProp["elements"] = mElts;
                        mProperties[pPropertyName] = mProp;
                        OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

                    }
                    else
                    {
                        sendWarning("deleteOstElementLov - property " + pPropertyName + " element " + pElementName + " has no " + pLovCode +
                                    " member in list of values.");
                    }
                }
                else
                {
                    sendWarning("deleteOstElementLov - property " + pPropertyName + " element " + pElementName + " has no list of values.");
                }
            }
            else
            {
                sendWarning("deleteOstElementLov - property " + pPropertyName + " has no " + pElementName + " element.");
            }
        }
        else
        {
            sendWarning("deleteOstElementLov - property " + pPropertyName + " contains no elements.");
        }
    }
    else
    {
        sendWarning("deleteOstElementLov - property " + pPropertyName + " not found.");
    }

}
void Datastore::clearOstLov(const QString &pPropertyName)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            mProp["listOfValues"] = QVariantMap();
            mProperties[pPropertyName] = mProp;
            OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        }
        else
        {
            sendWarning("clearOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("clearOstLov - property " + pPropertyName + " not found.");
        return;
    }
}
void Datastore::addOstLov(const QString &pPropertyName, const QString &pLovCode,
                          const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (mLov.contains(pLovCode))
            {
                sendWarning("addOstLov - property " + pPropertyName + " list of values member " + pLovCode +
                            " already exists");
                return;

            }
            else
            {
                mLov[pLovCode] = pLovLabel;
                mProp["listOfValues"] = mLov;
                mProperties[pPropertyName] = mProp;
                OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

            }
        }
        else
        {
            sendWarning("addOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("addOstLov - property " + pPropertyName + " not found.");
        return;
    }
    return;
}
QVariant Datastore::getOstLov(const QString &pPropertyName, const QString &pLovCode)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (!mLov.contains(pLovCode))
            {
                sendWarning("getOstLov - property " + pPropertyName + " has no " + pLovCode +
                            " member in list of values.");

                return QVariant();
            }
            else
            {
                return mLov[pLovCode];
            }
        }
        else
        {
            sendWarning("getOstLov - property " + pPropertyName + " has no list of values.");
            return QVariant();
        }
    }
    else
    {
        sendWarning("getOstLov - property " + pPropertyName + " not found.");
        return QVariant();
    }
    return QVariant();
}
void Datastore::setOstLov(const QString &pPropertyName, const QString &pLovCode,
                          const QString &pLovLabel)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (!mLov.contains(pLovCode))
            {
                sendWarning("setOstLov - property " + pPropertyName + " list of values member " + pLovCode +
                            " does not exists");
                return;

            }
            else
            {
                mLov[pLovCode] = pLovLabel;
                mProp["listOfValues"] = mLov;
                mProperties[pPropertyName] = mProp;
                OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

            }
        }
        else
        {
            sendWarning("setOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("setOstLov - property " + pPropertyName + " not found.");
        return;
    }
    return;

}
void Datastore::deleteOstLov(const QString &pPropertyName, const QString &pLovCode)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap mProp = mProperties[pPropertyName].toMap();
        if (mProp.contains("listOfValues"))
        {
            QVariantMap mLov = mProp["listOfValues"].toMap();
            if (!mLov.contains(pLovCode))
            {
                sendWarning("deleteOstLov - property " + pPropertyName + " list of values member " + pLovCode +
                            " does not exists");
                return;

            }
            else
            {
                mLov[pLovCode].clear();
                mProp["listOfValues"] = mLov;
                mProperties[pPropertyName] = mProp;
                OnModuleEvent("ap", QString(), pPropertyName, mProperties[pPropertyName].toMap());

            }
        }
        else
        {
            sendWarning("deleteOstLov - property " + pPropertyName + " has no list of values.");
            return;
        }
    }
    else
    {
        sendWarning("deleteOstLov - property " + pPropertyName + " not found.");
        return;
    }
    return;

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
