#include "monitor.h"
#include "version.cc"
#include <propertyjsondumper.h>

Monitor *initialize(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
{
    Monitor *basemodule = new Monitor(name, label, profile, availableModuleLibs);
    return basemodule;
}

Monitor::Monitor(QString name, QString label, QString profile, QVariantMap availableModuleLibs)
    : IndiModule(name, label, profile, availableModuleLibs)
{
    loadOstPropertiesFromFile(":monitor.json");

    setMetadata("thisGithash",    QString::fromStdString(Version::GIT_SHA1));
    setMetadata("thisGitdate",    QString::fromStdString(Version::GIT_DATE));
    setMetadata("thisGitmessage", QString::fromStdString(Version::GIT_COMMIT_SUBJECT));
    setMetadata("thisversion",    QString::fromStdString(Version::GIT_TAG));
    setMetadata("description",    "Session activity monitor");
    setMetadata("template",       "monitor");

    giveMeAnActions();
    giveMeAState();

    OST::PropertyMulti* pm = getProperty("actions");
    OST::ElementBool* b = new OST::ElementBool("start", "Start", "010", "");
    b->setValue(false, false);
    pm->addElt(b);
    b = new OST::ElementBool("stop", "Stop", "020", "");
    b->setValue(false, false);
    pm->addElt(b);
}

Monitor::~Monitor()
{
}

bool Monitor::isWatchedModule(const QString &mod)
{
    auto *elts = getProperty("slaves")->getElts();
    for (auto it = elts->begin(); it != elts->end(); ++it)
    {
        if (getString("slaves", it.key()) == mod) return true;
    }
    return false;
}

void Monitor::appendEvent(const QString &module, const QString &type,
                          const QString &key, double valNum, int valInt, const QString &valStr)
{
    QDateTime now = QDateTime::currentDateTime();
    QVariantMap ts;
    ts["year"]  = now.date().year();
    ts["month"] = now.date().month();
    ts["day"]   = now.date().day();
    ts["hh"]    = now.time().hour();
    ts["mm"]    = now.time().minute();
    ts["ss"]    = now.time().second();
    ts["ms"]    = now.time().msec();

    QVariantMap row;
    row["ts"]      = ts;
    row["module"]  = module;
    row["type"]    = type;
    row["key"]     = key;
    row["val_num"] = valNum;
    row["val_int"] = valInt;
    row["val_str"] = valStr;
    getStore()["events"]->newLine(row);
}

void Monitor::startSession()
{
    mSessionStart = QDateTime::currentDateTime();
    getStore()["events"]->clearGrid();
    getProperty("actions")->setState(OST::Busy, true);
    getEltBool("actions", "start")->setValue(true,  false);
    getEltBool("actions", "stop") ->setValue(false, true);
    setStateEvent(OST::Busy, "recording", "sessionstart", "Session started");
}

void Monitor::stopSession()
{
    setStateEvent(OST::Ok, "ready", "sessionstop", "Session stopped");
    getProperty("actions")->setState(OST::Ok, true);
    getEltBool("actions", "start")->setValue(false, false);
    getEltBool("actions", "stop") ->setValue(false, true);

    if (!mSessionStart.isValid()) return;

    QDir().mkpath(getWebroot() + "/monitor");
    QString filename = getWebroot() + "/monitor/" +
                       mSessionStart.toString("yyyyMMdd-HHmmss") + ".json";

    auto *prop = getStore()["events"];
    OST::PropertyJsonDumper dumper(OST::EvType::av, QVariant(), nullptr, prop);
    prop->accept(&dumper);

    QJsonObject root;
    root["session_start"] = mSessionStart.toString(Qt::ISODate);
    root["module"]        = getModuleName();
    root["events"]        = dumper.getResult();

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(root).toJson());
}

void Monitor::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV && event.prpkey == "actions")
    {
        if (event.eltkey == "start") startSession();
        if (event.eltkey == "stop")  stopSession();
    }
}

void Monitor::onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line)
{
    Q_UNUSED(elt)
    Q_UNUSED(line)

    if (ev != OST::EvType::ea) return;
    if (!isWatchedModule(mod)) return;

    QJsonObject e = data.toJsonValue().toObject()["e"].toObject();

    if (prp != "signals") return;

    appendEvent(mod, e["statedescription"].toString(), e["event"].toString(),
                e["val_num"].toDouble(), e["val_int"].toInt(), e["val_str"].toString());
}
