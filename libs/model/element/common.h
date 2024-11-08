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
    OneOfMany = 0,
    AtMostOne,
    Any
} SwitchsRule;
inline SwitchsRule IntToRule(int val )
{
    if (val == 0) return OneOfMany;
    if (val == 1) return AtMostOne;
    if (val == 2) return Any;
    qDebug() << "Cant convert " << val << " to OST::SwitchsRule (0-2) - defaults to any";
    return Any;
}
typedef enum
{
    SliderNone = 0,
    SliderOnly,
    SliderAndValue
} SliderRule;
inline SliderRule IntToSlider(int val )
{
    if (val == 0) return SliderNone;
    if (val == 1) return SliderOnly;
    if (val == 2) return SliderAndValue;
    qDebug() << "Cant convert " << val << " to OST::SliderRule (0-2) - defaults to SliderNone";
    return SliderNone;
}

typedef enum
{
    Info = 0,
    Warn,
    Err
} MsgLevel;

typedef struct ModuleStatus
{
    State state = Idle;
    QDateTime ts;
    QString message;
} ModuleStatus;

inline MsgLevel IntToMsgLevel(int val )
{
    if (val == 0) return Info;
    if (val == 1) return Warn;
    if (val == 2) return Err;
    qDebug() << "Cant convert " << val << " to OST::MsgLevel (0-2) - defaults to Info";
    return Info;
}

typedef struct MsgData
{
    MsgLevel level = Info;
    QDateTime ts;
    QString message;
} MsgData;

typedef struct PrgData
{
    QString dynlabel = "";
    double  value = 0;
} PrgData;

class DateData
{
    public:
        int year(void)
        {
            return mYear;
        };
        int month(void)
        {
            return mMonth;
        };
        int day(void)
        {
            return mDay;
        };
        bool setYear(int i)
        {
            mYear = i;
            return true;
        }
        bool setMonth(int i)
        {
            if (i > 12 || i < 0) return false;
            mMonth = i;
            return true;
        }
        bool setDay(int i)
        {
            if (i > 31 || i < 0) return false;
            mDay = i;
            return true;
        }
        bool setYMD(int y, int m, int d)
        {
            return setYear(y) && setMonth(m) && setDay(d);
        }
    private:
        int mYear = 0;
        int mMonth = 0;
        int mDay = 0;
};

class TimeData
{
    public:
        int HH(void)
        {
            return hh;
        };
        int MM(void)
        {
            return mm;
        };
        float SS(void)
        {
            return ss;
        };
        bool setHH(int i)
        {
            if (i > 23 || i < 0) return false;
            hh = i;
            return true;
        }
        bool setMM(int i)
        {
            if (i >= 60 || i < 0) return false;
            mm = i;
            return true;
        }
        bool setSS(float s)
        {
            if (s >= 60 || s < 0) return false;
            ss = s;
            return true;
        }
        bool setHMS(int h, int m, float s)
        {
            return setHH(h) && setMM(m) && setSS(s);
        }
    private:
        int hh = 0;
        int mm = 0;
        float ss = 0;
};


typedef struct ImgData
{
    QString mUrlJpeg = "";
    QString mUrlFits = "";
    QString mUrlOverlay = "";
    QString mUrlThumbnail = "";
    QVector<QString> mAlternates;
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
    double HFRavg{ 0 };                 // Average HFR
    double starsCount{ 0 };             // Stars count
    bool isSolved = false;
    double solverRA{ 0 };
    double solverDE{ 0 };
    QVector<double> histogram[3];
} ImgData;

inline ImgData QVariantMapToImgData(QVariantMap m)
{
    ImgData i;
    if (m.contains("urljpeg")) i.mUrlJpeg = m["urljpeg"].toString();
    return i;
}
typedef struct VideoData
{
    QString url = "";
} VideoData;

inline QVariantMap VideoDataToVariantMap(VideoData d)
{
    QVariantMap m;
    m["url"] = d.url;
    return m;
}

inline VideoData VariantMapToVideoData(QVariantMap m)
{
    VideoData v;
    if (m.contains("url")) v.url = m["url"].toString();
    return v;
}
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

typedef enum
{
    bar,
    spinner
} PrgType;

}

#endif


