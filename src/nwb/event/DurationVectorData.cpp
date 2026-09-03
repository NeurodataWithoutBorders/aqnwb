#include "DurationVectorData.hpp"

#include "Utils.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(DurationVectorData)

// Constructor
DurationVectorData::DurationVectorData(const std::string& path,
                                       std::shared_ptr<AQNWB::IO::BaseIO> io)
    : VectorData(path, io)
{
}

// Initialize the object
Status DurationVectorData::initialize(const AQNWB::IO::ArrayDataSetConfig& data,
                                      const std::string& description,
                                      std::optional<float> resolution)
{
  Status initStatus = Status::Success;

  // Retrieve the IO object
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr
        << "IO object has been deleted. Can't initialize DurationVectorData: "
        << m_path << std::endl;
    return Status::Failure;
  }

  // Call parent initialize method, which initializes the description attribute
  // and the data dataset.
  Status parentInitStatus = VectorData::initialize(data, description);
  initStatus = initStatus && parentInitStatus;

  // Initialize attributes, datasets, and groups
  // Initialize unit attribute with fixed value const std::string& "seconds"
  const std::string& unit = "seconds";
  Status unitStatus = ioPtr->createAttribute(unit, m_path, "unit");
  // Initialize resolution attribute
  Status resolutionStatus = Status::Success;
  if (resolution.has_value()) {
    float resValue = resolution.value();
    resolutionStatus = ioPtr->createAttribute(
        AQNWB::IO::BaseDataType::F32, &resValue, m_path, "resolution");
  }

  // Update the status to reflect the success or failure of the initialization
  initStatus = initStatus && parentInitStatus && unitStatus && resolutionStatus;
  // Return the final status of the initialization process
  return initStatus;
}

Status DurationVectorData::DataSpec::initialize(Data& data) const
{
  auto* durationData = dynamic_cast<DurationVectorData*>(&data);
  if (!durationData) {
    std::cerr << "DurationVectorData::DataSpec::initialize received "
                 "incompatible Data object"
              << std::endl;
    return Status::Failure;
  }
  return durationData->initialize(
      static_cast<const AQNWB::IO::ArrayDataSetConfig&>(*this),
      description,
      resolution);
}

std::shared_ptr<DurationVectorData::DataSpec>
DurationVectorData::createDataSpec(
    const std::string& name,
    const AQNWB::IO::ArrayDataSetConfig& dataConfig,
    const std::string& description,
    std::optional<float> resolution)
{
  return std::make_shared<DataSpec>(name, dataConfig, description, resolution);
}
