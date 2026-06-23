#include "meteo.h"
#include "versionModule.cc"

Meteo *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Meteo *basemodule = new Meteo(name, label, profile, availableModuleLibs);
    return basemodule;
}

Meteo::Meteo(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)

{

    loadOstPropertiesFromFile(":meteo.json");
    setMetadata("thisGithash", QString::fromStdString(VersionModule::GIT_SHA1));
    setMetadata("thisGitdate", QString::fromStdString(VersionModule::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(VersionModule::GIT_COMMIT_SUBJECT));
    setMetadata("description", "Meteo module");
    setMetadata("thisversion", QString::fromStdString(VersionModule::GIT_TAG));
    setMetadata("template", "default");

    connectIndi();
    connectAllDevices();

    auto* i = new OST::ElementInt("interval", "Refresh interval (s)", "0", "");
    i->setValue(5, false);
    i->setDirectEdit(true);
    i->setAutoUpdate(true);
    getProperty("parms")->addElt( i);
    i = new OST::ElementInt("histo", "Keep x values", "0", "");
    i->setValue(10, false);
    i->setDirectEdit(true);
    i->setAutoUpdate(true);
    getProperty("parms")->addElt( i);
    connect(&mTimer, &QTimer::timeout, this, &Meteo::OnTimer);
    mTimer.start(getInt("parms", "interval") * 1000);

}

Meteo::~Meteo()
{

}
void Meteo::onExternalEvent(OST::ExtEvent event)
{

    if (event.ev == OST::ExtEvType::SV && event.prpkey == "parms")
    {
        if (event.eltkey == "interval")
        {
            if (getEltBool(event.prpkey, event.eltkey)->setValue(true, true))
            {
                mTimer.stop();
                mTimer.start(getInt("parms", "interval") * 1000);
            }

        }
    }

    if (event.prpkey == "selection")
    {
        //get elements data from event data
        QJsonObject e =
            event.data["m"].toObject()[this->getModuleName()].toObject()["p"].toObject()[event.prpkey].toObject()["e"].toObject();

        if (event.ev == OST::ExtEvType::GC)
        {
            getProperty(event.prpkey)->newLine(e.toVariantMap());
            QString id = getString("selection", "dpv", getProperty("selection")->getGrid().size() - 1);
            declareNewGraph(id);
        }
        if (event.ev == OST::ExtEvType::GD)
        {
            QString id = getString("selection", "dpv", event.line);
            getStore()[event.prpkey]->deleteLine(event.line);
            deleteOstProperty(id);

        }
    }

    if (event.ev == OST::ExtEvType::I3 && event.prpkey == "search")
    {
        updateSearchList();
    }

    if (event.ev == OST::ExtEvType::I4 && event.prpkey == "search")
    {
        addMeasure(getString("search", "dpvsearchid"));
    }

}
void Meteo::newProperty(INDI::Property property)
{
    if (property.getType() == INDI_NUMBER)
    {
        INDI::PropertyNumber n = property;
        for (unsigned int i = 0; i < n.count(); i++)
        {
            QString propname = QString(n.getDeviceName()) +  "-" + n.getName() + "-" + n[i].getName();
            if (!mAvailableMeasures.contains(propname))
            {
                QString lab = QString(n.getDeviceName()) + "-" +  n.getLabel() + "-" + n[i].getLabel();
                mAvailableMeasures[propname] = lab;
                /*this goes too fast and kills everything - we'll se that later as we dont really need that
                getEltString("selection", "dpv")->lovAdd(propname, lab);*/

                //logInfo(lab);
            }
            QStringList propnames;
            for (int i = 0; i < getProperty("selection")->getGrid().size(); i++)
            {
                propnames.append(getString("selection", "dpv", i));
            }
            if ( propnames.contains(propname))
            {
                declareNewGraph(propname);
                getEltFloat(propname, "time")->setValue(QDateTime::currentDateTime().toMSecsSinceEpoch(), false);
                getEltFloat(propname, propname)->setValue(n[i].getValue(), false);
            }
        }
    }


}
void Meteo::updateProperty(INDI::Property property)
{
    if (property.getType() == INDI_NUMBER)
    {
        INDI::PropertyNumber n = property;
        for (unsigned int i = 0; i < n.count(); i++)
        {
            QString propname = QString(n.getDeviceName()) +  "-" + n.getName() + "-" + n[i].getName();
            QStringList propnames;
            for (int i = 0; i < getProperty("selection")->getGrid().size(); i++)
            {
                propnames.append(getString("selection", "dpv", i));
            }
            if ( propnames.contains(propname))
            {
                declareNewGraph(propname);
                getProperty(propname)->setGridLimit(getInt("parms", "histo"));
                getEltFloat(propname, "time")->setValue(QDateTime::currentDateTime().toMSecsSinceEpoch(), false);
                getEltFloat(propname, propname)->setValue(n[i].getValue(), false);
            }
        }
    }

}
void Meteo::initIndi()
{
    connectIndi();
}
void Meteo::OnTimer()
{
    //QVariantList propnames = getOstElementGrid("selection", "dpv");
    QStringList propnames;

    for (int i = 0; i < getProperty("selection")->getGrid().size(); i++)
    {
        propnames.append(getString("selection", "dpv", i));
    }
    for (int i = 0; i < propnames.count(); i++)
    {
        QString propname = propnames[i];
        declareNewGraph(propname);
        if (getStore().contains(propname))
        {
            getProperty(propname)->setGridLimit(getInt("parms", "histo"));
            getEltFloat(propname, "time")->setValue(QDateTime::currentDateTime().toMSecsSinceEpoch(), true);
            getProperty(propname)->push();
        }
    }


}
void Meteo::declareNewGraph(const QString  &pName)
{
    if (getStore().contains(pName))
    {
        return;
    }
    if (!mAvailableMeasures.contains(pName))
    {
        logInfo("Can't follow " + pName + " at the moment");
        return;
    }
    QString lab = mAvailableMeasures[pName];
    OST::PropertyMulti* pm = new OST::PropertyMulti(pName, lab, OST::ReadOnly, "Measures", lab, 0, false, true);
    pm->setShowGrid(false);
    pm->setGridLimit(getInt("parms", "histo"));
    pm->setShowElts(false);
    auto* t = new OST::ElementFloat("time", "Time", "00", "");
    pm->addElt( t);
    t = new OST::ElementFloat(pName, pName, "10", "");
    pm->addElt( t);

    OST::GraphDefs def;
    def.type = OST::GraphType::DY;
    def.params["D"] = "time";
    def.params["Y"] = pName;
    pm->setGraphDefs(def);

    createProperty(pm);

}
void Meteo::updateSearchList(void)
{
    QString sid = getString("search", "dpvsearchid");
    QString slab = getString("search", "dpvsearchlab");
    logInfo("Searching " + sid + "/" + slab);
    getProperty("search")->clearGrid();

    for (auto i = mAvailableMeasures.cbegin(), end = mAvailableMeasures.cend(); i != end; ++i)
    {
        if ((i.key().contains(sid)) && (i.value().contains(slab)))
        {
            getEltString("search", "dpvsearchid")->setValue(i.key(), false);
            getEltString("search", "dpvsearchlab")->setValue(i.value(), false);
            getProperty("search")->push();
        }
    }


}

void Meteo::addMeasure(QString s)
{
    for (auto i = mAvailableMeasures.cbegin(), end = mAvailableMeasures.cend(); i != end; ++i)
    {
        if (i.key() == s)
        {
            logInfo("Adding " + s);
            getEltString("selection", "dpv")->setValue(i.key(), false);
            getProperty("selection")->push();
        }
    }
}
