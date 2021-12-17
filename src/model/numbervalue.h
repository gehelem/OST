#ifndef OST_NUMBERVALUE_H
#define OST_NUMBERVALUE_H

#include <string>

#include "model/value.h"

/*!
 * INDI-like numerical value
 */
class NumberValue : public Value {

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
    NumberValue(const QString& name, const QString& label, const QString &hint, const double& value,
                const QString& format, const double& min, const double& max, const double& step)
    : Value(name, label, hint), _value(value), _format(format), _min(min), _max(max), _step(step) {}

    ~NumberValue() override = default;

    [[nodiscard]] inline double getValue() const { return _value; }
    [[nodiscard]] inline const QString &getFormat() const { return _format; }
    [[nodiscard]] inline double getMin() const { return _min; }
    [[nodiscard]] inline double getMax() const { return _max; }
    [[nodiscard]] inline double getStep() const { return _step; }
    void setValue(double val)  { _value=val; }

private:
    double _value;
    QString _format;
    double _min;
    double _max;
    double _step;
};


#endif //OST_NUMBERVALUE_H
