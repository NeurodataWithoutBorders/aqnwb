#include "nwb/hdmf/base/Data.hpp"

namespace AQNWB::NWB
{

// Explicitly register the Data<std::any> specialization with the type registry.
// This ensures that the most generic specialization is available for dynamic
// creation with RegisteredType::create. The REGISTER_SUBCLASS_IMPL macro
// initializes the static member to trigger the registration.
template<>
REGISTER_SUBCLASS_IMPL(Data<std::any>)

// Explicitly instantiate the Data template for all common data types.
// This ensures that these specializations are generated by the compiler,
// reducing compile times and ensuring availability throughout the program.
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
