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
     * @param label: diplsay label for this value
     */
    ValueBase(const std::string& name, const std::string& label);
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

protected:
    std::string _name;
    std::string _label;
};


#endif //OST_VALUEBASE_H
