#ifndef SOLVER_h_
#define SOLVER_h_
#include <QCoreApplication>
#include <QtCore>
#include <memory>
#include <QtConcurrent>

//Includes for this project
#include <structuredefinitions.h>
#include <stellarsolver.h>
#include <boost/log/trivial.hpp>

class Solver : public QObject
{
    Q_OBJECT
public:
    Solver();
    ~Solver();

    // Stellasolver stuff
        QPointer<StellarSolver> stellarSolver;
        QList<SSolver::Parameters> stellarSolverProfiles;

        QList<FITSImage::Star> stars;
        float HFRavg;
        void ResetSolver(FITSImage::Statistic &stats, uint8_t *m_ImageBuffer);
        void FindStars(void);
        void SolveStars(void);
public slots:
        void sslogOutput(QString text);
        void ssReadySEP();
        void ssReadySolve();
        void ssFinished();
signals:
        void successSEP(void);
        void successSolve(void);

};
#endif
