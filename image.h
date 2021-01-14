#ifndef IMAGE_h_
#define IMAGE_h_
#pragma once

#include <basedevice.h>
#include <memory>

//Includes for this project
#include <structuredefinitions.h>
#include <stellarsolver.h>
#define cimg_display 0
#include <CImg.h>

using namespace cimg_library;


class Image : public QObject
{
    Q_OBJECT
public:
    Image();
    ~Image();
// Stellasolver stuff
    FITSImage::Statistic stats;
    //std::unique_ptr<StellarSolver> stellarSolver =nullptr;
    QPointer<StellarSolver> stellarSolver;
    QList<FITSImage::Star> stars;
    uint8_t m_Channels { 1 };
    uint8_t *m_ImageBuffer { nullptr };
    uint32_t m_ImageBufferSize { 0 };
    float HFRavg;
    CImg<uint16_t> img;

    bool LoadFromBlob(IBLOB *bp);
    bool saveToJpeg(QString filename);
    void ResetData(void);
    void CalcStats(void);
    void FindStars(void);
    void SolveStars(void);
    bool FindStarsFinished = true;
    bool SolveStarsFinished = true;
public slots:
    void sslogOutput(QString text);
    void ssReadySEP(void);
    void ssReadySolve(void);
signals:
    void successSEP(void);
    void successSolve(void);

};

#endif
