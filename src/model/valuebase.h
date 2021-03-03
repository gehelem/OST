#ifndef OST_VALUEBASE_H
#define OST_VALUEBASE_H

#include <string>

/*!
 * Abstract base class for all our INDI related values
*/
class ValueBase {

protected:
    /*!
     * constructor
     *
     * @param name: unique name of this value
     * @param label: display label for this value
     * @param hint: hint for this value
     */
    ValueBase(const std::string& name, const std::string& label,const std::string& hint);
    virtual ~ValueBase() = default;

public:
    /*!
     * Get value's name
     *
     * @return this value's name
     */
    [[nodiscard]] inline const std::string& name() const {return _name;}

    /*!
     * Get value's label
     *
     * @return this value's label
     */
    [[nodiscard]] inline const std::string& label() const {return _label;}

    /*!
     * Get hint
     *
     * @return this value's hint
     */
    [[nodiscard]] inline const std::string& hint() const {return _hint;}

protected:
    std::string _name;
    std::string _label;
    std::string _hint;

};


#endif //OST_VALUEBASE_H
