#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{
// Register the base Data type (with default std::any) for use with
// RegisteredType::create
template<>
REGISTER_SUBCLASS_IMPL(Data<std::any>)

// Explicitly instantiate for all common data types
template class Data<std::any>;
template class Data<uint8_t>;
template class Data<uint16_t>;
template class Data<uint32_t>;
template class Data<uint64_t>;
template class Data<int8_t>;
template class Data<int16_t>;
template class Data<int32_t>;
template class Data<int64_t>;
template class Data<float>;
template class Data<double>;
template class Data<std::string>;
}  // namespace AQNWB::NWB
