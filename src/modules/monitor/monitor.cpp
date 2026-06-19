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

void Monitor::onExternalEvent(OST::ExtEvent event)
{
    Q_UNUSED(event)
}

void Monitor::onOtherModuleEvent(OST::EvType ev, QString mod, QString prp, QString elt, QVariant data, int line)
{
    Q_UNUSED(line)

    if (ev != OST::EvType::ea) return;
    if (!isWatchedModule(mod)) return;

    QJsonObject e = data.toJsonValue().toObject()["e"].toObject();

    if (prp == "signals")
    {
        QString event = e["event"].toString();
        appendEvent(mod, e["statedescription"].toString(), event,
                    e["state"].toInt(), e["eventdescription"].toString());
        if (event == "focusdone" && mPendingHFR.contains(mod))
        {
            appendEvent(mod, "metric", "hfr", mPendingHFR.take(mod), "");
        }
    }
    else if (prp == "values" && elt == "rmsTotal")
    {
        appendEvent(mod, "metric", "rmsRA",    e["rmsRA"].toDouble(),    "");
        appendEvent(mod, "metric", "rmsDEC",   e["rmsDEC"].toDouble(),   "");
        appendEvent(mod, "metric", "rmsTotal", e["rmsTotal"].toDouble(), "");
    }
    else if (prp == "results" && elt == "hfr")
    {
        mPendingHFR[mod] = e["hfr"].toDouble();
    }
}
