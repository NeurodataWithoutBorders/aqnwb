#include "aqnwb/BaseIO.hpp"

#include "aqnwb/Utils.hpp"

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

// BaseIO

BaseIO::BaseIO()
    : readyToOpen(true)
    , opened(false)
{
}

BaseIO::~BaseIO() {}

bool BaseIO::isOpen() const
{
  return opened;
}

bool BaseIO::isReadyToOpen() const
{
  return readyToOpen;
}

bool BaseIO::canModifyObjects()
{
  return true;
}

Status BaseIO::createCommonNWBAttributes(const std::string& path,
                                         const std::string& objectNamespace,
                                         const std::string& neurodataType,
                                         const std::string& description)
{
  createAttribute(objectNamespace, path, "namespace");
  createAttribute(generateUuid(), path, "object_id");
  if (neurodataType != "")
    createAttribute(neurodataType, path, "neurodata_type");
  if (description != "")
    createAttribute(description, path, "description");
  return Status::Success;
}

Status BaseIO::createDataAttributes(const std::string& path,
                                    const float& conversion,
                                    const float& resolution,
                                    const std::string& unit)
{
  createAttribute(BaseDataType::F32, &conversion, path + "/data", "conversion");
  createAttribute(BaseDataType::F32, &resolution, path + "/data", "resolution");
  createAttribute(unit, path + "/data", "unit");

  return Status::Success;
}

Status BaseIO::createTimestampsAttributes(const std::string& path)
{
  int interval = 1;
  createAttribute(BaseDataType::I32,
                  static_cast<const void*>(&interval),
                  path + "/timestamps",
                  "interval");
  createAttribute("seconds", path + "/timestamps", "unit");

  return Status::Success;
}

// BaseRecordingData

BaseRecordingData::BaseRecordingData() {}

BaseRecordingData::~BaseRecordingData() {}

// Overload that uses the member variable position (works for simple data
// extension)
Status BaseRecordingData::writeDataBlock(const std::vector<SizeType>& dataShape,
                                         const BaseDataType& type,
                                         const void* data)
{
  return writeDataBlock(dataShape, position, type, data);
}
