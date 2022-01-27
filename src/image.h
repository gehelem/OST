#ifndef IMAGE_h_
#define IMAGE_h_

#include <basedevice.h>
#include <memory>
#include <QtCore>
#include <QtConcurrent>
//Includes for this project
#include <structuredefinitions.h>
#include <stellarsolver.h>
#define cimg_display 0
/*#define cimg_use_tiff 1
#define cimg_use_png 1
#define cimg_use_jpeg 1*/
#include <CImg.h>
#include <boost/log/trivial.hpp>

using namespace cimg_library;


class Image : public QObject
{
    Q_OBJECT

public:
    Image();
    ~Image();

    FITSImage::Statistic stats;
    uint8_t m_Channels { 1 };
    uint8_t *m_ImageBuffer { nullptr };
    uint32_t m_ImageBufferSize { 0 };
    float HFRavg;
    CImg<uint16_t> img;
    CImg<uint32_t> histogram;
    CImg<uint16_t> img_stretched;
    CImg<double>   histogram256;
    bool LoadFromBlob(IBLOB *bp);
    bool LoadFromFile(QString filename);
    bool saveToJpeg(QString filename,int compress);
    bool saveMapToJpeg(QString filename,int compress,QList<FITSImage::Star> stars);
    bool saveStretchedToJpeg(QString filename,int compress);
    void ResetData(void);
    void CalcStats(void);
    void computeHistogram(void);
    void appendStarsFound(QList<FITSImage::Star> stars);


};

#endif
