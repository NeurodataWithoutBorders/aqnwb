#include "nwb/event/EventsTable.hpp"

#include "Utils.hpp"
// Includes for referenced types
#include "nwb/event/DurationVectorData.hpp"
#include "nwb/event/TimestampVectorData.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"

using namespace AQNWB::NWB;
using namespace AQNWB::IO;

// Initialize the static registered_ member to trigger registration
REGISTER_SUBCLASS_IMPL(EventsTable)

// Constructor
EventsTable::EventsTable(const std::string& path,
                         std::shared_ptr<AQNWB::IO::BaseIO> io)
    : DynamicTable(path, io)
{
}

// Initialize the object
Status EventsTable::initialize(const std::string& description,
                               const std::string& sourceDescription,
                               const SizeType rowChunkSize)
{
  Status initStatus = Status::Success;

  // Retrieve the IO object
  auto ioPtr = getIO();
  if (ioPtr == nullptr) {
    std::cerr << "IO object has been deleted. Can't initialize EventsTable: "
              << m_path << std::endl;
    return Status::Failure;
  }

  // Call parent initialize method.
  Status parentInitStatus = DynamicTable::initialize(description);
  initStatus = initStatus && parentInitStatus;

  // Initialize attributes, datasets, and groups

  // TODO: Initialize source_description attribute
  // ioPtr->createAttribute(sourceDescription, m_path, "source_description");

  // TODO: Initialize colnames attribute. This attribute is_inheritted=True,
  // is_overridden=False ioPtr->createAttribute(colnames, m_path, "colnames");

  // TODO: Initialize timestamp dataset
  // auto TimestampPath = AQNWB::mergePaths(m_path, "timestamp");
  // create scalar dataset at TimestampPath with default value const
  // std::shared_ptr<CORE::TimestampVectorData>& None

  // TODO: Initialize duration dataset
  // auto DurationPath = AQNWB::mergePaths(m_path, "duration");
  // create scalar dataset at DurationPath with default value const
  // std::shared_ptr<CORE::DurationVectorData>& nullptr

  // TODO: Initialize annotation dataset
  // auto AnnotationPath = AQNWB::mergePaths(m_path, "annotation");
  // create scalar dataset at AnnotationPath with default value const
  // std::shared_ptr<HDMF_COMMON::VectorData>& nullptr

  // TODO: Initialize id dataset. This dataset is_inheritted=True,
  // is_overridden=False auto IdPath = AQNWB::mergePaths(m_path, "id"); create
  // scalar dataset at IdPath with default value const
  // std::shared_ptr<HDMF_COMMON::ElementIdentifiers>& None

  // NOTE: Anonymous dataset of type VectorData passed as parameter
  // paramVectorData. No initialization needed here.

  // TODO: Optional RegisteredType const
  // std::vector<std::shared_ptr<HDMF_COMMON::MeaningsTable>>& passed as
  // parameter meaningsTablesParamMeaningsTable. Usually created after
  // initialize.

  return initStatus;
}
