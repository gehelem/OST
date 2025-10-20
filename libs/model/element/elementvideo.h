/**
 * @file elementvideo.h
 * @brief Video element implementation for video stream display
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementVideo (video stream/recording element) and
 * ValueVideo (video data container for grid storage).
 */

#ifndef ELEMENTVIDEO_h
#define ELEMENTVIDEO_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementVideo
 * @brief Video element for displaying video streams and recordings
 *
 * ElementVideo is a specialized element type for video stream display.
 * It stores VideoData structures containing video file paths or stream URLs.
 * Typically used for:
 * - Live camera stream preview
 * - All-sky camera video feeds
 * - Guider video streams
 * - Timelapse video playback
 * - Recording/replay of observing sessions
 *
 * @par Basic Usage (live stream):
 * @code
 * auto liveStream = new ElementVideo("Live Stream", "10", "Camera live view");
 *
 * VideoData videoData;
 * videoData.url = "/webroot/stream/camera.mjpg";
 * videoData.mimeType = "video/mjpeg";
 * liveStream->setValue(videoData, false);
 *
 * property->addElt("stream", liveStream);
 * @endcode
 *
 * @par All-sky Timelapse:
 * @code
 * auto timelapse = new ElementVideo("Timelapse", "20", "All-sky timelapse");
 *
 * VideoData timelapseData;
 * timelapseData.url = "/webroot/allsky/timelapse_2025-10-20.mp4";
 * timelapseData.mimeType = "video/mp4";
 * timelapse->setValue(timelapseData, true);
 * @endcode
 *
 * @par Guider Stream:
 * @code
 * auto guiderStream = new ElementVideo("Guider", "30", "Guider camera stream");
 *
 * VideoData guiderData;
 * guiderData.url = "/webroot/guider/stream.webm";
 * guiderData.mimeType = "video/webm";
 * guiderStream->setValue(guiderData, false);
 * @endcode
 *
 * @note VideoData structure is defined in common.h
 * @note Videos are served via webroot and accessed by URL path
 * @note Frontend uses HTML5 video player for playback
 * @note Supports common formats: MP4, WebM, MJPEG streams
 *
 * @see VideoData
 * @see ElementImg
 * @see ElementTemplateNotNumeric
 */
class ElementVideo: public ElementTemplateNotNumeric<VideoData>
{

        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ElementVisitor *pVisitor) override;

        /**
         * @brief Accept a visitor with data parameter
         * @param pVisitor Pointer to visitor object
         * @param data QVariantMap with operation data
         */
        void accept(ElementVisitor *pVisitor, QVariantMap &data) override;

        /**
         * @brief Accept a visitor with action and data parameters
         * @param pVisitor Pointer to visitor object
         * @param action Action string (e.g., "newline", "cleargrid")
         * @param data QVariantMap with operation data
         */
        void accept(ElementVisitor *pVisitor, QString &action, QVariantMap &data) override;

        /**
         * @brief Construct a new ElementVideo object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementVideo(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementVideo object
         */
        ~ElementVideo();

        /**
         * @brief Get element type as string
         * @return "video"
         */
        QString getType() override;

    private:

};

/**
 * @class ValueVideo
 * @brief Video data container for grid storage
 *
 * ValueVideo is the grid value copy for ElementVideo. It stores VideoData structures
 * and provides methods to synchronize with its parent ElementVideo.
 *
 * Useful for storing video sequences in grids (e.g., timelapse archive,
 * recording log with video references).
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueVideo = new ValueVideo(streamElement);
 * valueVideo->updateValue();  // Copy element->value() to valueVideo->value
 * gridLine["stream"] = valueVideo;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueVideo->updateElement(true);  // Copy valueVideo->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementVideo
 * @see VideoData
 * @see PropertyMulti::push()
 */
class ValueVideo: public ValueTemplate<VideoData>
{
        Q_OBJECT

    public:

        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueVideo object
         * @param element Pointer to parent ElementVideo
         */
        ValueVideo(ElementBase* element): ValueTemplate<VideoData>(element) {}

        /**
         * @brief Destroy the ValueVideo object
         */
        ~ValueVideo() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current video data into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this video data back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};
}
#endif
