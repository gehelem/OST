#include "indipanel.h"
#include "version.cc"

static void atomicSaveJpeg(const QImage &img, const QString &finalPath)
{
    const QString tmp = finalPath + ".tmp";
    if (img.save(tmp, "JPG", 100))
        ::rename(tmp.toLocal8Bit().constData(), finalPath.toLocal8Bit().constData());
}

IndiPanel *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    IndiPanel *basemodule = new IndiPanel(name, label, profile, availableModuleLibs);
    return basemodule;
}

IndiPanel::IndiPanel(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    setMetadata("thisGithash", QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Indi control panel");
    setMetadata("thisversion", QString::fromStdString(Version::GIT_TAG));

    connectIndi();
}

IndiPanel::~IndiPanel()
{

}
void IndiPanel::onNewDevice(INDI::BaseDevice dp)
{
    auto props = dp.getProperties();

    for (auto pProperty : props)
    {
        QString dev = pProperty.getDeviceName();
        QString pro = pProperty.getName();
        QString devpro = dev + pro;
        QString mess;
        OST::PropertyMulti *pm = new OST::PropertyMulti(devpro, pProperty.getLabel(),
                OST::IntToPermission(pProperty.getPermission()),
                pProperty.getDeviceName(),
                pProperty.getGroupName(), "00", false, false);
        createProperty(pm);
    }
}
void IndiPanel::onRemoveDevice(INDI::BaseDevice dp)
{
    for(const QString &key : getStore().keys())    {
        getProperty(key)->level1();
        if (strcmp(dp.getDeviceName(), getProperty(key)->level1().toStdString().c_str()) == 0)
        {
            deleteOstProperty(key);
        }
    }
}
void IndiPanel::onNewProperty(INDI::Property pProperty)
{
    //logDebug("IndiPanel::onNewProperty %1 %2", {pProperty.getDeviceName(), pProperty.getName()});

    QString dev = pProperty.getDeviceName();
    QString pro = pProperty.getName();
    QString devpro = dev + pro;
    QString mess;

    /* force group for blob types */
    QString group;
    if (pProperty.getType() != INDI_BLOB) group = pProperty.getGroupName();
    else group = pProperty.getLabel();


    OST::PropertyMulti* p = new OST::PropertyMulti(devpro, pProperty.getLabel(),
            OST::IntToPermission(pProperty.getPermission()),
            pProperty.getDeviceName(),
            group, "00", false, false);
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
                    setBLOBMode(B_ALSO, wdp.getDeviceName(), nullptr);
                }

            }
            for (unsigned int i = 0; i < n.count(); i++)
            {
                OST::ElementFloat* v = new OST::ElementFloat(n[i].getName(), n[i].label, QString::number(i), n[i].label);
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
                OST::ElementBool* v = new OST::ElementBool(s[i].getName(), s[i].label, QString::number(i), s[i].label);
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
                OST::ElementString* v = new OST::ElementString(t[i].getName(), t[i].label, QString::number(i), t[i].label);
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
                OST::ElementLight* v = new OST::ElementLight(l[i].getName(), l[i].label, QString::number(i), l[i].label);
                v->setValue(OST::IntToState(l[i].getState()), true);
                p->addElt(v);
            }
            break;
        }
        case INDI_BLOB:
        {
            INDI::PropertyBlob b = pProperty;
            setBLOBMode(B_ALSO, b.getDeviceName(), nullptr);
            for (unsigned int i = 0; i < b.count(); i++)
            {
                OST::ElementImg* ei = new OST::ElementImg(b[i].getName(), b[i].label, QString::number(i), b[i].label);
                delete _image;
                _image = new fileio();
                OST::ImgData dta;
                if (_image->loadBlob(b, 64, i))
                {
                    QImage rawImage = _image->getRawQImage();
                    atomicSaveJpeg(rawImage, getWebroot() + "/" + getModuleName() + QString(b.getDeviceName()) + b[i].label + ".jpeg");
                    dta = _image->ImgStats();
                    dta.mUrlJpeg = getModuleName() + QString(b.getDeviceName()) + b[i].label + ".jpeg";
                    dta.mUrlFits = getModuleName() + QString(b.getDeviceName()) + b[i].label + ".FITS";
                }
                ei->setValue(dta, true);
                p->addElt(ei);
            }
            break;
        }
        case INDI_UNKNOWN:
        {
            break;
        }
    }
    createProperty(p);
}
void IndiPanel::onUpdateProperty (INDI::Property property)
{
    //    logDebug("IndiPanel::onUpdateProperty %1 %2", {property.getDeviceName(), property.getName()});

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
                o[t[i].name] = QString::fromUtf8(t[i].text);
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
            INDI::PropertyBlob b = property;
            for (unsigned int i = 0; i < b.count(); i++)
            {
                OST::ElementImg* ei = getEltImg(devpro, b[i].getName());
                delete _image;
                _image = new fileio();
                OST::ImgData dta;

                if (_image->loadBlob(b, 64, i))
                {
                    QImage rawImage = _image->getRawQImage();
                    atomicSaveJpeg(rawImage, getWebroot() + "/" + getModuleName() + QString(b.getDeviceName()) + b[i].label + ".jpeg");
                    dta = _image->ImgStats();
                    dta.mUrlJpeg = getModuleName() + QString(b.getDeviceName()) + b[i].label + ".jpeg";
                    dta.mUrlFits = getModuleName() + QString(b.getDeviceName()) + b[i].label + ".FITS";
                }
                ei->setValue(dta, true);
            }
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


void IndiPanel::onRemoveProperty(INDI::Property property)
{
    //    logDebug("IndiPanel::onRemoveProperty %1 %2", {property.getDeviceName(), property.getName()});

    QString dev = property.getDeviceName();
    QString pro = property.getName();
    QString devpro = dev + pro;
    //BOOST_LOG_TRIVIAL(debug) << "indi remove property " << devpro.toStdString();
    deleteOstProperty(devpro);
}

void IndiPanel::newMessage     (INDI::BaseDevice dp, int messageID)
{
    logInfo("%1 %2", {dp.getDeviceName(), QString::fromStdString(dp.messageQueue(messageID))});
}

void IndiPanel::onExternalEvent(OST::ExtEvent event)
{
    //qDebug() << "IndiPanel::onExternalEvent event = " << OST::ExtEvToString(event.ev) << " p=" << event.prpkey << " e=" <<
    //         event.eltkey << " l=" << event.lovkey << " i=" << event.line;

    /* don't process Indimodule specific events again */
    if (getStore().value(event.prpkey)->hasProfile()) return;
    //if (getStore().value(event.prpkey)->level1() == "Parameters") return;
    //if (getStore().value(event.prpkey)->level1() == "Module") return;

    QJsonObject o = event.data["m"].toObject()[this->getModuleName()].toObject();
    QJsonObject p = o["p"].toObject();
    QJsonObject e = p[event.prpkey].toObject()["e"].toObject();


    if (event.ev == OST::ExtEvType::SV)
    {
        QString realDevice = getStore().value(event.prpkey)->getFreeValue();
        QString realprop = event.prpkey;
        realprop.replace(realDevice, "");
        if (getStore().value(event.prpkey)->getElt(event.eltkey)->getType() == "bool")
        {
            bool b = e.begin().value().toBool();
            if ( b) sendModNewSwitch(realDevice, realprop, event.eltkey, ISS_ON);
            if (!b) sendModNewSwitch(realDevice, realprop, event.eltkey, ISS_OFF);
        }
    }

    if (event.ev == OST::ExtEvType::SA)
    {
        QJsonObject e = p[event.prpkey].toObject()["e"].toObject();
        QString eltkey = e.begin().key();
        QString realDevice = getStore().value(event.prpkey)->getFreeValue();
        QString realprop = event.prpkey;
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
}


