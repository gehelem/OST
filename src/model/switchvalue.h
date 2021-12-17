#ifndef OST_SWITCHVALUE_H
#define OST_SWITCHVALUE_H

#include <string>

#include "model/value.h"

/*!
 * Indi-like switch
 */
class SwitchValue : public Value {

public:
    /*!
     * constructor
     *
     * @param name value's name
     * @param label value's display label
     * @param switchState switch state
     */
    SwitchValue(const QString& name, const QString& label, const QString &hint,const bool& switchState )
    : Value(name, label, hint), _switchState(switchState) {}

    ~SwitchValue() override = default;

    /*!
     * Get switch state
     *
     * @return the switch state
     */
    [[nodiscard]] inline const bool& switchState() const {return _switchState;}
    void setState(bool val)  { _switchState=val; }

private:
    bool _switchState;

};


#endif //OST_SWITCHVALUE_H
