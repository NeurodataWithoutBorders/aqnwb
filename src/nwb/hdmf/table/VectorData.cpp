#include "nwb/hdmf/table/VectorData.hpp"

namespace AQNWB::NWB
{
// Register the base VectorData type (with default std::any) for use with
// RegisteredType::create
template<>
REGISTER_SUBCLASS_IMPL(VectorData<std::any>)

// Explicitly instantiate for all common data types
template class VectorData<std::any>;
template class VectorData<uint8_t>;
template class VectorData<uint16_t>;
template class VectorData<uint32_t>;
template class VectorData<uint64_t>;
template class VectorData<int8_t>;
template class VectorData<int16_t>;
template class VectorData<int32_t>;
template class VectorData<int64_t>;
template class VectorData<float>;
template class VectorData<double>;
template class VectorData<std::string>;
}  // namespace AQNWB::NWB
