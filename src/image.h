#ifndef OSTIMAGE_h_
#define OSTIMAGE_h_

#include <basedevice.h>
#include <memory>
#include <QtCore>
#include <QtConcurrent>
#include <structuredefinitions.h>
#include <stellarsolver.h>
#include <boost/log/trivial.hpp>

class Image : public QObject
{
    Q_OBJECT

public:
    Image();
    ~Image();

    float HFRavg;
    FITSImage::Statistic stats;
    uint8_t *m_ImageBuffer { nullptr };
    //CImg<uint16_t> img;
    //CImg<uint32_t> histogram;
    //CImg<uint16_t> img_stretched;
    //CImg<double>   histogram256;
    bool LoadFromBlob(IBLOB *bp);
    bool LoadFromBlob2(IBLOB *bp);
    bool LoadFromFile(QString filename);
    bool saveToJpeg(QString filename,int compress);
    bool saveMapToJpeg(QString filename,int compress,QList<FITSImage::Star> stars);
    bool saveStretchedToJpeg(QString filename,int compress);
    void ResetData(void);
    void CalcStats(void);
    void computeHistogram(void);
    void appendStarsFound(QList<FITSImage::Star> stars);
protected:
    void deleteImageBuffer(void);
    uint8_t m_Channels { 1 };
    uint32_t m_ImageBufferSize { 0 };


};

#endif
