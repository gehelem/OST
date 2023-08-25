#ifndef VALUEIMG_h
#define VALUEIMG_h

#include <valuebase.h>

namespace  OST
{

class ValueImg: public ValueBase
{

        Q_OBJECT

    public:
        void accept(ValueVisitor *pVisitor) override
        {
            pVisitor->visit(this);
        }
        void accept(ValueVisitor *pVisitor, QVariantMap &data) override
        {
            pVisitor->visit(this, data);
        }

        ValueImg(const QString &label, const QString &order, const QString &hint);
        ~ValueImg();
        QString urlJpeg()
        {
            return mUrlJpeg;
        }
        void setUrlJpeg(const QString &url)
        {
            mUrlJpeg = url;
        }
        QString urlFits()
        {
            return mUrlFits;
        }
        void setUrlFits(const QString &url)
        {
            mUrlFits = url;
        }
        QString urlOverlay()
        {
            return mUrlOverlay;
        }
        void setUrlOverlay(const QString &url)
        {
            mUrlOverlay = url;
        }
        QString urlThumbnail()
        {
            return mUrlThumbnail;
        }
        void setUrlThumbnail(const QString &url)
        {
            mUrlThumbnail = url;
        }
        long min()
        {
            return mMin;
        }
        void setMin(const long &min)
        {
            mMin = min;
        }
        long max()
        {
            return mMax;
        }
        void setMax(const long &max)
        {
            mMax = max;
        }
        long height()
        {
            return mHeight;
        }
        void setHeight(const long &height)
        {
            mHeight = height;
        }
        long width()
        {
            return mWidth;
        }
        void setWidth(const long &width)
        {
            mWidth = width;
        }
        double mean()
        {
            return mMean;
        }
        void setMean(const double &mean)
        {
            mMean = mean;
        }
        long median()
        {
            return mMedian;
        }
        void setMedian(const long &median)
        {
            mMedian = median;
        }
        double stddev()
        {
            return mStdDev;
        }
        void setStddev(const double &stddev)
        {
            mStdDev = stddev;
        }
        double SNR()
        {
            return mSNR;
        }
        void setSNR(const double &snr)
        {
            mSNR = snr;
        }

    private:
        QString mUrlJpeg = "";
        QString mUrlFits = "";
        QString mUrlOverlay = "";
        QString mUrlThumbnail = "";
        long mMin = 0;
        long mMax = 0;
        long mHeight = 0;
        long mWidth = 0;
        double mMean = 0;
        long mMedian = 0;
        double mStdDev = 0;
        double mSNR = 0;


};

}
#endif
