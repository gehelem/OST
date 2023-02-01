#include <QtCore>
#include "datastore.h"

Datastore::Datastore()
{
    /*mParams["name"] = params["name"];
    mParams["label"] = params["label"];
    mParams["type"] = this->metaObject()->className();
    mParams["description"] = params["description"];*/
}
Datastore::~Datastore()
{
}
bool Datastore::createOstProperty(const QString &pPropertyName, const QString &pPropertyLabel,
                                  const int &pPropertyPermission,
                                  const  QString &pPropertyDevcat, const QString &pPropertyGroup, QString &err)
{
    if (mProperties.contains(pPropertyName))
    {
        sendMessage("createOstProperty - property " + pPropertyName + " already exists.");
        return false;
    }
    else
    {
        QVariantMap prop;
        prop["propertyLabel"] = pPropertyLabel;
        prop["permission"] = pPropertyPermission;
        prop["devcat"] = pPropertyDevcat;
        prop["group"] = pPropertyGroup;
        prop["name"] = pPropertyName;
        mProperties[pPropertyName] = prop;
        qDebug() << "Datastore createProperty";
        OnModuleEvent("createProperty", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        return true;
    }
}

bool Datastore::setOstPropertyValue(const QString &pPropertyName, const QVariant &pPropertyValue, bool mEmitEvent)
{
    if (mProperties.contains(pPropertyName))
    {
        QVariantMap prop = mProperties[pPropertyName].toMap();
        prop["value"] = pPropertyValue;
        mProperties[pPropertyName] = prop;
        qDebug() << "Datastore setPropertyValue";
        if (mEmitEvent) OnModuleEvent("setPropertyValue", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        return true;
    }
    else
    {
        return false;
    }
}
QVariant Datastore::getOstPropertyValue(QString &pPropertyName)
{
    if (mProperties.contains(pPropertyName))
    {
        if (mProperties[pPropertyName].toMap().contains("value"))
        {
            return mProperties[pPropertyName].toMap()["value"];
        }
        else
        {
            sendMessage("getPropertyValue - property " + pPropertyName + " contains no value.");
            return QVariant();
        }
    }
    else
    {
        sendMessage("getPropertyValue - property " + pPropertyName + " not found.");
        return QVariant();
    }
}

bool Datastore::createOstElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
                                 bool mEmitEvent)
{
    QVariantMap _props = mProperties["properties"].toMap();
    if (!_props.contains(pPropertyName) )
    {
        sendMessage(" createElement - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = _props[pPropertyName].toMap();
    QVariantMap _elements = _prop["elements"].toMap();
    if (_elements.contains(pPropertyName) )
    {
        sendMessage(" createElement - property " + pPropertyName + " : element " + pElementName + "already exists.");
        return false ;
    }
    QVariantMap _element = _elements[pElementName].toMap();
    _element["elementLabel"] = pElementLabel;
    _elements[pElementName] = _element;
    _prop["elements"] = _elements;
    _props[pPropertyName] = _prop;
    mProperties["properties"] = _props;
    if (mEmitEvent) OnModuleEvent("createElement", QString(), pPropertyName, mProperties[pPropertyName].toMap());
    return true;
}
bool Datastore::setOstElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
                                   bool mEmitEvent)
{
    QVariantMap _props = mProperties["properties"].toMap();
    if (!_props.contains(pPropertyName) )
    {
        sendMessage("setElementValue - property " + pPropertyName + " not found");
        return false;
    }

    QVariantMap _prop = _props[pPropertyName].toMap();

    if (_prop.contains("elements"))
    {
        if (_prop["elements"].toMap().contains(pElementName))
        {
            QVariantMap _elements = _prop["elements"].toMap();
            if (!_elements.contains(pElementName) )
            {
                sendMessage("setElementValue - property " + pPropertyName + " : element " + pElementName + " not found.");
                return false;
            }

            QVariantMap element = _elements[pElementName].toMap();
            if (element.contains("value"))
            {
                if (strcmp(element["value"].typeName(), "double") == 0)
                {
                    //element["value"]=pElementValue.toDouble();
                    element["value"].setValue(pElementValue.toDouble());
                }
                if (strcmp(element["value"].typeName(), "float") == 0)
                {
                    //element["value"]=pElementValue.toFloat();
                    element["value"].setValue(pElementValue.toFloat());
                }
                if (strcmp(element["value"].typeName(), "qlonglong") == 0)
                {
                    //element["value"]=pElementValue.toFloat();
                    element["value"].setValue(pElementValue.toDouble());
                }
                if (strcmp(element["value"].typeName(), "int") == 0)
                {
                    //element["value"]=pElementValue.toInt();
                    element["value"].setValue(pElementValue.toInt());
                }
                if (strcmp(element["value"].typeName(), "QString") == 0)
                {
                    element["value"] = pElementValue.toString();
                }
                if (strcmp(element["value"].typeName(), "bool") == 0)
                {
                    element["value"] = pElementValue.toBool();
                }
            }
            else
            {
                element["value"] = pElementValue;
            }
            _elements[pElementName] = element;
            _prop["elements"] = _elements;
        }
    }
    _props[pPropertyName] = _prop;
    mProperties["properties"] = _props;
    if (mEmitEvent) OnModuleEvent("setElementValue", QString(), pPropertyName, mProperties[pPropertyName].toMap());


    return true; // should return false when request is invalid, we'll see that later
}
QVariant Datastore::getOstElementValue(const QString &pPropertyName, const QString &pElementName)
{
    return mProperties["properties"].toMap()[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["value"];
    if (mProperties.contains(pPropertyName))
    {
        return false;
        if (mProperties[pPropertyName].toMap().contains("elements"))
        {
            QVariantMap elts = mProperties[pPropertyName].toMap()["elements"].toMap();
            if (elts.contains(pElementName))
            {
                return mProperties["properties"].toMap()[pPropertyName].toMap()["elements"].toMap()[pElementName].toMap()["value"];
            }
            else
            {
                sendMessage("getElementValue - property " + pPropertyName + " has no " + pElementName + " element.");
                return QVariant();
            }
        }
        else
        {
            sendMessage("getElementValue - property " + pPropertyName + " contains no elements.");
            return QVariant();
        }
    }
    else
    {
        sendMessage("getElementValue - property " + pPropertyName + " not found.");
        return QVariant();
    }


}
void Datastore::loadOstPropertiesFromFile(const QString &pFileName)
{
    QVariantMap _props = mProperties["properties"].toMap();

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
        _props[key] = props[key].toVariant();
    }
    mProperties["properties"] = _props;

    //QByteArray docByteArray = d.toJson(QJsonDocument::Compact);
    //QString strJson = QLatin1String(docByteArray);
    //BOOST_LOG_TRIVIAL(debug) << "loadPropertiesFromFile  - " << mModulename.toStdString() << " - filename=" << fileName.toStdString() << " - " << strJson.toStdString();

}

void Datastore::saveOstPropertiesToFile(const QString &pFileName)
{
    QJsonObject obj = QJsonObject::fromVariantMap(mProperties);
    QJsonDocument doc(obj);

    QFile jsonFile(pFileName);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(doc.toJson());
    jsonFile.close();
}
