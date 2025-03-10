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
   * @brief Constructs an HDF5FilterConfig object with the specified filter ID
   * and client data array.
   * @param filter_id The ID of the filter.
   * @param cd_values The client data array.
   */
  HDF5FilterConfig(H5Z_filter_t filter_id,
                   const std::vector<unsigned int>& cd_values);

  // The ID of the filter
  H5Z_filter_t filter_id;
  // The client data array
  std::vector<unsigned int> cd_values;

  /**
   * @brief Creates a GZIP (DEFLATE) filter configuration.
   * @param level The compression level (0-9). Default is 4.
   * @return A HDF5FilterConfig object for the GZIP filter.
   */
  static HDF5FilterConfig createGzipFilter(unsigned int level = 4);

  /**
   * @brief Creates a Shuffle filter configuration.
   * @return A HDF5FilterConfig object for the Shuffle filter.
   */
  static HDF5FilterConfig createShuffleFilter();

  /**
   * @brief Creates a Fletcher32 checksum filter configuration.
   * @return A HDF5FilterConfig object for the Fletcher32 filter.
   */
  static HDF5FilterConfig createFletcher32Filter();

  /**
   * @brief Creates an N-Bit filter configuration.
   * @return A HDF5FilterConfig object for the N-Bit filter.
   */
  static HDF5FilterConfig createNbitFilter();
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
   * @param cd_values The client data array.
   */
  void addFilter(H5Z_filter_t filter_id,
                 const std::vector<unsigned int>& cd_values);

  /**
   * @brief Adds a filter to the dataset configuration using an HDF5FilterConfig
   * object.
   * @param filter The HDF5FilterConfig object.
   */
  void addFilter(const HDF5FilterConfig& filter);

  /**
   * @brief Adds multiple filters to the dataset configuration using a vector of
   * HDF5FilterConfig objects.
   * @param filters The vector of HDF5FilterConfig objects.
   */
  void addFilters(const std::vector<HDF5FilterConfig>& filters);

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
