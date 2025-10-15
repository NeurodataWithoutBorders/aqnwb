#pragma once

// Common STL includes
#include <memory>
#include <string>
#include <vector>
#include <optional>
// Base AqNWB includes for IO and RegisteredType
#include "nwb/RegisteredType.hpp"
#include "io/ReadIO.hpp"
#include "io/BaseIO.hpp"
// Include for parent type
#include "nwb/hdmf/base/Data.hpp"
// Include for the namespace schema header
#include "spec/core.hpp"

namespace AQNWB::NWB
{

/**
 * @brief An abstract data type for a dataset.
 */
class NWBData : public AQNWB::NWB::Data
{
public:
    /**
     * @brief Constructor
     * @param path Path to the object in the file
     * @param io IO object for reading/writing
     */
    NWBData(
        const std::string& path,
        std::shared_ptr<AQNWB::IO::BaseIO> io);
    
    
    /**
     * @brief Initialize the object
     * @param dataConfig The configuration for the dataset
     * @return Status::Success if successful, otherwise Status::Failure.
     */
     Status initialize(const AQNWB::IO::ArrayDataSetConfig& dataConfig);

    REGISTER_SUBCLASS(
        NWBData,
        AQNWB::SPEC::CORE::namespaceName)
    
};


/**
 * @brief A typed data container for a dataset.
 *
 * This typed variant of NWBData allows for the specification of the data type
 * at compile time, enabling type-safe access to the data. This is useful for
 * data read to simplify access when the type is known. While we can use
 * the typed version also for data write, in most case the base version
 * of NWBData is sufficient. NOTE: Only NWBData is registered with the
 * RegisteredType class registry. The NWBDataTyped class is not registered
 * since the DTYPE information is not available as part of the neurodata_type
 * attribute in the NWB file.
 *
 * @tparam DTYPE The data type of the data managed by DataTyped
 */
template<typename DTYPE = std::any>
class NWBDataTyped : public NWBData
{
public:
  /**
   * @brief Constructor.
   *
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  NWBDataTyped(const std::string& path, std::shared_ptr<IO::BaseIO> io)
      : NWBData(path, io)
  {
  }

  /**
   * @brief Virtual destructor.
   */
  virtual ~NWBDataTyped() override {}

  /**
   *  \brief Create a DataTyped object from a Data object
   *
   *  This function is useful when the type of the data is known and we want
   *  read data in a typed manner where the type is stored in the DTYPE template
   *  parameter. NOTE: The original Data object retains ownership of the
   *  Data.m_dataset recording dataset object if it was initialized, i.e.,
   *  the returned DataTyped object will have a nullptr m_dataset.
   *
   *  @param data The Data object to convert
   *  @return A DataTyped object with the same path and IO object as the input
   */
  static std::shared_ptr<NWBDataTyped<DTYPE>> fromNWBData(const Data& data)
  {
    return std::make_shared<NWBDataTyped<DTYPE>>(data.getPath(), data.getIO());
  }

  // Define the data fields to expose for lazy read access
  DEFINE_DATASET_FIELD(readData, recordData, DTYPE, "", The main data)

  using RegisteredType::m_io;
  using RegisteredType::m_path;
};

}  // namespace AQNWB::NWB

