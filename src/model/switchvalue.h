#ifndef OST_SWITCHVALUE_H
#define OST_SWITCHVALUE_H

#include <string>

#include "model/valuebase.h"

/*!
 * Indi-like switch
 */
class SwitchValue : public ValueBase {

public:
    /*!
     * constructor
     *
     * @param name value's name
     * @param label value's display label
     * @param switchState switch state
     */
    SwitchValue(const std::string& name, const std::string& label, const bool& switchState);
    ~SwitchValue() override = default;

    /*!
     * Get switch state
     *
     * @return the switch state
     */
    [[nodiscard]] inline const bool& switchState() const {return _switchState;}

private:
    bool _switchState;

};


#endif //OST_SWITCHVALUE_H
