#include "maincontrol.h"

Maincontrol *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Maincontrol *basemodule = new Maincontrol(name, label, profile, availableModuleLibs);
    return basemodule;
}

Maincontrol::Maincontrol(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : Basemodule(name, label, profile, availableModuleLibs)
{

    Q_INIT_RESOURCE(maincontrol);
    setClassName(QString(metaObject()->className()).toLower());

    loadOstPropertiesFromFile(":maincontrol.json");
    getProperty("moduleInfo")->setElt("moduleLabel", "Main control");
    getEltString("moduleInfo", "moduleLabel")->setValue("Main control");
    getEltString("moduleInfo", "moduleDescription")->setValue("Maincontrol module - this one should always be there");
    getEltString("moduleInfo", "moduleVersion")->setValue("0.1");
    deleteOstProperty("saveprofile");
    deleteOstProperty("loadprofile");
    deleteOstProperty("moduleactions");
}

Maincontrol::~Maincontrol()
{
    Q_CLEANUP_RESOURCE(maincontrol);
}

void Maincontrol::OnMyExternalEvent(const QString &pEventType, const QString  &pEventModule, const QString  &pEventKey,
                                    const QVariantMap &pEventData)
{
    Q_UNUSED(pEventType);
    Q_UNUSED(pEventKey);

    if (getModuleName() == pEventModule)
    {
        if (pEventType == "refreshConfigurations")
        {
            setConfigurations();
        }
        if ((pEventType == "Fposticon") && (pEventData.contains("load")))
        {
            QVariantMap m = pEventData["load"].toMap()["elements"].toMap();
            QString pp = m.firstKey();
            pp.replace("libost", "");
            QString elt = getString("load", m.firstKey());
            QString eltwithoutblanks = getString("load", m.firstKey());
            eltwithoutblanks.replace(" ", "");
            QString prof = "default";


            emit loadOtherModule(pp,
                                 eltwithoutblanks,
                                 elt,
                                 prof);
            getProperty("load")->setState(OST::Ok);
        }
        if ((pEventType == "Fposticon") && (pEventData.contains("saveconf")))
        {
            emit mainCtlEvent("saveconf", QString(), getString("saveconf", "name"), QVariantMap());
            setConfigurations();
        }
        if ((pEventType == "Fposticon") && (pEventData.contains("loadconf")))
        {
            emit mainCtlEvent("loadconf", QString(), getString("loadconf", "name"), QVariantMap());
        }
        if ((pEventType == "Fpreicon") && (pEventData.contains("loadconf")))
        {
            setConfigurations();
        }
        foreach(const QString &keyprop, pEventData.keys())
        {
            foreach(const QString &keyelt, pEventData[keyprop].toMap()["elements"].toMap().keys())
            {
                if (keyelt == "name" && keyprop == "loadconf" && pEventType == "Fsetproperty")
                {

                    QString val = pEventData[keyprop].toMap()["elements"].toMap()["name"].toString();
                    getEltString("loadconf", "name")->setValue(val, true);
                    getEltString("saveconf", "name")->setValue(val, true);
                }
                if (keyelt == "kill" && keyprop == "killall")
                {
                    emit mainCtlEvent("killall", QString(), QString(), QVariantMap());
                }
                if (keyprop == "indidrivers" && keyelt == "search" && pEventType == "Fposticon")
                {
                    this->searchDriver();
                }
                if (keyprop == "indidrivers" && keyelt == "driver" && pEventType == "Fposticon")
                {
                    emit mainCtlEvent("startindidriver", QString(), getString("indidrivers", "driver"), QVariantMap());
                }
                if (keyprop == "indidrivers" && keyelt == "driver" && pEventType == "Fpreicon")
                {
                    emit mainCtlEvent("stopindidriver", QString(), keyelt, QVariantMap());
                }
                if (keyprop == "indiserver" && pEventType == "Fposticon")
                {
                    emit mainCtlEvent("indiserver", QString(), "start", QVariantMap());
                }
                if (keyprop == "indiserver" && pEventType == "Fpreicon")
                {
                    emit mainCtlEvent("indiserver", QString(), "stop", QVariantMap());
                }
            }
        }
    }
}
void Maincontrol::setConfigurations(void)
{
    QVariantMap confs;
    if (!getDbConfigurations( confs))
    {
        sendError("Can't refresh available configurations");
        return;
    }

    getEltString("loadconf", "name")->lovClear();
    for(QVariantMap::const_iterator iter = confs.begin(); iter != confs.end(); ++iter)
    {
        getEltString("loadconf", "name")->lovAdd(iter.key(), iter.key());
    }
    sendMessage("Available configurations refreshed");
}

void Maincontrol::sendMainMessage(const QString &pMessage)
{
    sendMessage(pMessage);
}
void Maincontrol::sendMainError(const QString &pMessage)
{
    sendError(pMessage);
}
void Maincontrol::sendMainWarning(const QString &pMessage)
{
    sendWarning(pMessage);
}
void Maincontrol::sendMainConsole(const QString &pMessage)
{
    sendConsole(pMessage);
}
void Maincontrol::setAvailableModuleLibs(const QVariantMap libs)
{

    OST::PropertyMulti *dynprop = new OST::PropertyMulti("load", "Available modules", OST::Permission::ReadWrite,
            "Modules",
            "", "00", false, false);
    foreach(QString key, libs.keys())
    {
        QVariantMap info = libs[key].toMap()["elements"].toMap();
        QString lab = info["moduleDescription"].toMap()["value"].toString();
        OST::ElementString* dyntext = new OST::ElementString(lab, lab, "");
        QString ss = key.replace("libost", "");
        ss.replace(0, 1, ss.at(0).toUpper());
        dyntext->setValue(ss, false);
        dyntext->setAutoUpdate(true);
        dyntext->setDirectEdit(true);
        dyntext->setPostIcon("forward");
        dynprop->addElt(key, dyntext);

    }
    createProperty("load", dynprop);

}
void Maincontrol::addModuleData(const QString  &pName, const QString  &pLabel, const QString  &pType,
                                const QString  &pProfile)
{
    getEltString("modules", "name")->setValue(pName);
    getEltString("modules", "label")->setValue(pLabel);
    getEltString("modules", "type")->setValue(pType);
    getEltString("modules", "profile")->setValue(pProfile);


    getStore()["modules"]->push();
}
void Maincontrol::setModuleData(const QString  &pName, const QString  &pLabel, const QString  &pType,
                                const QString  &pProfile)
{
    Q_UNUSED(pLabel);
    Q_UNUSED(pType);
    QVariantList l;
    for (int i = 0; i < getProperty("modules")->getGrid().size(); i++)
    {
        l.append(getString("modules", "name", i));
    }
    for (int i = 0; i < l.count(); i++)
    {
        if( l[i].toString() == pName)
        {
            //getEltString("modules", "profile")->getGrid()[i] = pProfile;
            getEltString("modules", "profile")->setValue(pProfile, true);
            getProperty("modules")->getGrid()[i]["profile"]->updateValue();
        }
    }

}
void Maincontrol::deldModuleData(const QString  &pName)
{
    QVariantList l;
    for (int i = 0; i < getProperty("modules")->getGrid().size(); i++)
    {
        l.append(getString("modules", "name", i));
    }
    for (int i = 0; i < l.count(); i++)
    {
        if( l[i].toString() == pName)
        {
            getStore()["modules"]->deleteLine(i);
        }
    }

}
void Maincontrol::searchDriver(void)
{
    getProperty("indidrivers")->clearGrid();
    QString srch = getString("indidrivers", "search");

    foreach(QString key, mIndiDriverList)
    {
        if (key.contains(srch))
        {
            getEltString("indidrivers", "driver")->setValue(key, false);
            getProperty("indidrivers")->push();

        }
    }
}
void Maincontrol::setIndiDriverList(const QStringList pDrivers)
{
    mIndiDriverList = pDrivers;
    OST::PropertyMulti *dynprop2 = new OST::PropertyMulti("indidrivers", "Available indi drivers", OST::Permission::ReadWrite,
            "Indi server",
            "", "1", false, true);
    OST::ElementString* dyntext = new OST::ElementString("Search", "20", "");
    dyntext->setValue("*", false);
    dyntext->setAutoUpdate(true);
    dyntext->setDirectEdit(true);
    dyntext->setPostIcon("play_arrow");

    dynprop2->setHasGrid(true);
    dynprop2->setShowGrid(true);
    dynprop2->setShowElts(true);
    OST::ElementString* dyntext2 = new OST::ElementString("Indi driver", "10", "");
    dyntext2->setValue("", false);
    dyntext2->setAutoUpdate(true);
    dyntext2->setDirectEdit(true);
    dyntext2->setPreIcon("stop");
    dyntext2->setPostIcon("play_arrow");
    dynprop2->addElt("driver", dyntext2);
    dynprop2->addElt("search", dyntext);

    foreach(QString key, mIndiDriverList)
    {
        dyntext2->setValue(key, false);
        dynprop2->push();
    }
    createProperty("indidrivers", dynprop2);

}
