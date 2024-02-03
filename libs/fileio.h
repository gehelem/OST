#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QImageReader>
#include <QFile>
#include <QVariant>
#include <basedevice.h>
#include <baseclient.h>
#include <libstellarsolver/parameters.h>
#include <libstellarsolver/structuredefinitions.h>

//CFitsio Includes
#include "longnam.h"
#include "fitsio.h"

//KStars related includes
#include "stretch.h"
#include "math.h"
#include "dms.h"
#include "bayer.h"
#include <valueimg.h>


class fileio : public QObject
{
        Q_OBJECT
    public:

        /** Structure to hold FITS Header records */
        typedef struct
        {
            QString key;      /** FITS Header Key */
            QVariant value;   /** FITS Header Value */
            QString comment;  /** FITS Header Comment, if any */
        } Record;


        fileio();
        ~fileio();
        void deleteImageBuffer();
        bool logToSignal = false;
        bool loadImage(QString fileName);
        bool loadImageBufferOnly(QString fileName);
        bool loadFits(QString fileName);
        bool loadBlob(INDI::PropertyBlob pblob)
        {
            return loadBlob(pblob, 32);
        }
        bool loadBlob(INDI::PropertyBlob pblob, int histoSize);
        bool parseHeader();
        bool saveAsFITS(QString fileName, FITSImage::Statistic &imageStats, uint8_t *m_ImageBuffer, FITSImage::Solution solution,
                        QList<Record> &records, bool hasSolution);
        bool saveAsFITSSimple(QString fileName);

        bool loadOtherFormat(QString fileName);
        bool checkDebayer();
        bool debayer();
        bool debayer_8bit();
        bool debayer_16bit();
        bool getSolverOptionsFromFITS();

        bool position_given = false;
        double ra;
        double dec;

        bool scale_given = false;
        double scale_low;
        double scale_high;
        SSolver::ScaleUnits scale_units;

        bool imageBufferTaken = false;
        uint8_t *getImageBuffer();

        FITSImage::Statistic getStats()
        {
            return stats;
        }

        const QList<Record> &getRecords() const
        {
            return m_HeaderRecords;
        }

        void setRecords(QList<Record> &records)
        {
            m_HeaderRecords = records;
        }

        QImage getRawQImage()
        {
            return rawImage;
        }
        const QVector<uint32_t> &getCumulativeFrequency(uint8_t channel = 0) const
        {
            return m_CumulativeFrequency[channel];
        }
        const QVector<double> &getHistogramIntensity(uint8_t channel = 0) const
        {
            return m_HistogramIntensity[channel];
        }
        const QVector<double> &getHistogramFrequency(uint8_t channel = 0) const
        {
            return m_HistogramFrequency[channel];
        }
        OST::ImgData ImgStats();

    private:
        QString file;
        fitsfile *fptr { nullptr };
        FITSImage::Statistic stats;
        QList<Record> m_HeaderRecords;
        /// Generic data image buffer
        uint8_t *m_ImageBuffer { nullptr };
        /// Above buffer size in bytes
        uint32_t m_ImageBufferSize { 0 };
        bool justLoadBuffer = false;
        StretchParams stretchParams;
        BayerParams debayerParams;
        void logIssue(QString messsage);

        QImage rawImage;
        void CalcStats(int size);
        void generateQImage();

        template <typename T>
        QPair<T, T> getParitionMinMax(uint32_t start, uint32_t stride);
        template <typename T>
        void calculateMinMax();
        template <typename T>
        void calculateMedian();
        template <typename T>
        void runningAverageStdDev();
        template <typename T>
        QPair<double, double> getSquaredSumAndMean(uint32_t start, uint32_t stride);
        template <typename T>
        void CalcHisto(int size);

        QVector<QVector<uint32_t>> m_CumulativeFrequency;
        QVector<QVector<double>> m_HistogramIntensity;
        QVector<QVector<double>> m_HistogramFrequency;
        QVector<double> m_HistogramBinWidth;
        uint16_t m_HistogramBinCount { 0 };
        double m_JMIndex { 1 };
        bool m_HistogramConstructed { false };


    signals:
        void logOutput(QString logText);

    private:
        void DummyFunctionToAvoidDefinedButNotUsedWarnings(void)
        {
            FITSImage::getColorChannelText(FITSImage::ColorChannel::RED);
            FITSImage::getShortParityText(FITSImage::Parity::BOTH);
            FITSImage::getParityText(FITSImage::Parity::BOTH);
            SSolver::getLogLevelString(SSolver::logging_level::LOG_ALL);
            SSolver::getMultiAlgoString(SSolver::MultiAlgo::MULTI_AUTO);
            SSolver::getCommandString(SSolver::ProcessType::EXTRACT, SSolver::EXTRACTOR_INTERNAL, SSolver::SOLVER_STELLARSOLVER);
            SSolver::getScaleUnitString(SSolver::ScaleUnits::ARCMIN_WIDTH);
            SSolver::getConvFilterString(SSolver::ConvFilterType::CONV_CUSTOM);
            SSolver::getShapeString(SSolver::SHAPE_CIRCLE);

        }
        void sendMessage(const QString &pMessage);

};

#endif // FILEIO_H
