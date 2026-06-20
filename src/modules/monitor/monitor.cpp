#include "monitor.h"
#include "version.cc"

static const QSet<QString> kChartKeys = { "guideRMS", "guideSNR", "imagehfr", "focusdone" };

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
    QTimer::singleShot(1000, this, &Monitor::onAutoStart);

    OST::PropertyMulti* pm = getProperty("actions");
    OST::ElementBool* b = new OST::ElementBool("start", "Start", "010", "");
    b->setValue(false, false);
    pm->addElt(b);
    b = new OST::ElementBool("stop", "Stop", "020", "");
    b->setValue(false, false);
    pm->addElt(b);

    getProperty("events")->setGridLimit(1000000);
}

Monitor::~Monitor()
{
}

void Monitor::onAutoStart()
{
    scanArchive();
    if (getBool("parms", "autostart"))
        startSession();
}

void Monitor::scanArchive()
{
    QDir dir(getWebroot() + "/monitor");
    if (!dir.exists()) return;

    QFileInfoList files = dir.entryInfoList({"*.json"}, QDir::Files, QDir::Name | QDir::Reversed);

    getStore()["archive"]->clearGrid();
    for (const QFileInfo &fi : files)
    {
        QDateTime dt = QDateTime::fromString(fi.baseName(), "yyyyMMdd-HHmmss");
        QVariantMap row;
        row["date"]     = dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm:ss") : fi.baseName();
        row["filename"] = fi.fileName();
        row["size"]     = (int)(fi.size() / 1024);
        getStore()["archive"]->newLine(row);
    }
}

void Monitor::loadArchive(int line)
{
    getProperty("archive")->fetchLine(line);
    QString filename = getString("archive", "filename");
    if (filename.isEmpty()) return;

    QFile file(getWebroot() + "/monitor/" + filename);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonArray grid = QJsonDocument::fromJson(file.readAll()).object()["grid"].toArray();

    mEvents.clear();
    for (const QJsonValue &jRow : grid)
    {
        QJsonArray arr = jRow.toArray();
        if (arr.size() < 7) continue;
        if (!kChartKeys.contains(arr[3].toString())) continue;
        QVariantMap row;
        row["ts"]      = arr[0].toObject().toVariantMap();
        row["module"]  = arr[1].toString();
        row["type"]    = arr[2].toString();
        row["key"]     = arr[3].toString();
        row["val_num"] = arr[4].toDouble();
        row["val_int"] = arr[5].toInt();
        row["val_str"] = arr[6].toString();
        mEvents.append(row);
    }

    if (!mEvents.isEmpty())
    {
        auto tsToDateTime = [](const QVariantMap &ts) {
            return QDateTime(QDate(ts["year"].toInt(), ts["month"].toInt(), ts["day"].toInt()),
                             QTime(ts["hh"].toInt(), ts["mm"].toInt(), ts["ss"].toInt(), ts["ms"].toInt()));
        };
        QDateTime tsMin = tsToDateTime(mEvents.first()["ts"].toMap());
        QDateTime tsMax = tsMin;
        for (const QVariantMap &row : mEvents)
        {
            QDateTime dt = tsToDateTime(row["ts"].toMap());
            if (dt < tsMin) tsMin = dt;
            if (dt > tsMax) tsMax = dt;
        }
        getEltDateTime("filter", "ts_start")->setValue(tsMin, true);
        getEltDateTime("filter", "ts_end")->setValue(tsMax, true);
    }

    refreshView();
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
    if (!kChartKeys.contains(key)) return;
    mEvents.append(row);
    getStore()["events"]->newLine(row);
    getEltDateTime("filter", "ts_end")->setValue(now, true);
}

void Monitor::startSession()
{
    mSessionStart  = QDateTime::currentDateTime();
    mSessionActive = true;
    mEvents.clear();
    getEltDateTime("filter", "ts_start")->setValue(mSessionStart, true);
    getEltDateTime("filter", "ts_end")->setValue(mSessionStart, true);
    int maxRows = getInt("filter", "maxrows");
    if (maxRows <= 0) maxRows = 200;
    getStore()["events"]->setGridLimit(maxRows);
    getStore()["events"]->clearGrid();
    getProperty("actions")->setState(OST::Busy, true);
    getEltBool("actions", "start")->setValue(true,  false);
    getEltBool("actions", "stop") ->setValue(false, true);
    setStateEvent(OST::Busy, "recording", "sessionstart", "Session started");
}

void Monitor::stopSession()
{
    mSessionActive = false;
    setStateEvent(OST::Ok, "ready", "sessionstop", "Session stopped");
    getProperty("actions")->setState(OST::Ok, true);
    getEltBool("actions", "start")->setValue(false, false);
    getEltBool("actions", "stop") ->setValue(false, true);

    if (!mSessionStart.isValid() || mEvents.isEmpty()) return;

    QDir().mkpath(getWebroot() + "/monitor");
    QString filename = getWebroot() + "/monitor/" +
                       mSessionStart.toString("yyyyMMdd-HHmmss") + ".json";

    QJsonArray grid;
    for (const QVariantMap &row : mEvents)
    {
        QJsonArray jRow;
        jRow.append(QJsonObject::fromVariantMap(row["ts"].toMap()));
        jRow.append(row["module"].toString());
        jRow.append(row["type"].toString());
        jRow.append(row["key"].toString());
        jRow.append(row["val_num"].toDouble());
        jRow.append(row["val_int"].toInt());
        jRow.append(row["val_str"].toString());
        grid.append(jRow);
    }

    QJsonObject root;
    root["session_start"] = mSessionStart.toString(Qt::ISODate);
    root["module"]        = getModuleName();
    root["gridheaders"]   = QJsonArray({"ts", "module", "type", "key", "val_num", "val_int", "val_str"});
    root["grid"]          = grid;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(root).toJson());

    scanArchive();
}

void Monitor::refreshView()
{
    QDateTime tsStart = getEltDateTime("filter", "ts_start")->value();
    QDateTime tsEnd   = getEltDateTime("filter", "ts_end")->value();
    int maxRows = getInt("filter", "maxrows");
    if (maxRows <= 0 || maxRows > 2000) maxRows = 2000;

    QVector<const QVariantMap*> matching;
    for (const QVariantMap &row : mEvents)
    {
        QVariantMap ts = row["ts"].toMap();
        QDateTime dt(QDate(ts["year"].toInt(), ts["month"].toInt(), ts["day"].toInt()),
                     QTime(ts["hh"].toInt(),   ts["mm"].toInt(),   ts["ss"].toInt(), ts["ms"].toInt()));
        if (tsStart.isValid() && dt < tsStart) continue;
        if (tsEnd.isValid()   && dt > tsEnd)   continue;
        matching.append(&row);
    }

    getStore()["events"]->clearGrid();
    int start = qMax(0, matching.size() - maxRows);
    for (int i = start; i < matching.size(); ++i)
        getStore()["events"]->newLine(*matching[i], true);
    getStore()["events"]->emitAll();
}

void Monitor::onExternalEvent(OST::ExtEvent event)
{
    if (event.ev == OST::ExtEvType::SV)
    {
        if (event.prpkey == "actions")
        {
            if (event.eltkey == "start") startSession();
            if (event.eltkey == "stop")  stopSession();
        }
        if (event.prpkey == "filter") refreshView();
    }
    if (event.ev == OST::ExtEvType::GF && event.prpkey == "archive")
    {
        if (mSessionActive)
            logWarning("Cannot load archive while a session is active");
        else
            loadArchive(event.line);
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
