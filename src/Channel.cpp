#include <string>

#include "aqnwb/Channel.hpp"

using namespace AQNWB;

Channel::Channel(const std::string name,
                 const std::string groupName,
                 const SizeType groupIndex,
                 const SizeType localIndex,
                 const SizeType globalIndex,
                 const float conversion,
                 const float samplingRate,
                 const float bitVolts,
                 const std::array<float, 3> position,
                 const std::string comments)
    : name(name)
    , groupName(groupName)
    , groupIndex(groupIndex)
    , localIndex(localIndex)
    , globalIndex(globalIndex)
    , position(position)
    , conversion(conversion)
    , samplingRate(samplingRate)
    , bitVolts(bitVolts)
    , comments(comments)
{
}

Channel::~Channel() {}

float Channel::getConversion() const
{
  return bitVolts / conversion;
}

float Channel::getSamplingRate() const
{
  return samplingRate;
}

float Channel::getBitVolts() const
{
  return bitVolts;
}
