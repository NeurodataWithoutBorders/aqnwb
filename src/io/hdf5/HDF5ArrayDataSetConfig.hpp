#pragma once

#include <vector>

#include <H5Ppublic.h>

#include "io/BaseIO.hpp"

/*!
 * \namespace AQNWB::HDF5
 * \brief Namespace for all components of the HDF5 I/O backend
 */
namespace AQNWB::IO::HDF5
{

/**
 * @brief The configuration for an HDF5 filter
 *
 * This class defines the properties of an HDF5 filter, including the filter ID,
 * the number of elements in the client data array, and the client data array
 * itself.
 */
class HDF5FilterConfig
{
public:
  /**
   * @brief Constructs an HDF5FilterConfig object with the specified filter ID,
   * number of elements in the client data array, and the client data array.
   * @param filter_id The ID of the filter.
   * @param cd_nelmts The number of elements in the client data array.
   * @param cd_values The client data array.
   */
  HDF5FilterConfig(H5Z_filter_t filter_id,
                   unsigned int cd_nelmts,
                   const unsigned int* cd_values);

  // The ID of the filter
  H5Z_filter_t filter_id;
  // The number of elements in the client data array
  unsigned int cd_nelmts;
  // The client data array
  std::vector<unsigned int> cd_values;
};

/**
 * @brief The configuration for an HDF5 array dataset
 *
 * This class extends ArrayDataSetConfig to add additional configuration options
 * for HDF5-specific features, such as filters.
 */
class HDF5ArrayDataSetConfig : public ArrayDataSetConfig
{
public:
  /**
   * @brief Constructs an HDF5ArrayDataSetConfig object with the specified type,
   * shape, and chunking.
   * @param type The data type of the dataset.
   * @param shape The shape of the dataset.
   * @param chunking The chunking of the dataset.
   */
  HDF5ArrayDataSetConfig(const BaseDataType& type,
                         const SizeArray& shape,
                         const SizeArray& chunking);

  /**
   * @brief Adds a filter to the dataset configuration.
   * @param filter_id The ID of the filter.
   * @param cd_nelmts The number of elements in the client data array.
   * @param cd_values The client data array.
   */
  void addFilter(H5Z_filter_t filter_id,
                 unsigned int cd_nelmts,
                 const unsigned int* cd_values);

  /**
   * @brief Adds a filter to the dataset configuration using an HDF5FilterConfig
   * object.
   * @param filter The HDF5FilterConfig object.
   */
  void addFilter(const HDF5FilterConfig& filter);

  /**
   * @brief Returns the filters of the dataset.
   * @return The filters of the dataset.
   */
  const std::vector<HDF5FilterConfig>& getFilters() const;

private:
  // The filters of the dataset
  std::vector<HDF5FilterConfig> m_filters;
};

}  // namespace AQNWB::IO::HDF5
