#include "monitor.h"
#include "version.cc"
#include <algorithm>
#include <cmath>

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
    getEltBool("actions", "stop") ->setValue(true, true);
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
    QDir dir(getWebroot() + "/monitor-data");
    if (!dir.exists()) return;

    QFileInfoList files = dir.entryInfoList({"*.json"}, QDir::Files, QDir::Name | QDir::Reversed);

    getStore()["archive"]->clearGrid();
    for (const QFileInfo &fi : files)
    {
        QString base     = fi.baseName();
        int sep          = base.indexOf('_');
        QString instance = (sep >= 0) ? base.left(sep)     : QString();
        QString datePart = (sep >= 0) ? base.mid(sep + 1)  : base;
        QDateTime dt     = QDateTime::fromString(datePart, "yyyyMMdd-HHmmss");
        QVariantMap row;
        row["date"]     = dt.isValid() ? dt.toString("yyyy-MM-dd HH:mm:ss") : datePart;
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

    QFile file(getWebroot() + "/monitor-data/" + filename);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonArray grid = QJsonDocument::fromJson(file.readAll()).object()["grid"].toArray();

    mEvents.clear();
    for (const QJsonValue &jRow : grid)
    {
        QJsonArray arr = jRow.toArray();
        if (arr.size() < 18) continue;
        if (!kChartKeys.contains(arr[3].toString())) continue;
        QVariantMap row;
        row["ts"]       = arr[0].toObject().toVariantMap();
        row["module"]   = arr[1].toString();
        row["type"]     = arr[2].toString();
        row["key"]      = arr[3].toString();
        row["val_num1"] = arr[4].toDouble();
        row["val_num2"] = arr[5].toDouble();
        row["val_num3"] = arr[6].toDouble();
        row["val_num4"] = arr[7].toDouble();
        row["val_num5"] = arr[8].toDouble();
        row["val_num6"] = arr[9].toDouble();
        row["val_num7"] = arr[10].toDouble();
        row["val_num8"] = arr[11].toDouble();
        row["val_int1"] = arr[12].toInt();
        row["val_int2"] = arr[13].toInt();
        row["val_int3"] = arr[14].toInt();
        row["val_str1"] = arr[15].toString();
        row["val_str2"] = arr[16].toString();
        row["val_str3"] = arr[17].toString();
        mEvents.append(row);
    }

    if (!mEvents.isEmpty())
    {
        auto tsToDateTime = [](const QVariantMap & ts)
        {
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

void Monitor::appendEvent(const QString &module, const QString &type, const QString &key,
                          double valNum1, double valNum2, double valNum3,
                          double valNum4, double valNum5, double valNum6,
                          double valNum7, double valNum8,
                          int valInt1, int valInt2, int valInt3,
                          const QString &valStr1, const QString &valStr2, const QString &valStr3)
{
    if (!kChartKeys.contains(key)) return;
    if (!mSessionActive) return;

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
    row["ts"]       = ts;
    row["module"]   = module;
    row["type"]     = type;
    row["key"]      = key;
    row["val_num1"] = valNum1;
    row["val_num2"] = valNum2;
    row["val_num3"] = valNum3;
    row["val_num4"] = valNum4;
    row["val_num5"] = valNum5;
    row["val_num6"] = valNum6;
    row["val_num7"] = valNum7;
    row["val_num8"] = valNum8;
    row["val_int1"] = valInt1;
    row["val_int2"] = valInt2;
    row["val_int3"] = valInt3;
    row["val_str1"] = valStr1;
    row["val_str2"] = valStr2;
    row["val_str3"] = valStr3;
    mEvents.append(row);
    getStore()["events"]->newLine(row);

    int maxRows = getInt("parms", "maxrows");
    if (maxRows > 0 && mEvents.size() >= maxRows)
    {
        logInfo("Session reached max rows limit (%1), archiving and starting new session", {maxRows});
        stopSession();
        startSession();
    }
}

void Monitor::startSession()
{
    mSessionStart  = QDateTime::currentDateTime();
    mSessionActive = true;
    mEvents.clear();
    mGuideRmsBuf.clear();
    mGuideSNRBuf.clear();
    int maxRows = getInt("parms", "maxrows");
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
    getEltBool("actions", "stop") ->setValue(true, true);

    if (!mSessionStart.isValid() || mEvents.isEmpty()) return;

    QDir().mkpath(getWebroot() + "/monitor-data");
    QString filename = getWebroot() + "/monitor-data/" +
                       getModuleName() + "_" + mSessionStart.toString("yyyyMMdd-HHmmss") + ".json";

    QJsonArray grid;
    for (const QVariantMap &row : mEvents)
    {
        QJsonArray jRow;
        jRow.append(QJsonObject::fromVariantMap(row["ts"].toMap()));
        jRow.append(row["module"].toString());
        jRow.append(row["type"].toString());
        jRow.append(row["key"].toString());
        jRow.append(row["val_num1"].toDouble());
        jRow.append(row["val_num2"].toDouble());
        jRow.append(row["val_num3"].toDouble());
        jRow.append(row["val_num4"].toDouble());
        jRow.append(row["val_num5"].toDouble());
        jRow.append(row["val_num6"].toDouble());
        jRow.append(row["val_num7"].toDouble());
        jRow.append(row["val_num8"].toDouble());
        jRow.append(row["val_int1"].toInt());
        jRow.append(row["val_int2"].toInt());
        jRow.append(row["val_int3"].toInt());
        jRow.append(row["val_str1"].toString());
        jRow.append(row["val_str2"].toString());
        jRow.append(row["val_str3"].toString());
        grid.append(jRow);
    }

    QJsonObject root;
    root["session_start"] = mSessionStart.toString(Qt::ISODate);
    root["module"]        = getModuleName();
    root["gridheaders"]   = QJsonArray({"ts", "module", "type", "key",
                                        "val_num1", "val_num2", "val_num3", "val_num4",
                                        "val_num5", "val_num6", "val_num7", "val_num8",
                                        "val_int1", "val_int2", "val_int3",
                                        "val_str1", "val_str2", "val_str3"});
    root["grid"]          = grid;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(root).toJson());

    scanArchive();
}

void Monitor::refreshView()
{
    int maxRows = getInt("parms", "maxrows");
    if (maxRows <= 0 || maxRows > 2000) maxRows = 2000;

    QVector<const QVariantMap*> matching;
    for (const QVariantMap &row : mEvents)
        matching.append(&row);

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

    QString key  = e["event"].toString();
    double  num  = e["val_num"].toDouble();

    auto flushBuf = [&](QVector<double> &buf, const QString & bufMod, const QString & bufType, const QString & bufKey)
    {
        QVector<double> sorted = buf;
        std::sort(sorted.begin(), sorted.end());
        double mean = 0;
        for (double v : sorted) mean += v;
        mean /= sorted.size();
        double vmin   = sorted.first();
        double vmax   = sorted.last();
        double median = sorted.size() % 2 == 0
                        ? (sorted[sorted.size() / 2 - 1] + sorted[sorted.size() / 2]) / 2.0
                        : sorted[sorted.size() / 2];
        double variance = 0;
        for (double v : sorted) variance += (v - mean) * (v - mean);
        double stddev = std::sqrt(variance / sorted.size());
        appendEvent(bufMod, bufType, bufKey, mean, vmin, vmax, median, stddev);
        buf.clear();
    };

    int sampling = getInt("parms", "monitor_sampling");

    if (key == "guideRMS" || key == "guideSNR")
    {
        QVector<double> &buf    = (key == "guideRMS") ? mGuideRmsBuf    : mGuideSNRBuf;
        QString         &bufMod = (key == "guideRMS") ? mGuideRmsModule  : mGuideSNRModule;
        QString         &bufType = (key == "guideRMS") ? mGuideRmsType    : mGuideSNRType;
        if (sampling <= 1)
        {
            appendEvent(mod, e["statedescription"].toString(), key, num);
        }
        else
        {
            bufMod  = mod;
            bufType = e["statedescription"].toString();
            buf.append(num);
            if (buf.size() >= sampling)
                flushBuf(buf, bufMod, bufType, key);
        }
    }
    else
    {
        appendEvent(mod, e["statedescription"].toString(), key,
                    num, 0, 0, 0, 0, 0, 0, 0,
                    e["val_int"].toInt(), 0, 0,
                    e["val_str"].toString());
    }
}
