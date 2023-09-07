#ifndef COMMON_VALUE_h_
#define COMMON_VALUE_h_

#include <QtCore>

namespace  OST
{

typedef enum
{
    Idle = 0, /*!< State is idle */
    Ok,       /*!< State is ok */
    Busy,     /*!< State is busy */
    Error     /*!< State is error */
} State;
inline State IntToState(int val )
{
    if (val == 0) return Idle;
    if (val == 1) return Ok;
    if (val == 2) return Busy;
    if (val == 3) return Error;
    qDebug() << "Cant convert " << val << " to OST::State (0-3) - defaults to Idle";
    return Idle;
}
inline int StateToInt(State val )
{
    if (val == Idle) return 0;
    if (val == Ok) return 1;
    if (val == Busy) return 2;
    if (val == Error) return 3;
    qDebug() << "StateToInt Cant convert state - return State::Error";
    return 3;
}
typedef enum
{
    Info = 0,
    Warn,
    Err
} MsgLevel;

template <typename U>
class ListOfValues
{
    public:
        QMap<U, QString> getList()
        {
            return list;
        }
        bool add(U val, QString label)
        {
            if (list.contains(val))
            {
                qDebug() << "ListOfValues - add - key " << val << " already exists (" << list[val] << ").";
                return false;
            }
            list[val] = label;
            return true;
        }
        bool update(U val, QString label)
        {
            if (!list.contains(val))
            {
                qDebug() << "ListOfValues - update - key " << val << " doesn't exist (" << list[val] << ").";
                return false;
            }
            list[val] = label;
            return true;
        }
        bool del(U val)
        {
            if (!list.contains(val))
            {
                qDebug() << "ListOfValues - del - key " << val << " does not exist.";
                return false;
            }
            list.remove(val);
            return true;
        }
        bool clear()
        {
            list.clear();
            return true;
        }

    private:
        QMap<U, QString> list;
};

template <typename U>
class Grid
{
    public:
        QList<U> getGrid()
        {
            return mGridValues;
        }
        bool add(U val)
        {
            mGridValues.append(val);
            return true;
        }
        bool del(int i)
        {
            if (i >= mGridValues.size())
            {
                qDebug() << "Grid - del - trying to delete item " << i << " while grid size is " << mGridValues.size();
                return false;
            }
            mGridValues.removeAt(i);
            return true;
        }
        bool update(U val, int i)
        {
            if (i >= mGridValues.size())
            {
                qDebug() << "Grid - update - trying to update item " << i << " while grid size is " << mGridValues.size();
                return false;
            }
            mGridValues[i] = val;
            return true;
        }
        bool clear()
        {
            mGridValues.clear();
            return true;
        }
        void setGridValues(QList<U> vals )
        {
            mGridValues.clear();
            mGridValues = vals;
        }
    private:
        QList<U> mGridValues;

};

typedef struct ImgData
{
    QString mUrlJpeg = "";
    QString mUrlFits = "";
    QString mUrlOverlay = "";
    QString mUrlThumbnail = "";
    double min[3] = {0}, max[3] = {0};  // Minimum and Maximum R, G, B pixel values in the image
    double mean[3] = {0};               // Average R, G, B value of the pixels in the image
    double stddev[3] = {0};             // Standard Deviation of the R, G, B pixel values in the image
    double median[3] = {0};             // Median R, G, B pixel value in the image
    double SNR { 0 };                   // Signal to noise ratio
    uint32_t dataType { 0 };            // FITS image data type (TBYTE, TUSHORT, TULONG, TFLOAT, TLONGLONG, TDOUBLE)
    int bytesPerPixel { 1 };            // Number of bytes used for each pixel, size of datatype above
    int ndim { 2 };                     // Number of dimensions in a fits image
    int64_t size { 0 };                 // Filesize in bytes
    uint32_t samples_per_channel { 0 }; // area of the image in pixels
    uint16_t width { 0 };               // width of the image in pixels
    uint16_t height { 0 };              // height of the image in pixels
    uint8_t channels { 1 };             // Mono Images have 1 channel, RGB has 3 channels
} ImgData;

typedef enum
{
    none, /* no graph, just to handle conversion errors */
    XY, /* Basic X-Y graph */
    DY, /* X data is a timestamp */
    PHD, /* PHD-like graph */
} GraphType;
inline QString GraphTypeToString(GraphType val )
{
    if (val == none) return "none";
    if (val == XY) return "XY";
    if (val == DY) return "DY";
    if (val == PHD) return "PHD";
    qDebug() << "GraphTypeToString Cant convert type - return 'none' ";
    return "none";
}
inline GraphType StringToGraphType(QString val)
{
    if (val == "none") return none;
    if (val == "XY") return XY;
    if (val == "DY") return DY;
    if (val == "PHD") return PHD;
    qDebug() << "StringToGraphType Cant convert type " << val << " - return 'none' ";
    return none;

}
typedef struct GraphDefs
{
    GraphType type = none;
    QVariantMap params = QVariantMap();
} GraphDefs;


}

#endif


