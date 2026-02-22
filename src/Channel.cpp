#include <string>

#include "Channel.hpp"

using namespace AQNWB;

Channel::Channel(const std::string& name,
                 const std::string& groupName,
                 const SizeType groupIndex,
                 const SizeType localIndex,
                 const SizeType globalIndex,
                 const float conversion,
                 const float samplingRate,
                 const float bitVolts,
                 const std::array<float, 3> position,
                 const std::string& comments)
    : m_name(name)
    , m_groupName(groupName)
    , m_groupIndex(groupIndex)
    , m_localIndex(localIndex)
    , m_globalIndex(globalIndex)
    , m_conversion(conversion)
    , m_samplingRate(samplingRate)
    , m_bitVolts(bitVolts)
    , m_position(position)
    , m_comments(comments)
{
}

Channel::~Channel() {}
