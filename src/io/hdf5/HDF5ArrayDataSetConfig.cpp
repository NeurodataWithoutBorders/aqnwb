#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"

using namespace AQNWB::IO::HDF5;

// HDF5FilterConfig

HDF5FilterConfig::HDF5FilterConfig(
    H5Z_filter_t in_filter_id, const std::vector<unsigned int>& in_cd_values)
    : filter_id(in_filter_id)
    , cd_values(in_cd_values)
{
}

// Factory methods for common filters

HDF5FilterConfig HDF5FilterConfig::createGzipFilter(unsigned int level)
{
  return HDF5FilterConfig(H5Z_FILTER_DEFLATE, {level});
}

HDF5FilterConfig HDF5FilterConfig::createShuffleFilter()
{
  return HDF5FilterConfig(H5Z_FILTER_SHUFFLE, {});
}

HDF5FilterConfig HDF5FilterConfig::createFletcher32Filter()
{
  return HDF5FilterConfig(H5Z_FILTER_FLETCHER32, {});
}

HDF5FilterConfig HDF5FilterConfig::createNbitFilter()
{
  return HDF5FilterConfig(H5Z_FILTER_NBIT, {});
}

// HDF5ArrayDataSetConfig

HDF5ArrayDataSetConfig::HDF5ArrayDataSetConfig(const BaseDataType& type,
                                               const SizeArray& shape,
                                               const SizeArray& chunking)
    : ArrayDataSetConfig(type, shape, chunking)
{
}

void HDF5ArrayDataSetConfig::addFilter(
    H5Z_filter_t filter_id, const std::vector<unsigned int>& cd_values)
{
  m_filters.emplace_back(filter_id, cd_values);
}

void HDF5ArrayDataSetConfig::addFilter(const HDF5FilterConfig& filter)
{
  m_filters.push_back(filter);
}

void HDF5ArrayDataSetConfig::addFilters(
    const std::vector<HDF5FilterConfig>& filters)
{
  m_filters.insert(m_filters.end(), filters.begin(), filters.end());
}

const std::vector<HDF5FilterConfig>& HDF5ArrayDataSetConfig::getFilters() const
{
  return m_filters;
}
