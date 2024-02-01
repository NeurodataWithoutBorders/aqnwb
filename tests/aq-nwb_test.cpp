#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "aq-nwb" ? 0 : 1;
}
