#include <string>

#include "aqnwb/aqnwb.hpp"

exported_class::exported_class()
    : m_name {"aqnwb"}
{
}

auto exported_class::name() const -> char const*
{
  return m_name.c_str();
}