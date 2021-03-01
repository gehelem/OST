#ifndef OST_TEXTVALUE_H
#define OST_TEXTVALUE_H

#include <string>

#include "model/valuebase.h"

/*!
 * Basic text value
 */
class TextValue : public ValueBase {

public:
    /*!
     * constructor
     *
     * @param name: unique name of this value
     * @param label: diplsay label for this value
     * @param text: text value
     */
    TextValue(const std::string& name, const std::string& label, const std::string& text);

public:
    /*!
     * Get value's text
     *
     * @return this value's text
     */
    [[nodiscard]] inline const std::string& text() const {return _text;}
private:
    std::string _text;
};


#endif //OST_TEXTVALUE_H
