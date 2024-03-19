#include "BaseIO.hpp"


using namespace AQNWBIO;

// BaseDataType

BaseDataType::BaseDataType(BaseDataType::Type t, size_t s)
    : type(t)
    , typeSize(s)
{
}

BaseDataType BaseDataType::STR(size_t size)
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

// BaseRecordingData

BaseRecordingData::BaseRecordingData() {}

BaseRecordingData::~BaseRecordingData() {}

int BaseRecordingData::writeDataBlock(int xDataSize,
                                      BaseDataType type,
                                      const void* data)
{
  return writeDataBlock(xDataSize, size[1], type, data);
}
