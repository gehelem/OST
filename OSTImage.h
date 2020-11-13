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

class OSTImage : public QObject
{
    Q_OBJECT
public:
    OSTImage();
    ~OSTImage();
// Stellasolver stuff
    FITSImage::Statistic stats;
    std::unique_ptr<StellarSolver> stellarSolver =nullptr;
    //StellarSolver *stellarSolver;
    QList<FITSImage::Star> stars;
    // Number of channels
    uint8_t m_Channels { 1 };
    // Generic data image buffer
    uint8_t *m_ImageBuffer { nullptr };
    // Above buffer size in bytes
    uint32_t m_ImageBufferSize { 0 };

    bool LoadFromBlob(IBLOB *bp);
    void ResetData(void);
    void CalcStats(void);
    void FindStars(void);
//public slots:
    void sslogOutput(QString text);
    void ssReady(void);
};

#endif
