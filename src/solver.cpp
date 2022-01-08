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
    BOOST_LOG_TRIVIAL(debug) << "Reset solver";
    HFRavg=99;
    stellarSolver = new StellarSolver(stats, m_ImageBuffer);
    stellarSolver->moveToThread(this->thread());
    stellarSolver->setParent(this);
    stellarSolverProfiles=StellarSolver::getBuiltInProfiles();
}

void Solver::FindStars(Parameters param)
{

    if (!connect(stellarSolver,&StellarSolver::logOutput,this,&Solver::sslogOutput))
        BOOST_LOG_TRIVIAL(debug) << "Can't connect logOutput";
    if (!connect(stellarSolver,&StellarSolver::ready,this,&Solver::ssReadySEP))
        BOOST_LOG_TRIVIAL(debug) << "Can't connect ready";
    if (!connect(stellarSolver,&StellarSolver::finished,this,&Solver::ssFinished))
        BOOST_LOG_TRIVIAL(debug) << "Can't connect finished";


    //QList<Parameters> params = stellarSolver->getBuiltInProfiles();
    stellarSolver->setParameters(param);
    //stellarSolver->clearSubFrame();
    stellarSolver->setProperty("ProcessType",EXTRACT_WITH_HFR);
    //stellarSolver->setProperty("ProcessType",SOLVE);
    stellarSolver->setProperty("ExtractorType",EXTRACTOR_INTERNAL);
    stellarSolver->setProperty("SolverType",SOLVER_STELLARSOLVER);

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


    BOOST_LOG_TRIVIAL(debug) << "SS command string into solver" << stellarSolver->getCommandString().toStdString();

    //stellarSolver->extract(true);
    if (stellarSolver->failed()) BOOST_LOG_TRIVIAL(debug) << "SS Failed";

}
void Solver::SolveStars()
{


}


void Solver::ssFinished()
{
    BOOST_LOG_TRIVIAL(debug) << "solve finished";

}
void Solver::ssReadySEP()
{

    BOOST_LOG_TRIVIAL(debug) << "SSolver ready SEP";
    stars = stellarSolver->getStarList();
    for (int i=0;i<stars.size();i++)
    {
        HFRavg=(i*HFRavg + stars[i].HFR)/(i+1);
    }
    BOOST_LOG_TRIVIAL(debug) << "SSolver HFRavg = " << HFRavg;
    disconnect(stellarSolver,&StellarSolver::ready,this,&Solver::ssReadySEP);
    emit successSEP();
    return;
}

void Solver::ssReadySolve()
{
    BOOST_LOG_TRIVIAL(debug) << "SSolver ready solve";
    stars = stellarSolver->getStarListFromSolve();
    for (int i=0;i<stars.size();i++)
    {
        //IDLog("IMG HFR %i %f\n",i,stars[i].HFR);
        HFRavg=(i*HFRavg + stars[i].HFR)/(i+1);
    }

    BOOST_LOG_TRIVIAL(debug) << "SSolver HFRavg = " << HFRavg;
    emit successSolve();
}
void Solver::sslogOutput(QString text)
{
    BOOST_LOG_TRIVIAL(debug) << "SSolver log : " << text.toStdString();
}
