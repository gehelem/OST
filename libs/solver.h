#ifndef SOLVER_h_
#define SOLVER_h_
#include <QCoreApplication>
#include <QtCore>
#include <memory>
#include <QtConcurrent>

//Includes for this project
#include <stellarsolver.h>

class Solver : public QObject
{
        Q_OBJECT
    public:
        Solver();
        ~Solver();

        // Stellasolver stuff
        StellarSolver stellarSolver;
        QList<SSolver::Parameters> stellarSolverProfiles;

        QList<FITSImage::Star> stars;

        float HFRavg;
        int HFRZones = 1; /* default 1 : 1x1 - 2: 2x2 - 3: 3x3 ... */
        QList<float> HFRavgZone;
        QList<int> HFRavgCount;
        QList<float> thetaAvgZone;
        QList<float> thetaDevAvgZone; // theta deviation
        QList<float> aAxeAvgZone;
        QList<float> bAxeAvgZone;
        QList<float> eAxeAvgZone; // a/b
        void ResetSolver(FITSImage::Statistic &stats, uint8_t *m_ImageBuffer);
        void ResetSolver(FITSImage::Statistic &stats, uint8_t *m_ImageBuffer, int zones);
        void FindStars(Parameters param);
        void SolveStars(Parameters param);

    public slots:
        void sslogOutput(QString text);
        void ssReadySEP();
        void ssReadySolve();
        void ssFinished();
    signals:
        void successSEP(void);
        void successSolve(void);
        void solverLog(QString &text);
    private:
        void DummyFunctionToAvoidDefinedButNotUsedWarnings(void)
        {
            FITSImage::getColorChannelText(FITSImage::ColorChannel::RED);
            FITSImage::getShortParityText(FITSImage::Parity::BOTH);
            FITSImage::getParityText(FITSImage::Parity::BOTH);
        }
        void sendMessage(const QString &pMessage);
        int mImgWidth = 0;
        int mImgHeight = 0;

};
#endif
