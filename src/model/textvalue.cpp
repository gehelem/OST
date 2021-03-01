#include "textvalue.h"

TextValue::TextValue(const std::string &name, const std::string &label, const std::string &text)
: ValueBase(name, label),
  _text(text)
{

}
