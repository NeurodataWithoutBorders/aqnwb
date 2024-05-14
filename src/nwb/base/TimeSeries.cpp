#include "nwb/base/TimeSeries.hpp"

using namespace AQNWB::NWB;

// TimeSeries

/** Constructor */
TimeSeries::TimeSeries(const std::string& path,
                               std::shared_ptr<BaseIO> io,
                               const std::string& description,
                               const std::string& comments)
    : Container(path, io)
    , description(description)
    , comments(comments)
{
}

/** Destructor */
TimeSeries::~TimeSeries() {}

void TimeSeries::initialize()
{
  Container::initialize();

  io->createCommonNWBAttributes(path, "core", neurodataType, description);
  io->createAttribute(comments, path, "comments");
}

