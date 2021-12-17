#ifndef OST_LIGHTVALUE_H
#define OST_LIGHTVALUE_H

#include "model/value.h"

class LightValue : public Value {

public:
    /*!
     * constructor
     *
     * @param name value's name
     * @param label value's display label
     * @param state switch state
     */
    LightValue(const QString& name, const QString& label, const QString &hint,const int& state )
            : Value(name, label, hint), _lightState(state) {}

    ~LightValue() override = default;

    /*!
     * Get switch state
     *
     * @return the switch state
     */
    [[nodiscard]] inline const int& lightState() const {return _lightState;}
    void setState(bool val)  { _lightState=val; }

private:
    int _lightState;

};

#endif //OST_LIGHTVALUE_H
