#ifndef OST_NUMBERVALUE_H
#define OST_NUMBERVALUE_H

#include <string>

#include "model/valuebase.h"

/*!
 * INDI-like numerical value
 */
class NumberValue : public ValueBase {

public:
    /*!
     * Constructor
     *
     * @param name value name
     * @param label value label
     * @param value value value
     * @param format value format
     * @param min minimum allowed value
     * @param max maximum allowed value
     * @param step increment step
     */
    NumberValue(const std::string& name, const std::string& label, const std::string &hint,
                const double& value, const std::string& format,
                const double& min, const double& max, const double& step);
    ~NumberValue() override = default;

private:
    double _value;
    std::string _format;
    double _min;
    double _max;
    double _step;
};


#endif //OST_NUMBERVALUE_H
