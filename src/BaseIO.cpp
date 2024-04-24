#include "BaseIO.hpp"

#include "Utils.hpp"

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
    : readyToOpen(
        true)  // TODO - move the readyToOpen flag to the NWBFile building part
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

Status BaseIO::createCommonNWBAttributes(const std::string& path,
                                         const std::string& objectNamespace,
                                         const std::string& neurodataType,
                                         const std::string& description)
{
  createAttribute(objectNamespace, path, "namespace");
  createAttribute(neurodataType, path, "neurodata_type");
  createAttribute(generateUuid(), path, "object_id");
  if (description != "")
    createAttribute(description, path, "description");
  return Status::Success;
}

// BaseRecordingData

BaseRecordingData::BaseRecordingData() {}

BaseRecordingData::~BaseRecordingData() {}

Status BaseRecordingData::writeDataBlock(const SizeType& xDataSize,
                                         const BaseDataType& type,
                                         const void* data,
                                         const bool isVlenStr)
{
  return writeDataBlock(xDataSize, size[1], type, data, isVlenStr);
}
