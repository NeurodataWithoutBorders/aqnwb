#include "io/hdf5/HDF5ArrayDataSetConfig.hpp"

using namespace AQNWB::IO::HDF5;

HDF5FilterConfig::HDF5FilterConfig(H5Z_filter_t filter_id,
                                   unsigned int cd_nelmts,
                                   const unsigned int* cd_values)
    : filter_id(filter_id)
    , cd_nelmts(cd_nelmts)
    , cd_values(cd_values, cd_values + cd_nelmts)
{
}

HDF5ArrayDataSetConfig::HDF5ArrayDataSetConfig(const BaseDataType& type,
                                               const SizeArray& shape,
                                               const SizeArray& chunking)
    : ArrayDataSetConfig(type, shape, chunking)
{
}

void HDF5ArrayDataSetConfig::addFilter(H5Z_filter_t filter_id,
                                       unsigned int cd_nelmts,
                                       const unsigned int* cd_values)
{
  m_filters.emplace_back(filter_id, cd_nelmts, cd_values);
}

void HDF5ArrayDataSetConfig::addFilter(const HDF5FilterConfig& filter)
{
  m_filters.push_back(filter);
}

const std::vector<HDF5FilterConfig>& HDF5ArrayDataSetConfig::getFilters() const
{
  return m_filters;
}
