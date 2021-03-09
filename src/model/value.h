#ifndef OST_VALUE_H
#define OST_VALUE_H

#include <QString>

/*!
 * Abstract base class for all our INDI related values
*/
class Value {

protected:
    /*!
     * constructor
     *
     * @param name: unique name of this value
     * @param label: display label for this value
     * @param hint: hint for this value
     */
    Value(const QString& name, const QString& label, const QString& hint)
    : _name(name), _label(label), _hint(hint) {}

    virtual ~Value() = default;

public:
    /*!
     * Get value's name
     *
     * @return this value's name
     */
    [[nodiscard]] inline const QString& name() const {return _name;}

    /*!
     * Get value's label
     *
     * @return this value's label
     */
    [[nodiscard]] inline const QString& label() const {return _label;}

    /*!
     * Get hint
     *
     * @return this value's hint
     */
    [[nodiscard]] inline const QString& hint() const {return _hint;}

protected:
    QString _name;
    QString _label;
    QString _hint;

};


#endif //OST_VALUE_H
