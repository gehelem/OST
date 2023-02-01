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
bool Datastore::createProperty(const QString &pPropertyName, const QString &pPropertyLabel, bool mEmitEvent)
{
    if (mProperties.contains(pPropertyName))
    {
        return false;
    }
    else
    {
        QVariantMap prop;
        prop["label"] = pPropertyLabel;
        //prop["value"] = val;
        mProperties[pPropertyName] = prop;
        qDebug() << "Datastore createProperty";
        if (mEmitEvent) OnModuleEvent("createProperty", QString(), pPropertyName, mProperties[pPropertyName].toMap());
        return true;
    }
}

bool Datastore::setPropertyValue(const QString &pPropertyName, const QVariant &pPropertyValue, bool mEmitEvent)
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
QVariant Datastore::getPropertyValue(QString &pPropertyName)
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

bool Datastore::createElement(const QString &pPropertyName, const QString &pElementName, const QString &pElementLabel,
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
bool Datastore::setElementValue(const QString &pPropertyName, const QString &pElementName, const QVariant &pElementValue,
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
QVariant Datastore::getElementValue(const QString &pPropertyName, const QString &pElementName)
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
