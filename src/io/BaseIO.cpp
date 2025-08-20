#include "io/BaseIO.hpp"

#include "Utils.hpp"
#include "nwb/RecordingObjects.hpp"

using namespace AQNWB::IO;
using namespace AQNWB;

// BaseDataType

BaseDataType::BaseDataType(BaseDataType::Type t, SizeType s)
    : type(t)
    , typeSize(s)
{
}

BaseDataType BaseDataType::STR(SizeType size)
{
  return BaseDataType(T_STR, size);
}

const BaseDataType BaseDataType::U8 = BaseDataType(T_U8, 1);
const BaseDataType BaseDataType::U16 = BaseDataType(T_U16, 1);
const BaseDataType BaseDataType::U32 = BaseDataType(T_U32, 1);
const BaseDataType BaseDataType::U64 = BaseDataType(T_U64, 1);
const BaseDataType BaseDataType::I8 = BaseDataType(T_I8, 1);
const BaseDataType BaseDataType::I16 = BaseDataType(T_I16, 1);
const BaseDataType BaseDataType::I32 = BaseDataType(T_I32, 1);
const BaseDataType BaseDataType::I64 = BaseDataType(T_I64, 1);
const BaseDataType BaseDataType::F32 = BaseDataType(T_F32, 1);
const BaseDataType BaseDataType::F64 = BaseDataType(T_F64, 1);
const BaseDataType BaseDataType::DSTR = BaseDataType(T_STR, DEFAULT_STR_SIZE);

// ArrayDataSetConfig
ArrayDataSetConfig::ArrayDataSetConfig(const BaseDataType& type,
                                       const SizeArray& shape,
                                       const SizeArray& chunking)
    : m_type(type)
    , m_shape(shape)
    , m_chunking(chunking)
{
}

// BaseIO

BaseIO::BaseIO(const std::string& filename)
    : m_filename(filename)
    , m_readyToOpen(true)
    , m_opened(false)
    , m_recording_containers(std::make_shared<NWB::RecordingObjects>())
{
}

BaseIO::~BaseIO() {}

Status BaseIO::createCommonNWBAttributes(const std::string& path,
                                         const std::string& objectNamespace,
                                         const std::string& neurodataType)
{
  createAttribute(objectNamespace, path, "namespace");
  createAttribute(generateUuid(), path, "object_id");
  if (neurodataType != "")
    createAttribute(neurodataType, path, "neurodata_type");
  return Status::Success;
}

std::unordered_map<std::string, std::string> BaseIO::findTypes(
    const std::string& starting_path,
    const std::unordered_set<std::string>& types,
    SearchMode search_mode,
    bool exclude_starting_path) const
{
  std::unordered_map<std::string, std::string> found_types;

  //
  // Helper function to recursively search for types.
  //
  // This function checks each object's attributes to determine its type and
  // matches it against the given types. If a match is found, it adds the
  // object's path and type to the found_types map.
  //
  // Declared as an internal function to encapsulate the recursive search logic,
  // avoid recursive function call overhead, and reduce the risk of stack
  // overflow.
  //
  // current_path The current path being searched.
  //
  std::function<void(const std::string&)> searchTypes =
      [&](const std::string& current_path)
  {
    // Check if the current object exists as a dataset or group
    if (objectExists(current_path)) {
      // Check if we have a typed object
      if (attributeExists(current_path + "/neurodata_type")
          && attributeExists(current_path + "/namespace"))
      {
        // Read the namespace and neurodata_type attributes
        try {
          auto namespace_attr = DataBlock<std::string>::fromGeneric(
              readAttribute(current_path + "/namespace"));
          auto neurodata_type_attr = DataBlock<std::string>::fromGeneric(
              readAttribute(current_path + "/neurodata_type"));
          // Combine the namespace and neurodata_type attributes
          std::string full_type =
              namespace_attr.data[0] + "::" + neurodata_type_attr.data[0];

          // Check if we should exclude the current path
          bool exclude_start_conditon =
              (exclude_starting_path && (current_path == starting_path));

          // Check if the full type matches any of the given types
          if (types.find(full_type) != types.end() || types.empty()) {
            // Ignore the starting path if exclude_starting_path is true
            if (!exclude_start_conditon) {
              // Add the object's path and type to the found_types map
              found_types[current_path] = full_type;
            }
          }

          // If search_mode is CONTINUE_ON_TYPE, continue searching inside this
          // object or when the current path is the starting path and
          // exclude_starting_path is true
          if (search_mode == SearchMode::CONTINUE_ON_TYPE
              || exclude_start_conditon)
          {
            // Get the list of objects inside the current group to
            // continue the search
            std::vector<std::pair<std::string, StorageObjectType>> objects =
                getStorageObjects(current_path, StorageObjectType::Undefined);
            for (const auto& obj : objects) {
              if (obj.second == StorageObjectType::Group
                  || obj.second == StorageObjectType::Dataset)
              {
                // Recursively continue the search
                searchTypes(AQNWB::mergePaths(current_path, obj.first));
              }
            }
          }
        } catch (...) {
          // Handle any exceptions that occur while reading the attributes
          return;
        }
      }
      // If the object is not a neurodata type then try to continue the search
      else
      {
        // Get the list of objects inside the current group
        std::vector<std::pair<std::string, StorageObjectType>> objects =
            getStorageObjects(current_path, StorageObjectType::Undefined);
        for (const auto& obj : objects) {
          if (obj.second == StorageObjectType::Group
              || obj.second == StorageObjectType::Dataset)
          {
            searchTypes(AQNWB::mergePaths(current_path, obj.first));
          }
        }
      }
    }
  };

  // Start the recursive search from the starting path
  searchTypes(starting_path);

  return found_types;
}

// BaseRecordingData

BaseRecordingData::BaseRecordingData() {}

BaseRecordingData::~BaseRecordingData() {}

Status BaseIO::stopRecording()
{
  // Finalize all recording objects before stopping recording
  if (m_recording_containers) {
    Status finalizeStatus = m_recording_containers->finalize();
    if (finalizeStatus != Status::Success) {
      // Log the error but continue with stopping recording
      std::cerr << "Warning: Failed to finalize some recording objects" << std::endl;
    }
  }
  
  return Status::Success;
}

// Overload that uses the member variable position (works for simple data
// extension)
Status BaseRecordingData::writeDataBlock(const std::vector<SizeType>& dataShape,
                                         const BaseDataType& type,
                                         const void* data)
{
  return writeDataBlock(dataShape, m_position, type, data);
}
