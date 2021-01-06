#ifndef OSTImage_h_
#define OSTImage_h_

#pragma once
#include <basedevice.h>
#include <memory>

//Includes for this project
#include <structuredefinitions.h>
#include <stellarsolver.h>
//QT Includes
//QT Includes
#include <QDir>
#include <QThread>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QRect>
#include <QPointer>
#include <QtNetwork>
#include <QImage>
#define cimg_display 0
#include <CImg.h>

using namespace cimg_library;


class OSTImage : public QObject
{
    Q_OBJECT
public:
    OSTImage();
    ~OSTImage();
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
    QImage jpegmem;
public slots:
    void sslogOutput(QString text);
    void ssReadySEP(void);
    void ssReadySolve(void);
signals:
    void successSEP(void);
    void successSolve(void);

};

#endif
