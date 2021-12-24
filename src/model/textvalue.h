#ifndef OST_TEXTVALUE_H
#define OST_TEXTVALUE_H

#include <string>

#include "model/value.h"

/*!
 * Basic text value
 */
class TextValue : public Value {

public:
    /*!
     * constructor
     *
     * @param name: unique name of this value
     * @param label: diplsay label for this value
     * @param text: text value
     */
    TextValue(const QString& name, const QString& label, const QString &hint, const QString& text)
    : Value(name, label, hint), _text(text) {}
    
    ~TextValue() override = default;

    /*!
     * Get value's text
     *
     * @return this value's text
     */
    [[nodiscard]] inline const QString& text() const {return _text;}
    void setText(QString val)  { _text=val; }


private:
    QString _text;
};


#endif //OST_TEXTVALUE_H
