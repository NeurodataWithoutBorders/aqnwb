#include <string>

#include "Channel.hpp"

using namespace AQNWB;

Channel::Channel(std::string name,
                 std::string groupName,
                 SizeType localIndex,
                 SizeType globalIndex,
                 float conversion,
                 float samplingRate,
                 float bitVolts,
                 std::vector<float> position)
    : name(name)
    , groupName(groupName)
    , localIndex(localIndex)
    , globalIndex(globalIndex)
    , position(position)
    , conversion(conversion)
    , samplingRate(samplingRate)
    , bitVolts(bitVolts)
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
