/**
 * @file common.h
 * @brief Common enumerations, structures, and types for OST element system
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines all common types, enumerations, and data structures used
 * throughout the OST element and property system. Includes:
 * - State and status enumerations (State, MsgLevel, SliderRule, SwitchsRule)
 * - Data structures for specialized elements (ImgData, VideoData, MsgData, PrgData)
 * - Graph configuration types (GraphType, GraphDefs)
 * - Conversion utility functions
 */

#ifndef COMMON_VALUE_h_
#define COMMON_VALUE_h_

#include <QtCore>

namespace  OST
{

/**
 * @enum State
 * @brief Runtime state enumeration for properties, elements, and operations
 *
 * State represents the current status of operations, properties, or elements.
 * Used by PropertyBase::state() and ElementLight for visual status indicators.
 *
 * Frontend typically displays these states as colored indicators:
 * - Idle: Gray (inactive, not running)
 * - Ok: Green (success, ready, completed)
 * - Busy: Yellow/Orange (working, in progress)
 * - Error: Red (failure, problem detected)
 *
 * Similar to INDI protocol IPState enumeration.
 *
 * @see PropertyBase::state()
 * @see ElementLight
 */
typedef enum
{
    Idle = 0, /*!< Inactive or waiting state */
    Ok,       /*!< Success or ready state */
    Busy,     /*!< Working or in-progress state */
    Error     /*!< Failure or error state */
} State;

/**
 * @brief Convert integer to State enum
 * @param val Integer value (0-3)
 * @return Corresponding State value
 *
 * Conversion table: 0=Idle, 1=Ok, 2=Busy, 3=Error
 * Invalid values default to Idle with debug warning.
 */
inline State IntToState(int val )
{
    if (val == 0) return Idle;
    if (val == 1) return Ok;
    if (val == 2) return Busy;
    if (val == 3) return Error;
    qDebug() << "Cant convert " << val << " to OST::State (0-3) - defaults to Idle";
    return Idle;
}

/**
 * @brief Convert State enum to integer
 * @param val State value
 * @return Corresponding integer (0-3)
 *
 * Used for serialization and database storage.
 */
inline int StateToInt(State val )
{
    if (val == Idle) return 0;
    if (val == Ok) return 1;
    if (val == Busy) return 2;
    if (val == Error) return 3;
    qDebug() << "StateToInt Cant convert state - return State::Error";
    return 3;
}

/**
 * @enum SwitchsRule
 * @brief Boolean element coordination rule for radio/checkbox behavior
 *
 * Defines how multiple boolean elements in a property interact with each other.
 * Used by PropertyMulti::setRule() to control boolean element behavior.
 *
 * @see PropertyMulti::setRule()
 * @see ElementBool
 */
typedef enum
{
    OneOfMany = 0,  /*!< Exactly one must be true (radio buttons) */
    AtMostOne,      /*!< Zero or one can be true (optional radio) */
    Any             /*!< No constraint (independent checkboxes) */
} SwitchsRule;

/**
 * @brief Convert integer to SwitchsRule enum
 * @param val Integer value (0-2)
 * @return Corresponding SwitchsRule value
 *
 * Conversion table: 0=OneOfMany, 1=AtMostOne, 2=Any
 * Invalid values default to Any with debug warning.
 */
inline SwitchsRule IntToRule(int val )
{
    if (val == 0) return OneOfMany;
    if (val == 1) return AtMostOne;
    if (val == 2) return Any;
    qDebug() << "Cant convert " << val << " to OST::SwitchsRule (0-2) - defaults to any";
    return Any;
}

/**
 * @enum SliderRule
 * @brief Slider display mode for numeric elements
 *
 * Controls whether and how numeric elements display slider UI in frontend.
 * Used by ElementTemplateNumeric::setSlider().
 *
 * @see ElementTemplateNumeric::setSlider()
 * @see ElementInt
 * @see ElementFloat
 */
typedef enum
{
    SliderNone = 0,      /*!< No slider, use spinbox/textfield only */
    SliderOnly,          /*!< Slider only, no numeric input */
    SliderAndValue       /*!< Slider with adjacent numeric input */
} SliderRule;

/**
 * @brief Convert integer to SliderRule enum
 * @param val Integer value (0-2)
 * @return Corresponding SliderRule value
 *
 * Conversion table: 0=SliderNone, 1=SliderOnly, 2=SliderAndValue
 * Invalid values default to SliderNone with debug warning.
 */
inline SliderRule IntToSlider(int val )
{
    if (val == 0) return SliderNone;
    if (val == 1) return SliderOnly;
    if (val == 2) return SliderAndValue;
    qDebug() << "Cant convert " << val << " to OST::SliderRule (0-2) - defaults to SliderNone";
    return SliderNone;
}

/**
 * @enum MsgLevel
 * @brief Message severity level enumeration
 *
 * Defines severity levels for log messages and notifications.
 * Used by ElementMessage and message signal handlers.
 *
 * Frontend displays with color coding:
 * - Info: Normal/white text
 * - Warn: Yellow/orange text
 * - Err: Red text
 *
 * @see ElementMessage
 * @see MsgData
 * @see PropertyBase::sendInfo()
 * @see PropertyBase::sendWarning()
 * @see PropertyBase::sendError()
 */
typedef enum
{
    Info = 0,  /*!< Informational message (normal priority) */
    Warn,      /*!< Warning message (non-critical issue) */
    Err        /*!< Error message (critical problem) */
} MsgLevel;

/**
 * @struct ModuleStatus
 * @brief Module status information structure
 *
 * Contains status information for module-level state tracking and communication.
 * Used in module event exchanges and status queries.
 *
 * @see Basemodule
 */
typedef struct ModuleStatus
{
    State state = Idle;      /*!< Current module state */
    QDateTime ts;            /*!< Timestamp of status update */
    QString message;         /*!< Status message text */
} ModuleStatus;

/**
 * @brief Convert integer to MsgLevel enum
 * @param val Integer value (0-2)
 * @return Corresponding MsgLevel value
 *
 * Conversion table: 0=Info, 1=Warn, 2=Err
 * Invalid values default to Info with debug warning.
 */
inline MsgLevel IntToMsgLevel(int val )
{
    if (val == 0) return Info;
    if (val == 1) return Warn;
    if (val == 2) return Err;
    qDebug() << "Cant convert " << val << " to OST::MsgLevel (0-2) - defaults to Info";
    return Info;
}

/**
 * @struct MsgData
 * @brief Message data structure for ElementMessage
 *
 * Contains all information for a timestamped log message.
 * Used by ElementMessage to display messages with severity levels.
 *
 * @see ElementMessage
 * @see MsgLevel
 */
typedef struct MsgData
{
    MsgLevel level = Info;   /*!< Message severity level */
    QDateTime ts;            /*!< Message timestamp */
    QString message;         /*!< Message text content */
} MsgData;

/**
 * @struct PrgData
 * @brief Progress data structure for ElementPrg
 *
 * Contains progress information for progress bars and activity indicators.
 * Used by ElementPrg to display operation progress with dynamic labels.
 *
 * @see ElementPrg
 * @see PrgType
 */
typedef struct PrgData
{
    QString dynlabel = "";   /*!< Dynamic status label text */
    double  value = 0;       /*!< Progress percentage (0.0-100.0) */
} PrgData;

/**
 * @struct ImgData
 * @brief Image data structure for ElementImg
 *
 * Comprehensive image metadata structure for astronomical images.
 * Contains URLs for different formats, statistics, solver data, and histograms.
 * Used by ElementImg to display and analyze astronomical images (FITS, JPEG, etc.).
 *
 * Statistics are provided per channel (R, G, B for RGB images, single channel for mono).
 *
 * @see ElementImg
 */
typedef struct ImgData
{
    QString mUrlJpeg = "";              /*!< URL path to JPEG preview version */
    QString mUrlFits = "";              /*!< URL path to FITS original file */
    QString mUrlOverlay = "";           /*!< URL path to overlay image (annotations, stars, etc.) */
    QString mUrlThumbnail = "";         /*!< URL path to thumbnail version */
    QVector<QString> mAlternates;       /*!< URLs to alternate versions/formats */
    double min[3] = {0}, max[3] = {0};  /*!< Minimum and maximum R,G,B pixel values */
    double mean[3] = {0};               /*!< Average R,G,B pixel values */
    double stddev[3] = {0};             /*!< Standard deviation of R,G,B pixel values */
    double median[3] = {0};             /*!< Median R,G,B pixel values */
    double SNR { 0 };                   /*!< Signal to noise ratio */
    uint32_t dataType { 0 };            /*!< FITS data type (TBYTE, TUSHORT, TULONG, TFLOAT, TLONGLONG, TDOUBLE) */
    int bytesPerPixel { 1 };            /*!< Bytes per pixel (size of dataType) */
    int ndim { 2 };                     /*!< Number of dimensions (typically 2 for images) */
    int64_t size { 0 };                 /*!< File size in bytes */
    uint32_t samples_per_channel { 0 }; /*!< Total pixels per channel (width × height) */
    uint16_t width { 0 };               /*!< Image width in pixels */
    uint16_t height { 0 };              /*!< Image height in pixels */
    uint8_t channels { 1 };             /*!< Number of color channels (1=mono, 3=RGB) */
    double HFRavg{ 0 };                 /*!< Average Half-Flux Radius (focus quality metric) */
    double starsCount{ 0 };             /*!< Number of detected stars */
    bool isSolved = false;              /*!< Whether plate solving succeeded */
    double solverRA{ 0 };               /*!< Solved Right Ascension (degrees) */
    double solverDE{ 0 };               /*!< Solved Declination (degrees) */
    QVector<double> histogram[3];       /*!< Histogram data for R,G,B channels */
} ImgData;

/**
 * @brief Convert QVariantMap to ImgData structure
 * @param m QVariantMap containing image data fields
 * @return ImgData structure (partially populated)
 *
 * @note Currently only extracts urljpeg field. Full implementation would
 *       extract all fields from map.
 */
inline ImgData QVariantMapToImgData(QVariantMap m)
{
    ImgData i;
    if (m.contains("urljpeg")) i.mUrlJpeg = m["urljpeg"].toString();
    return i;
}

/**
 * @struct VideoData
 * @brief Video data structure for ElementVideo
 *
 * Simple video metadata structure containing URL path to video file or stream.
 * Used by ElementVideo for video stream display and playback.
 *
 * @see ElementVideo
 */
typedef struct VideoData
{
    QString url = "";  /*!< URL path to video file or stream (e.g., MJPEG, MP4, WebM) */
} VideoData;

/**
 * @brief Convert VideoData structure to QVariantMap
 * @param d VideoData structure
 * @return QVariantMap containing video data fields
 *
 * Used for serialization to JSON/WebSocket.
 */
inline QVariantMap VideoDataToVariantMap(VideoData d)
{
    QVariantMap m;
    m["url"] = d.url;
    return m;
}

/**
 * @brief Convert QVariantMap to VideoData structure
 * @param m QVariantMap containing video data fields
 * @return VideoData structure
 *
 * Used for deserialization from JSON/WebSocket.
 */
inline VideoData VariantMapToVideoData(QVariantMap m)
{
    VideoData v;
    if (m.contains("url")) v.url = m["url"].toString();
    return v;
}

/**
 * @enum GraphType
 * @brief Graph visualization type enumeration
 *
 * Defines the type of graph/chart to display for grid data visualization.
 * Used by PropertyMulti::setGraphDefs() to configure chart rendering in frontend.
 *
 * @see PropertyMulti::setGraphDefs()
 * @see GraphDefs
 */
typedef enum
{
    none, /*!< No graph (default, or conversion error) */
    XY,   /*!< Basic X-Y scatter/line graph */
    DY,   /*!< Date/time-based graph (X axis is timestamps) */
    PHD,  /*!< PHD2-style guiding graph (RA/Dec drift plot) */
    SXY,  /*!< X-Y graph with multiple series (mixed data sets) */
    SDY,  /*!< Date/time graph with multiple series */
} GraphType;

/**
 * @brief Convert GraphType enum to string
 * @param val GraphType value
 * @return String representation ("XY", "DY", "PHD", "SXY", "SDY", "none")
 *
 * Used for serialization to JSON/frontend.
 */
inline QString GraphTypeToString(GraphType val )
{
    if (val == none) return "none";
    if (val == XY) return "XY";
    if (val == DY) return "DY";
    if (val == PHD) return "PHD";
    if (val == SXY) return "SXY";
    if (val == SDY) return "SDY";
    qDebug() << "GraphTypeToString Cant convert type - return 'none' ";
    return "none";
}

/**
 * @brief Convert string to GraphType enum
 * @param val String representation
 * @return Corresponding GraphType value
 *
 * Used for deserialization from JSON/configuration.
 * Invalid strings return 'none' with debug warning.
 */
inline GraphType StringToGraphType(QString val)
{
    if (val == "none") return none;
    if (val == "XY") return XY;
    if (val == "DY") return DY;
    if (val == "PHD") return PHD;
    if (val == "SXY") return SXY;
    if (val == "SDY") return SDY;
    qDebug() << "StringToGraphType Cant convert type " << val << " - return 'none' ";
    return none;

}

/**
 * @struct GraphDefs
 * @brief Graph configuration structure
 *
 * Defines graph visualization settings for property grids.
 * Used by PropertyMulti to configure Chart.js graphs in frontend.
 *
 * The params map contains graph-specific configuration:
 * - "X": Column key for X axis data
 * - "Y": Column key for Y axis data (or comma-separated keys for multiple series)
 * - "title": Graph title
 * - Other chart.js configuration options
 *
 * @par Example:
 * @code
 * GraphDefs defs;
 * defs.type = GraphType::XY;
 * defs.params["X"] = "focpos";
 * defs.params["Y"] = "loopHFRavg";
 * defs.params["title"] = "Focus Curve";
 * property->setGraphDefs(defs);
 * @endcode
 *
 * @see PropertyMulti::setGraphDefs()
 * @see GraphType
 */
typedef struct GraphDefs
{
    GraphType type = none;              /*!< Graph visualization type */
    QVariantMap params = QVariantMap(); /*!< Graph configuration parameters */
} GraphDefs;

/**
 * @enum PrgType
 * @brief Progress indicator display type
 *
 * Defines how progress elements are displayed in frontend.
 * Used by ElementPrg::setPrgType().
 *
 * @see ElementPrg
 * @see PrgData
 */
typedef enum
{
    bar,      /*!< Progress bar (determinate, shows percentage 0-100) */
    spinner   /*!< Spinner/activity indicator (indeterminate, no percentage) */
} PrgType;

}

#endif


