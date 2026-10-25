#include "tetra3solver.h"
#include <QJsonDocument>
#include <QJsonObject>

Tetra3Solver::Tetra3Solver()
{
    connect(&mProcess, &QProcess::finished,      this, &Tetra3Solver::onFinished);
    connect(&mProcess, &QProcess::errorOccurred, this, &Tetra3Solver::onErrorOccurred);
}

Tetra3Solver::~Tetra3Solver()
{
    if (mProcess.state() != QProcess::NotRunning)
    {
        mProcess.kill();
        mProcess.waitForFinished(1000);
    }
}

void Tetra3Solver::setPythonInterpreter(const QString &path)
{
    mPythonInterpreter = path;
}
void Tetra3Solver::setScriptPath(const QString &path)
{
    mScriptPath = path;
}
void Tetra3Solver::setDatabasePath(const QString &path)
{
    mDatabasePath = path;
}

void Tetra3Solver::SolveStars(const QString &imageFitsPath, double fovEstimateDeg, double fovMaxErrorDeg)
{
    mSolution = FITSImage::Solution();

    QStringList args;
    args << mScriptPath << "--image" << imageFitsPath;
    if (!mDatabasePath.isEmpty())
        args << "--database" << mDatabasePath;
    if (fovEstimateDeg > 0.0)
        args << "--fov-estimate" << QString::number(fovEstimateDeg);
    if (fovMaxErrorDeg > 0.0)
        args << "--fov-max-error" << QString::number(fovMaxErrorDeg);

    sendMessage("Tetra3 solve: " + mPythonInterpreter + " " + args.join(" "));
    mProcess.start(mPythonInterpreter, args);
}

void Tetra3Solver::abort()
{
    if (mProcess.state() != QProcess::NotRunning)
        mProcess.kill();
}

void Tetra3Solver::onErrorOccurred(QProcess::ProcessError error)
{
    sendMessage(QString("Tetra3 process error: %1").arg(error));
    emit failSolve();
}

void Tetra3Solver::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    const QByteArray stderrOutput = mProcess.readAllStandardError();
    if (!stderrOutput.isEmpty())
        sendMessage("Tetra3 stderr: " + QString::fromUtf8(stderrOutput));

    if (exitStatus != QProcess::NormalExit)
    {
        emit failSolve();
        return;
    }

    const QByteArray output = mProcess.readAllStandardOutput();
    const QJsonDocument doc = QJsonDocument::fromJson(output);
    if (!doc.isObject())
    {
        sendMessage("Tetra3: could not parse JSON output: " + QString::fromUtf8(output));
        emit failSolve();
        return;
    }

    const QJsonObject obj = doc.object();
    if (exitCode != 0 || !obj.value("solved").toBool())
    {
        emit failSolve();
        return;
    }

    mSolution.ra          = obj.value("ra").toDouble();
    mSolution.dec         = obj.value("dec").toDouble();
    mSolution.orientation = obj.value("orientation").toDouble();
    mSolution.fieldWidth  = obj.value("fieldWidthArcmin").toDouble();
    mSolution.fieldHeight = obj.value("fieldHeightArcmin").toDouble();
    mSolution.pixscale    = obj.value("pixscaleArcsecPerPix").toDouble();

    emit successSolve();
}

void Tetra3Solver::sendMessage(const QString &pMessage)
{
    QString text = pMessage;
    emit solverLog(text);
}
