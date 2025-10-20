/**
 * @file elementimg.h
 * @brief Image element implementation for astronomical image display
 * @author OST Development Team
 * @version 1.0
 *
 * This file defines ElementImg (image display element with statistics) and
 * ValueImg (image data container for grid storage).
 */

#ifndef ELEMENTIMG_h
#define ELEMENTIMG_h

#include <elementtemplate.h>

namespace  OST
{

/**
 * @class ElementImg
 * @brief Image element for displaying astronomical images with metadata
 *
 * ElementImg is a specialized element type for image display and management.
 * It stores ImgData structures containing image file paths and metadata.
 * Typically used for:
 * - Camera frame preview (light, dark, flat, bias)
 * - Focus frame display
 * - Guider frame display
 * - Plate solving results
 * - Image analysis output
 *
 * The element supports optional statistics display (mean, stddev, min, max, HFR)
 * controlled via the mShowStats flag.
 *
 * @par Basic Usage (camera preview):
 * @code
 * auto preview = new ElementImg("Preview", "10", "Camera frame preview");
 * preview->setShowStats(true);  // Show image statistics
 *
 * ImgData imgData;
 * imgData.url = "/webroot/images/frame001.fits";
 * imgData.mimeType = "image/fits";
 * preview->setValue(imgData, false);
 *
 * property->addElt("preview", preview);
 * @endcode
 *
 * @par Focus Frame Display:
 * @code
 * auto focusFrame = new ElementImg("Focus Frame", "20", "Current focus frame");
 * focusFrame->setShowStats(true);  // Show HFR and other stats
 *
 * ImgData focusData;
 * focusData.url = "/webroot/focus/focus_step_5.jpg";
 * focusData.mimeType = "image/jpeg";
 * focusFrame->setValue(focusData, true);
 * @endcode
 *
 * @par Without Statistics:
 * @code
 * auto thumbnail = new ElementImg("Thumbnail", "30", "Image thumbnail");
 * thumbnail->setShowStats(false);  // Don't show stats, just image
 * @endcode
 *
 * @note ImgData structure is defined in common.h
 * @note Images are served via webroot and accessed by URL path
 * @note Frontend displays images with optional overlay of statistics
 *
 * @see ImgData
 * @see ElementVideo
 * @see ElementTemplateNotNumeric
 */
class ElementImg: public ElementTemplateNotNumeric<ImgData>
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
         * @brief Construct a new ElementImg object
         * @param label Display label for frontend
         * @param order Sort order within property (e.g., "10", "20")
         * @param hint Tooltip/help text for frontend
         */
        ElementImg(const QString &label, const QString &order, const QString &hint);

        /**
         * @brief Destroy the ElementImg object
         */
        ~ElementImg();

        /**
         * @brief Get element type as string
         * @return "img"
         */
        QString getType() override;

        /**
         * @brief Check if statistics display is enabled
         * @return true if statistics shown, false otherwise
         *
         * When true, frontend displays image statistics overlay
         * (mean, stddev, min, max, HFR, etc.)
         */
        bool getShowStats(void);

        /**
         * @brief Enable or disable statistics display
         * @param b true to show statistics, false to hide
         *
         * Controls whether frontend displays image statistics overlay.
         * Useful to disable for simple thumbnails or when stats are irrelevant.
         *
         * @par Example:
         * @code
         * preview->setShowStats(true);   // Show stats (default)
         * thumbnail->setShowStats(false); // Hide stats for simple display
         * @endcode
         */
        void setShowStats(bool b);

    private:
        bool mShowStats = true;  /*!< Whether to display image statistics overlay in frontend */
};

/**
 * @class ValueImg
 * @brief Image data container for grid storage
 *
 * ValueImg is the grid value copy for ElementImg. It stores ImgData structures
 * and provides methods to synchronize with its parent ElementImg.
 *
 * Useful for storing image sequences in grids (e.g., focus run frames,
 * guiding log with frame references).
 *
 * @par Usage (internal to PropertyMulti):
 * @code
 * // PropertyMulti::push() creates values from elements
 * auto valueImg = new ValueImg(previewElement);
 * valueImg->updateValue();  // Copy element->value() to valueImg->value
 * gridLine["preview"] = valueImg;
 *
 * // PropertyMulti::fetchLine() restores elements from values
 * valueImg->updateElement(true);  // Copy valueImg->value to element->value()
 * @endcode
 *
 * @see ValueTemplate
 * @see ElementImg
 * @see ImgData
 * @see PropertyMulti::push()
 */
class ValueImg: public ValueTemplate<ImgData>
{
        Q_OBJECT

    public:
        /**
         * @brief Accept a visitor for the Visitor pattern
         * @param pVisitor Pointer to visitor object
         */
        void accept(ValueVisitor* pVisitor)  override;

        /**
         * @brief Construct a new ValueImg object
         * @param element Pointer to parent ElementImg
         */
        ValueImg(ElementBase* element): ValueTemplate<ImgData>(element) {}

        /**
         * @brief Destroy the ValueImg object
         */
        ~ValueImg() {}

        /**
         * @brief Update value from parent element
         *
         * Copies the parent element's current image data into this value object.
         */
        void updateValue() override;

        /**
         * @brief Update parent element from value
         * @param emitEvent Whether to emit valueSet signal
         *
         * Copies this image data back into the parent element.
         */
        void updateElement(const bool &emitEvent) override;

};
}
#endif
