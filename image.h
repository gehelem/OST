#ifndef IMAGE_h_
#define IMAGE_h_

#include <basedevice.h>
#include <memory>
#include <QtCore>
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
    CImg<uint32_t> histogram;
    CImg<uint16_t> img_stretched;
    CImg<double>   histogram256;
    bool LoadFromBlob(IBLOB *bp);
    bool saveToJpeg(QString filename,int compress);
    bool saveStretchedToJpeg(QString filename,int compress);
    void ResetData(void);
    void CalcStats(void);
    void FindStars(void);
    void computeHistogram(void);
    void SolveStars(void);
    void appendStarsFound(void);
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
