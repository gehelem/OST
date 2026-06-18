#include "monitor.h"
#include "version.cc"

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

    connect(&mTimer, &QTimer::timeout, this, &Monitor::onHeartbeat);
    mTimer.start(1000);
}

Monitor::~Monitor()
{
}

void Monitor::appendEvent(const QString &module, const QString &type,
                          const QString &key, double valNum, const QString &valStr)
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
    row["val_str"] = valStr;
    getStore()["events"]->newLine(row);
}

void Monitor::onHeartbeat()
{
    appendEvent("monitor", "heartbeat", "tick", 0.0, "");
}

void Monitor::onExternalEvent(OST::ExtEvent event)
{
    Q_UNUSED(event)
}

void Monitor::onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line)
{
    Q_UNUSED(ev)
    Q_UNUSED(mod)
    Q_UNUSED(prp)
    Q_UNUSED(elt)
    Q_UNUSED(data)
    Q_UNUSED(line)
}
