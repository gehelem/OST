#include "switchvalue.h"

SwitchValue::SwitchValue(const std::string &name, const std::string &label, const std::string &hint, const bool &switchState)
: ValueBase(name, label, hint),
  _switchState(switchState)
{

}
