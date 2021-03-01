#include "switchvalue.h"

SwitchValue::SwitchValue(const std::string &name, const std::string &label, const bool &switchState)
: ValueBase(name, label),
  _switchState(switchState)
{

}
