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
}

Monitor::~Monitor()
{
}

void Monitor::appendEvent(const QString &module, const QString &type,
                          const QString &key, double valNum, const QString &valStr)
{
    QVariantMap row;
    row["ts"]      = QDateTime::currentDateTime();
    row["module"]  = module;
    row["type"]    = type;
    row["key"]     = key;
    row["val_num"] = valNum;
    row["val_str"] = valStr;
    getStore()["events"]->newLine(row);
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
