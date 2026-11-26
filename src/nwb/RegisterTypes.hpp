#pragma once

namespace AQNWB::NWB
{

/**
 * @brief Register all neurodata_types implemented in AqNWB with the
 * RegisteredType registry.
 *
 * Each RegisteredType class registers itself via a static initializer (see
 * REGISTER_SUBCLASS_IMPL). When linking AqNWB as a static library, the linker
 * only includes the object files of classes that the application uses
 * directly. The classes of all other types are not registered, so
 * RegisteredType::create(path, io) cannot create them when reading a file and
 * returns nullptr.
 *
 * Applications that read files with neurodata_types they do not use directly
 * (e.g., a reader for arbitrary NWB files) should call this function once
 * before reading. Calling it more than once is harmless. The types of
 * disabled optional modules (e.g., AQNWB_ENABLE_OPHYS=OFF) are not
 * registered.
 */
void registerAllTypes();

}  // namespace AQNWB::NWB
