#include "solver.h"
/*!
 * ... ...
 */
Solver::Solver()
{
}


Solver::~Solver()
{
}

void Solver::ResetSolver(FITSImage::Statistic &stats, uint8_t *m_ImageBuffer)
{
    //sendMessage( "Reset solver";
    HFRavg = 99;
    delete stellarSolver;
    stellarSolver = new StellarSolver(stats, m_ImageBuffer);
    stellarSolver->moveToThread(this->thread());
    stellarSolver->setParent(this);
    stellarSolverProfiles = StellarSolver::getBuiltInProfiles();
}

void Solver::FindStars(Parameters param)
{

    if (!connect(stellarSolver, &StellarSolver::logOutput, this, &Solver::sslogOutput))
        sendMessage( "Can't connect sslogOutput");
    if (!connect(stellarSolver, &StellarSolver::ready, this, &Solver::ssReadySEP))
        sendMessage( "Can't connect ssReadySEP");
    if (!connect(stellarSolver, &StellarSolver::finished, this, &Solver::ssFinished))
        sendMessage( "Can't connect ssFinished");


    //QList<Parameters> params = stellarSolver->getBuiltInProfiles();
    stellarSolver->setParameters(param);
    //stellarSolver->clearSubFrame();
    stellarSolver->setProperty("ProcessType", EXTRACT_WITH_HFR);
    //stellarSolver->setProperty("ProcessType",SOLVE);
    stellarSolver->setProperty("ExtractorType", EXTRACTOR_INTERNAL);
    stellarSolver->setProperty("SolverType", SOLVER_STELLARSOLVER);

    //stellarSolver->setProperty("SolverType",SOLVER_STELLARSOLVER);
    stellarSolver->setLogLevel(LOG_ALL);
    stellarSolver->setSSLogLevel(LOG_VERBOSE);
    stellarSolver->setProperty("LogToFile", true);
    //stellarSolver->setProperty("UseScale", false);
    //stellarSolver->setProperty("UsePosition", false);
    stellarSolver->setProperty("LogFileName", "/home/gilles/projets/OST/solver.log");
    //stellarSolver->setLoadWCS(false);
    //stellarSolver->setParameters();
    //qDebug() << "SS command string " << stellarSolver->getCommandString();
    //QEventLoop loop;
    //connect(stellarSolver, &StellarSolver::finished, &loop, &QEventLoop::quit);
    //stellarSolver->extract(true);
    stellarSolver->start();
    //loop.exec(QEventLoop::ExcludeUserInputEvents);
    //IDLog("IMG stellarSolver SEP Start\n");


    //sendMessage( "SS command string into solver" << stellarSolver->getCommandString().toStdString();

    //stellarSolver->extract(true);
    if (stellarSolver->failed()) sendMessage( "SS Failed");

}
void Solver::SolveStars(Parameters param)
{
    if (!connect(stellarSolver, &StellarSolver::logOutput, this, &Solver::sslogOutput))
        sendMessage( "Can't connect sslogOutput");
    if (!connect(stellarSolver, &StellarSolver::ready, this, &Solver::ssReadySolve))
        sendMessage( "Can't connect ssReadySolve");
    if (!connect(stellarSolver, &StellarSolver::finished, this, &Solver::ssFinished))
        sendMessage( "Can't connect ssFinished");


    //QList<Parameters> params = stellarSolver->getBuiltInProfiles();
    stellarSolver->setParameters(param);
    //stellarSolver->clearSubFrame();
    //stellarSolver->setProperty("ProcessType",EXTRACT_WITH_HFR);
    stellarSolver->setProperty("ProcessType", SOLVE);
    stellarSolver->setProperty("ExtractorType", EXTRACTOR_INTERNAL);
    stellarSolver->setProperty("SolverType", SOLVER_STELLARSOLVER);

    //stellarSolver->setProperty("SolverType",SOLVER_STELLARSOLVER);
    stellarSolver->setLogLevel(LOG_ALL);
    stellarSolver->setSSLogLevel(LOG_VERBOSE);
    stellarSolver->setProperty("LogToFile", true);
    //stellarSolver->setProperty("UseScale", false);
    //stellarSolver->setProperty("UsePosition", false);
    stellarSolver->setProperty("LogFileName", "/home/gilles/projets/OST/solver.log");
    //stellarSolver->setLoadWCS(false);
    //stellarSolver->setParameters();
    //qDebug() << "SS command string " << stellarSolver->getCommandString();
    //QEventLoop loop;
    //connect(stellarSolver, &StellarSolver::finished, &loop, &QEventLoop::quit);
    //stellarSolver->extract(true);
    stellarSolver->start();
    //loop.exec(QEventLoop::ExcludeUserInputEvents);
    //IDLog("IMG stellarSolver SEP Start\n");


    //sendMessage( "SS command string into solver" << stellarSolver->getCommandString().toStdString();

    //stellarSolver->extract(true);
    if (stellarSolver->failed()) sendMessage( "SS Failed");

}


void Solver::ssFinished()
{
    //sendMessage( "solve finished";

}
void Solver::ssReadySEP()
{

    //sendMessage( "SSolver ready SEP";
    stars = stellarSolver->getStarList();
    for (int i = 0; i < stars.size(); i++)
    {
        HFRavg = (i * HFRavg + stars[i].HFR) / (i + 1);
    }
    //sendMessage( "SSolver Ready : HFRavg = " + QString::number(HFRavg));
    disconnect(stellarSolver, &StellarSolver::ready, this, &Solver::ssReadySEP);
    emit successSEP();
    return;
}

void Solver::ssReadySolve()
{
    emit successSolve();
}
void Solver::sslogOutput(QString text)
{
    emit solverLog(text);
}
void Solver::sendMessage(const QString &pMessage)
{
    QString messageWithDateTime = "[" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "]-" + pMessage;
    QDebug debug = qDebug();
    debug.noquote();
    debug << messageWithDateTime;
}
