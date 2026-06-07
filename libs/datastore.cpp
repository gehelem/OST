#include <QtCore>
#include "datastore.h"

Datastore::Datastore()
{
    //sendMessage("DataStore init");
    qRegisterMetaType<OST::EvType>("OST::EvType");
    qRegisterMetaType<OST::LogLevel>("OST::LogLevel");
    qRegisterMetaType<OST::ExtEvent>("OST::ExtEvent");

}
Datastore::~Datastore()
{
}
void Datastore::setMetadata(const QString key, const QVariant value)
{
    if (mMetadata.contains(key))
    {
        logWarning("module %1 - setMetadata - key %2 already exists (%3)", {this->getModuleName(), key, mMetadata[key]});
        return;
    }
    mMetadata[key] = value;
}

OST::PropertyMulti* Datastore::getProperty(QString pProperty)
{
    if (!mStore.contains(pProperty))
    {
        logWarning("getProperty - property %1 not found", {pProperty});
        //return nullptr;
    }

    return mStore[pProperty];
}
OST::ElementBase* Datastore::getEltBase(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltString - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltString - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    return p->getElt(pElement);
}

OST::ElementString* Datastore::getEltString(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltString - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltString - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "string")
    {
        logWarning("getEltString - property %1 : element %2 is not string", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementString*>(p->getElt(pElement));
}
OST::ElementPrg* Datastore::getEltPrg(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltPrg - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltPrg - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "prg")
    {
        logWarning("getEltPrg - property %1 : element %2 is not progress", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementPrg*>(p->getElt(pElement));
}
OST::ElementDate* Datastore::getEltDate(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltDate - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltDate - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "date")
    {
        logWarning("getEltDate - property %1 : element %2 is not date", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementDate*>(p->getElt(pElement));
}
OST::ElementTime* Datastore::getEltTime(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltTime - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltTime - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "time")
    {
        logWarning("getEltTime - property %1 : element %2 is not time", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementTime*>(p->getElt(pElement));
}
OST::ElementDateTime* Datastore::getEltDateTime(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltDateTime - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltDateTime - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "datetime")
    {
        logWarning("getEltDateTime - property %1 : element %2 is not datetime", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementDateTime*>(p->getElt(pElement));
}

QString Datastore::getString(QString pProperty, QString pElement)
{
    return getEltString(pProperty, pElement)->value();
}
QString Datastore::getString(QString pProperty, QString pElement, long line)
{
    if (getProperty(pProperty)->getGrid().size() < line + 1)
    {
        logWarning("getString - property %1 line : %2 does not exist", {pProperty, QString::number(line)});
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
        logWarning("getEltInt - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltInt - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "int")
    {
        logWarning("getEltInt - property %1 : element %2 is not int", {pProperty, pElement});
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
        logWarning("getInt- property %1 line : %2 does not exist", {pProperty, QString::number(line)});
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
        logWarning("getEltFloat - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltFloat - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "float")
    {
        logWarning("getEltFloat - property %1 : element %2 is not float", {pProperty, pElement});
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
        logWarning("getFloat - property %1 line : %2 does not exist", {pProperty, QString::number(line)});
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
        logWarning("getEltLight - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltLight - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "light")
    {
        logWarning("getEltLight - property %1 : element %2 is not light", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementLight*>(p->getElt(pElement));
}

OST::ElementImg* Datastore::getEltImg(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltImg - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltImg - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "img")
    {
        logWarning("getEltImg - property %1 : element %2 is not img", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementImg*>(p->getElt(pElement));
}
OST::ElementVideo* Datastore::getEltVideo(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("getEltVideo - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("getEltVideo - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "video")
    {
        logWarning("getEltVideo - property %1 : element %2 is not video", {pProperty, pElement});
        return nullptr;
    }
    return static_cast<OST::ElementVideo*>(p->getElt(pElement));
}
OST::ElementBool* Datastore::getEltBool(QString pProperty, QString pElement)
{
    OST::PropertyMulti* p = getProperty(pProperty);
    if (p == nullptr)
    {
        logWarning("ValueBool - property %1 not found", {pProperty});
        return nullptr;
    }
    if (!p->getElts()->contains(pElement))
    {
        logWarning("ValueBool - property %1 : element %2 not found", {pProperty, pElement});
        return nullptr;
    }
    if (p->getElt(pElement)->getType() != "bool")
    {
        logWarning("ValueBool - property %1 : element %2 is not bool", {pProperty, pElement});
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
        logWarning("getBool - property %1 line : %2 does not exist", {pProperty, QString::number(line)});
        return 0;
    }
    OST::ValueBool* v = static_cast<OST::ValueBool*>(getProperty(pProperty)->getGrid()[line][pElement]);
    return v->value;
}
QDate Datastore::getDate(QString pProperty, QString pElement)
{
    return getEltDate(pProperty, pElement)->value();
}
QDate Datastore::getDate(QString pProperty, QString pElement, long line)
{
    if (getProperty(pProperty)->getGrid().size() < line + 1)
    {
        logWarning("getDate - property %1 line : %2 does not exist", {pProperty, QString::number(line)});
        return QDate();
    }
    OST::ValueDate* v = static_cast<OST::ValueDate*>(getProperty(pProperty)->getGrid()[line][pElement]);
    return v->value;
}
QTime Datastore::getTime(QString pProperty, QString pElement)
{
    return getEltTime(pProperty, pElement)->value();
}
QTime Datastore::getTime(QString pProperty, QString pElement, long line)
{
    if (getProperty(pProperty)->getGrid().size() < line + 1)
    {
        logWarning("getTime - property %1 line : %2 does not exist", {pProperty, QString::number(line)});
        return QTime();
    }
    OST::ValueTime* v = static_cast<OST::ValueTime*>(getProperty(pProperty)->getGrid()[line][pElement]);
    return v->value;
}

void Datastore::onPropertyEvent(OST::EvType event, QVariant data, OST::ElementBase* e, OST::PropertyBase* p)
{
    emit datastoreEvent(event, data, e, p, nullptr, this);
}
void Datastore::onLovChanged()
{
    OST::LovBase* lov = qobject_cast<OST::LovBase*>(sender());
    emit datastoreEvent(OST::EvType::lc, lov->getKey(), nullptr, nullptr, lov, this);
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
    if (data.contains("p"))
    {
        QJsonObject props = data["p"].toObject();
        for(const QString &key : props.keys())        {
            //qDebug() << "***** create property from file " << key;
            QVariantMap tt = props[key].toVariant().toMap();
            OST::PropertyMulti *rp = OST::PropertyFactory::createProperty(key, tt);
            if (rp != nullptr)
            {
                mStore[key] = rp;
                connect(rp, &OST::PropertyMulti::prpEvent, this, &Datastore::onPropertyEvent);
                connect(rp, &OST::PropertyMulti::logMessage, this, &Datastore::onPropertyLog);
                mStore[key]->setState(OST::State::Idle, true);

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
        for(const QString &key : lovs.keys())        {
            QVariantMap tt = lovs[key].toVariant().toMap();
            OST::LovBase *lb = OST::LovFactory::createLov(tt);

            if (lb != nullptr)
            {
                mGlobLov[key] = lb;
                OST::LovJsonDumper d;
                mGlobLov[key]->accept(&d);
                connect(mGlobLov[key], &OST::LovBase::lovChanged, this, &Datastore::onLovChanged);
                connect(mGlobLov[key], &OST::LovBase::sendMessage, this, &Datastore::onPropertyLog);

            }
            else
            {
                qDebug() << "***** can't create globlov " << key;
            }

        }


    }


}

QVariantMap Datastore::getProfile(void)
{
    QVariantMap result;
    QVariantMap propertiesResult;
    //    OST::Event e;
    //    e.type = "moduledump";
    //    QVariantMap props = getPropertiesDump(e).toVariantMap();
    //    result["globallovs"] = getGlobalLovsDump().toVariantHash();
    //
    //
    //    for(const QString &keyprop : props.keys())    //    {
    //        if (props[keyprop].toMap().contains("hasprofile"))
    //        {
    //            if (props[keyprop].toMap()["hasprofile"].toBool())
    //            {
    //                QVariantMap property;
    //                if (props[keyprop].toMap().contains("e"))
    //                {
    //                    QVariantMap elements;
    //                    for(const QString &keyelt : props[keyprop].toMap()["e"].toMap().keys())    //                    {
    //                        QVariantMap element;
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("value"))
    //                        {
    //                            element["value"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["value"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("min"))
    //                        {
    //                            element["min"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["min"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("max"))
    //                        {
    //                            element["max"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["max"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("step"))
    //                        {
    //                            element["step"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["step"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("year"))
    //                        {
    //                            element["year"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["year"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("month"))
    //                        {
    //                            element["month"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["month"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("day"))
    //                        {
    //                            element["day"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["day"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("hh"))
    //                        {
    //                            element["hh"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["hh"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("mm"))
    //                        {
    //                            element["mm"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["mm"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("ss"))
    //                        {
    //                            element["ss"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["ss"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("ms"))
    //                        {
    //                            element["ms"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["ms"];
    //                        }
    //                        if (props[keyprop].toMap()["e"].toMap()[keyelt].toMap().contains("gridvalues"))
    //                        {
    //                            element["gridvalues"] = props[keyprop].toMap()["e"].toMap()[keyelt].toMap()["gridvalues"];
    //                        }
    //                        elements[keyelt] = element;
    //                    }
    //                    property["e"] = elements;
    //                }
    //                if (props[keyprop].toMap().contains("gridheaders"))
    //                {
    //                    property["gridheaders"] = props[keyprop].toMap()["gridheaders"].toList();
    //                }
    //                if (props[keyprop].toMap().contains("grid"))
    //                {
    //                    property["grid"] = props[keyprop].toMap()["grid"].toList();
    //                }
    //
    //                propertiesResult[keyprop] = property;
    //            }
    //
    //        }
    //    }
    //
    //    result["p"] = propertiesResult;
    return result;
}

void Datastore::deleteOstProperty(const QString &pPropertyName)
{
    if (!mStore.contains(pPropertyName))
    {
        logWarning("Can't delete inexistant property %1", {pPropertyName});
        return;
    }
    mStore.remove(pPropertyName);
    emit datastoreEvent(OST::EvType::dp, pPropertyName, nullptr,  nullptr, nullptr, this);
}


OST::LovString* Datastore::getGlovString(QString pLov)
{
    if (!getGlobLovs().contains(pLov))
    {
        logWarning("getGlovString - Globallov %1 not found", {pLov});
        return nullptr;
    }
    if (getGlobLovs()[pLov]->getType() != "string")
    {
        logWarning("getGlovString - Globallov %1 wrong type %2", {pLov, getGlobLovs()[pLov]->getType()});
        return nullptr;
    }

    return static_cast<OST::LovString*>(getGlobLovs()[pLov]);
}
void Datastore::onPropertyLog(OST::LogLevel l, QString m, QVariantList args)
{
    switch (l)
    {
        case OST::LogLevel::Debug:
            logDebug(m, args);
            break;
        case OST::LogLevel::Info:
            logInfo(m, args);
            break;
        case OST::LogLevel::Warning:
            logWarning(m, args);
            break;
        case OST::LogLevel::Error:
            logError(m, args);
            break;
        case OST::LogLevel::Critical:
            logCritical(m, args);
            break;
        default:
            logError(m, args);
            break;
    }

}
// ============================================================================
// NEW SYSTEM - Helper methods that emit the signal
// ============================================================================

void Datastore::logDebug(const QString &message, const QVariantList &args)
{
    emit logSignal(OST::LogLevel::Debug, message, args, getModuleName());
}

void Datastore::logInfo(const QString &message, const QVariantList &args)
{
    emit logSignal(OST::LogLevel::Info, message, args, getModuleName());
}

void Datastore::logWarning(const QString &message, const QVariantList &args)
{
    emit logSignal(OST::LogLevel::Warning, message, args, getModuleName());
}

void Datastore::logError(const QString &message, const QVariantList &args)
{
    emit logSignal(OST::LogLevel::Error, message, args, getModuleName());
}

void Datastore::logCritical(const QString &message, const QVariantList &args)
{
    emit logSignal(OST::LogLevel::Critical, message, args, getModuleName());
}
