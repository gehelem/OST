#include "indipanel.h"
#include "version.cc"

IndiPanel *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    IndiPanel *basemodule = new IndiPanel(name, label, profile, availableModuleLibs);
    return basemodule;
}

IndiPanel::IndiPanel(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    getEltString("thisGit", "hash")->setValue(QString::fromStdString(Version::GIT_SHA1), true);
    getEltString("thisGit", "date")->setValue(QString::fromStdString(Version::GIT_DATE), true);
    getEltString("thisGit", "message")->setValue(QString::fromStdString(Version::GIT_COMMIT_SUBJECT), true);

    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Indi control panel");
    setMetadata("thisversion", "0.11");

    connectIndi();
}

IndiPanel::~IndiPanel()
{

}
void IndiPanel::newDevice(INDI::BaseDevice dp)
{
    auto props = dp.getProperties();
    //BOOST_LOG_TRIVIAL(debug) << "Indipanel new device" << dp->getDeviceName();

    for (auto pProperty : props)
    {
        QString dev = pProperty.getDeviceName();
        QString pro = pProperty.getName();
        QString devpro = dev + pro;
        //BOOST_LOG_TRIVIAL(debug) << "Indipanel new property " << devpro.toStdString();
        QString mess;
        OST::PropertyMulti *pm = new OST::PropertyMulti(devpro, pProperty.getLabel(),
                OST::IntToPermission(pProperty.getPermission()),
                pProperty.getDeviceName(),
                pProperty.getGroupName(), "00", false, false);
        createProperty(pm);
    }
}
void IndiPanel::removeDevice(INDI::BaseDevice dp)
{
    foreach(const QString &key, getStore().keys())
    {
        getProperty(key)->level1();
        if (strcmp(dp.getDeviceName(), getProperty(key)->level1().toStdString().c_str()) == 0)
        {
            deleteOstProperty(key);
        }
    }

}
void IndiPanel::newProperty(INDI::Property pProperty)
{
    QString dev = pProperty.getDeviceName();
    QString pro = pProperty.getName();
    QString devpro = dev + pro;
    //sendMessage("Indipanel new property " + devpro);
    QString mess;

    OST::PropertyMulti* p = new OST::PropertyMulti(devpro, pProperty.getLabel(),
            OST::IntToPermission(pProperty.getPermission()),
            pProperty.getDeviceName(),
            pProperty.getGroupName(), "00", false, false);
    p->setFreeValue(dev); // we keep original device name to avoid unwanted level1 device translations
    p->setState(OST::IntToState(pProperty.getState()), false);

    switch (pProperty.getType())
    {

        case INDI_NUMBER:
        {
            INDI::PropertyNumber n = pProperty;

            if ( (strcmp(pProperty.getName(), "CCD_EXPOSURE") == 0) ||
                    (strcmp(pProperty.getName(), "GUIDER_EXPOSURE") == 0) )
            {
                INDI::BaseDevice wdp = getDevice(pProperty.getDeviceName());
                if (wdp.getDriverInterface() & INDI::BaseDevice::CCD_INTERFACE)
                {
                    //BOOST_LOG_TRIVIAL(debug) << "Setting blob mode for " << _wdp->getDeviceName();
                    setBLOBMode(B_ALSO, wdp.getDeviceName(), nullptr);
                }

            }
            for (unsigned int i = 0; i < n.count(); i++)
            {
                OST::ElementFloat* v = new OST::ElementFloat(n[i].getName(), n[i].label, QString(i), n[i].label);
                v->setValue(n[i].getValue(), false);
                //v->setMin(n[i].min, false); // commented as for example indi allows 0 with min=0.01 for CCD_EXPOSURE ... let's indi make controls
                //v->setMax(n[i].max, false); // "  "   "   "
                v->setStep(n[i].step, false);
                v->setFormat(n[i].format, false);
                p->addElt(v);
            }
            break;
        }
        case INDI_SWITCH:
        {
            INDI::PropertySwitch s = pProperty;
            for (unsigned int i = 0; i < s.count(); i++)
            {
                OST::ElementBool* v = new OST::ElementBool(s[i].getName(), s[i].label, QString(i), s[i].label);
                if (s[i].s == 0) v->setValue(false, false);
                if (s[i].s == 1) v->setValue(true, true);
                p->addElt(v);
            }
            p->setRule(OST::IntToRule(s.getRule()));

            break;
        }
        case INDI_TEXT:
        {
            INDI::PropertyText t = pProperty;
            for (unsigned int i = 0; i < t.count(); i++)
            {
                OST::ElementString* v = new OST::ElementString(t[i].getName(), t[i].label, QString(i), t[i].label);
                v->setValue(t[i].text, false);
                p->addElt(v);
            }
            break;
        }
        case INDI_LIGHT:
        {
            INDI::PropertyLight l = pProperty;
            for (unsigned int i = 0; i < l.count(); i++)
            {
                OST::ElementLight* v = new OST::ElementLight(l[i].getName(), l[i].label, QString(i), l[i].label);
                v->setValue(OST::IntToState(l[i].getState()), true);
                p->addElt(v);
            }
            break;
        }
        case INDI_BLOB:
        {

            break;
        }
        case INDI_UNKNOWN:
        {
            break;
        }
    }
    createProperty(p);


}
void IndiPanel::updateProperty (INDI::Property property)
{
    QString dev = property.getDeviceName();
    QString pro = property.getName();
    QString devpro = dev + pro;
    OST::PropertyMulti* p = getProperty(devpro);
    QVariantMap mm;
    QVariantMap o;
    switch (property.getType())
    {

        case INDI_NUMBER:
        {
            INDI::PropertyNumber n = property;
            for (unsigned int i = 0; i < n.count(); i++)
            {
                //qDebug() << "IndiPanel::updateProperty" << dev << pro << n[i].name << n[i].value;
                mm[n[i].name] = n[i].value;
                /*if (i == n.count() - 1)
                {
                    getEltFloat(devpro, n[i].name)->setValue(n[i].value, false);
                    //getEltFloat(devpro, n[i].name)->setMin(n[i].min,false); // commented as for example indi allows 0 with min=0.01 for CCD_EXPOSURE ... let's indi make controls
                    //getEltFloat(devpro, n[i].name)->setMax(n[i].max,false); // "  "   "
                    getEltFloat(devpro, n[i].name)->setStep(n[i].step, false);
                    getEltFloat(devpro, n[i].name)->setFormat(n[i].format, true); // only one event sent for all ...
                }

                else
                {
                    getEltFloat(devpro, n[i].name)->setValue(n[i].value, false);
                    getEltFloat(devpro, n[i].name)->setMin(n[i].min, false);
                    getEltFloat(devpro, n[i].name)->setMax(n[i].max, false);
                    getEltFloat(devpro, n[i].name)->setStep(n[i].step, false);
                    getEltFloat(devpro, n[i].name)->setFormat(n[i].format, false);
                }*/

            }
            getProperty(devpro)->setAll(mm);
            break;
        }
        case INDI_SWITCH:
        {
            INDI::PropertySwitch s = property;
            for (unsigned int i = 0; i < s.count(); i++)
            {
                if (s[i].s == 0) o[s[i].name] = false;
                else o[s[i].name] = true;
            }
            getProperty(devpro)->setAll(o);
            break;
        }
        case INDI_TEXT:
        {
            INDI::PropertyText t = property;

            for (unsigned int i = 0; i < t.count(); i++)
            {
                o[t[i].name] = t[i].text;
            }
            getProperty(devpro)->setAll(o);
            break;
        }
        case INDI_LIGHT:
        {
            INDI::PropertyLight l = property;
            for (unsigned int i = 0; i < l.count(); i++)
            {
                o[l[i].name] = l[i].getState();
            }
            getProperty(devpro)->setAll(o);
            break;
        }
        case INDI_BLOB:
        {

            break;
        }
        case INDI_UNKNOWN:
        {
            break;
        }
    }
    /* update state only if needed */
    if (p->state() != OST::IntToState(property.getState())) p->setState(OST::IntToState(property.getState()), true);
}


void IndiPanel::removeProperty(INDI::Property property)
{
    QString dev = property.getDeviceName();
    QString pro = property.getName();
    QString devpro = dev + pro;
    //BOOST_LOG_TRIVIAL(debug) << "indi remove property " << devpro.toStdString();
    deleteOstProperty(devpro);
}

void IndiPanel::newBLOB(IBLOB bp)
{
    Q_UNUSED(bp)
}
void IndiPanel::newMessage     (INDI::BaseDevice dp, int messageID)
{
    logInfo("%1 %2", {dp.getDeviceName(), QString::fromStdString(dp.messageQueue(messageID))});
}

void IndiPanel::onExternalEvent(OST::ExtEvent event)
{
    qDebug() << "IndiPanel::onExternalEvent event = " << OST::ExtEvToString(event.ev) << " p=" << event.prpkey << " e=" <<
             event.eltkey << " l=" << event.lovkey << " i=" << event.line;

    /* don't process Indimodule specific events again */
    if (getStore().value(event.prpkey)->level1() == "Parameters") return;

    QJsonObject o = event.data["m"].toObject()[this->getModuleName()].toObject();
    QJsonObject p = o["p"].toObject();
    QJsonObject e = p[event.prpkey].toObject()["e"].toObject();


    if (event.ev == OST::ExtEvType::SV)
    {
        QJsonObject e = p[event.prpkey].toObject()["e"].toObject();
        QString eltkey = e.begin().key();
        //QVariantMap ostprop = m[keyprop].toMap();
        //QString devcat = ostprop["level1"].toString();
        QString realDevice = getStore().value(event.prpkey)->getFreeValue();
        QString realprop = event.prpkey;
        //BOOST_LOG_TRIVIAL(debug) << "DEVCAT - recv : "  << devcat.toStdString();
        realprop.replace(realDevice, "");
        if (getStore().value(event.prpkey)->getElt(eltkey)->getType() == "bool")
        {
            bool b = e.begin().value().toBool();
            if ( b) sendModNewSwitch(realDevice, realprop, eltkey, ISS_ON);
            if (!b) sendModNewSwitch(realDevice, realprop, eltkey, ISS_OFF);
        }
    }

    if (event.ev == OST::ExtEvType::SA)
    {
        QJsonObject e = p[event.prpkey].toObject()["e"].toObject();
        QString eltkey = e.begin().key();
        //QVariantMap ostprop = m[keyprop].toMap();
        //QString devcat = ostprop["level1"].toString();
        QString realDevice = getStore().value(event.prpkey)->getFreeValue();
        QString realprop = event.prpkey;
        //BOOST_LOG_TRIVIAL(debug) << "DEVCAT - recv : "  << devcat.toStdString();
        realprop.replace(realDevice, "");
        INDI::BaseDevice dp = getDevice(realDevice.toStdString().c_str());
        if (!dp.isValid())
        {
            logError("Unable to find %2 device. Aborting.", {realDevice});
            return;
        }

        /* bools */
        if (getStore().value(event.prpkey)->getElt(eltkey)->getType() == "bool")
        {
            INDI::PropertySwitch prop = dp.getSwitch(realprop.toStdString().c_str());
            if (!prop.isValid())
            {
                logError("Unable to find %2/%3 property. Aborting.", {realDevice, realprop});
                return;
            }

            for (std::size_t i = 0; i < prop.size(); i++)
            {
                bool b = e.begin().value().toBool();
                if ( b) prop[i].setState(ISS_ON);
                if (!b) prop[i].setState(ISS_OFF);
                sendNewSwitch(prop);
            }
            return;
        }

        /* Numbers */
        if (getStore().value(event.prpkey)->getElt(eltkey)->getType() == "int"
                || getStore().value(event.prpkey)->getElt(eltkey)->getType() == "float")
        {
            INDI::PropertyNumber prop = dp.getNumber(realprop.toStdString().c_str());
            if (!prop.isValid())
            {
                logError("Unable to find %2/%3 property. Aborting.", {realDevice, realprop});
                return;
            }

            for (std::size_t i = 0; i < prop.size(); i++)
            {
                prop[i].value = e.value(prop[i].name).toDouble();
                sendNewNumber(prop);
            }
            return;
        }

        /* texts */
        if (getStore().value(event.prpkey)->getElt(eltkey)->getType() == "string")
        {
            INDI::PropertyText prop = dp.getText(realprop.toStdString().c_str());
            if (!prop.isValid())
            {
                logError("Unable to find %2/%3 property. Aborting.", {realDevice, realprop});
                return;
            }

            for (std::size_t i = 0; i < prop.size(); i++)
            {
                prop[i].setText(e.value(prop[i].name).toString().toStdString());
                sendNewText(prop);
            }
            return;
        }


    }

    //    if (e.module != this->getModuleName()) return;
    //
    //    QVariantMap m = getPropertiesDump(OST::Event()).toVariantMap();
    //    foreach(const QString &keyprop, e.data.keys())
    //    {
    //        if (!m.contains(keyprop))
    //        {
    //            logError("OnMyExternalEvent - property %1 does not exist (indipanel)", {keyprop});
    //            return;
    //        }
    //        QString prop = keyprop;
    //        QVariantMap ostprop = m[keyprop].toMap();
    //        QString devcat = ostprop["level1"].toString();
    //        QString realDevice = getStore()[keyprop]->getFreeValue();
    //        //BOOST_LOG_TRIVIAL(debug) << "DEVCAT - recv : "  << devcat.toStdString();
    //        prop.replace(realDevice, "");
    //        if (!(devcat == "Indi"))
    //        {
    //            foreach(const QString &keyelt, e.data[keyprop].toMap()["elements"].toMap().keys())
    //            {
    //                if (!m[keyprop].toMap()["e"].toMap().contains(keyelt))
    //                {
    //                    logError ("OnMyExternalEvent - property %1 element %2 does not exist (indipanel)", {keyprop, keyelt});
    //                    return;
    //                }
    //                if (getStore()[keyprop]->getElt(keyelt)->getType() == "string")
    //                {
    //                    sendModNewText(realDevice, prop, keyelt,
    //                                   e.data[keyprop].toMap()["elements"].toMap()[keyelt].toString());
    //                }
    //                if (getStore()[keyprop]->getElt(keyelt)->getType() == "int"
    //                        || getStore()[keyprop]->getElt(keyelt)->getType() == "float")
    //                {
    //                    //qDebug() << "indipanel - OnMyExternalEvent" << e.data[keyprop].toMap()["elements"].toMap()[keyelt];
    //                    sendModNewNumber(realDevice, prop, keyelt,
    //                                     e.data[keyprop].toMap()["elements"].toMap()[keyelt].toFloat());
    //                }
    //                if (getStore()[keyprop]->getElt(keyelt)->getType() == "bool")
    //                {
    //                    //qDebug() << "bool" << keyprop << keyelt << eventData[keyprop].toMap()["elements"].toMap()[keyelt];
    //                    keyelt.toStdString();
    //                    if ( e.data[keyprop].toMap()["elements"].toMap()[keyelt].toBool()) sendModNewSwitch(realDevice, prop,
    //                                keyelt, ISS_ON);
    //                    if (!e.data[keyprop].toMap()["elements"].toMap()[keyelt].toBool()) sendModNewSwitch(realDevice, prop,
    //                                keyelt, ISS_OFF);
    //                }
    //            }
    //
    //        }
    //
    //    }
}


