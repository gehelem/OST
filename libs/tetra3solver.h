#ifndef TETRA3SOLVER_H_
#define TETRA3SOLVER_H_
#include <QObject>
#include <QProcess>
#include <QString>
#include <structuredefinitions.h>

// Wraps esa/tetra3 (Python) as a subprocess, behind the same solve-only
// shape StellarSolver's SOLVE path already exposes (successSolve/
// failSolve/getSolution() returning a FITSImage::Solution) so navigator
// and polar can switch backends with a minimal call-site change. Tetra3
// has no SEP equivalent - see
// notes/NOTES-backend-tetra3-plate-solver-evaluation.md - focus/guider/
// inspector/sequencer stay on StellarSolver regardless of this class.
class Tetra3Solver : public QObject
{
        Q_OBJECT
    public:
        Tetra3Solver();
        ~Tetra3Solver();

        void setPythonInterpreter(const QString &path); // default "python3"
        void setScriptPath(const QString &path);         // tetra3_solve.py location
        void setDatabasePath(const QString &path);        // generated offline, see libs/tetra3solver/README.md

        // imageFitsPath: a FITS file already on disk (e.g. via
        // fileio::saveAsFITS()). fovEstimateDeg/fovMaxErrorDeg <= 0 means
        // "no hint, let tetra3 decide".
        void SolveStars(const QString &imageFitsPath, double fovEstimateDeg = 0.0, double fovMaxErrorDeg = 0.0);
        void abort();

        FITSImage::Solution getSolution() const
        {
            return mSolution;
        }

    signals:
        void successSolve();
        void failSolve();
        void solverLog(QString &text);

    private slots:
        void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void onErrorOccurred(QProcess::ProcessError error);

    private:
        void sendMessage(const QString &message);

        QProcess mProcess;
        QString mPythonInterpreter = "python3";
        QString mScriptPath;
        QString mDatabasePath;
        FITSImage::Solution mSolution {};
};
#endif
