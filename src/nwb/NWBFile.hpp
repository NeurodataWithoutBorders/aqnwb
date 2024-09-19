#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "BaseIO.hpp"
#include "Types.hpp"
#include "nwb/RecordingContainers.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/file/ElectrodeTable.hpp"
#include "nwb/hdmf/base/Container.hpp"

/*!
 * \namespace AQNWB::NWB
 * \brief Namespace for all classes related to the NWB data standard
 */
namespace AQNWB::NWB
{

/**
 * @brief The NWBFile class provides an interface for setting up and managing
 * the NWB file.
 */
class NWBFile : public Container
{
public:
  /**
   * @brief Constructor for NWBFile class.
   * @param idText The identifier text for the NWBFile.
   * @param io The shared pointer to the IO object.
   */
  NWBFile(const std::string& idText, std::shared_ptr<BaseIO> io);

  /**
   * @brief Deleted copy constructor to prevent construction-copying.
   */
  NWBFile(const NWBFile&) = delete;

  /**
   * @brief Deleted copy assignment operator to prevent copying.
   */
  NWBFile& operator=(const NWBFile&) = delete;

  /**
   * @brief Destructor for NWBFile class.
   */
  ~NWBFile();

  /**
   * @brief Initializes the NWB file by opening and setting up the file
   * structure.
   * @param description A description of the NWBFile session.
   * @param dataCollection Information about the data collection methods.
   */
  Status initialize(const std::string description = "a recording session",
                    const std::string dataCollection = "");

  /**
   * @brief Finalizes the NWB file by closing it.
   */
  Status finalize();

  /**
   * @brief Create ElectricalSeries objects to record data into.
   * Created objects are stored in recordingContainers.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param recordingArrays vector of ChannelVector indicating the electrodes to
   *                        record from. A separate ElectricalSeries will be
   *                        created for each ChannelVector.
   * @param recordingNames vector indicating the names of the ElectricalSeries
   * within the acquisition group
   * @param dataType The data type of the elements in the data block.
   * @param recordingContainers The container to store the created TimeSeries.
   * @param containerIndexes The indexes of the containers added to
   * recordingContainers
   * @return Status The status of the object creation operation.
   */
  Status createElectricalSeries(
      std::vector<Types::ChannelVector> recordingArrays,
      std::vector<std::string> recordingNames,
      const BaseDataType& dataType = BaseDataType::I16,
      RecordingContainers* recordingContainers = nullptr,
      std::vector<SizeType>& containerIndexes = emptyContainerIndexes);

  /**
   * @brief Create SpikeEventSeries objects to record data into.
   * Created objects are stored in recordingContainers.
   * @param recordingArrays vector of ChannelVector indicating the electrodes to
   *                        record from. A separate ElectricalSeries will be
   *                        created for each ChannelVector.
   * @param recordingNames vector indicating the names of the SpikeEventSeries
   * within the acquisition group
   * @param dataType The data type of the elements in the data block.
   * @param recordingContainers The container to store the created TimeSeries.
   * @param containerIndexes The indexes of the containers added to
   * recordingContainers
   * @return Status The status of the object creation operation.
   */
  Status createSpikeEventSeries(
      std::vector<Types::ChannelVector> recordingArrays,
      std::vector<std::string> recordingNames,
      const BaseDataType& dataType = BaseDataType::I16,
      RecordingContainers* recordingContainers = nullptr,
      std::vector<SizeType>& containerIndexes = emptyContainerIndexes);

protected:
  /**
   * @brief Creates the default file structure.
   * Note, this function will fail if the file is in a mode where
   * new objects cannot be added, which can be checked via
   * nwbfile.io->canModifyObjects()
   * @param description A description of the NWBFile session.
   * @param dataCollection Information about the data collection methods.
   * @return Status The status of the file structure creation.
   */
  Status createFileStructure(std::string description,
                             std::string dataCollection);

private:
  /**
   * @brief Factory method for creating recording data.
   * @param type The base data type.
   * @param size The size of the dataset.
   * @param chunking The chunking size of the dataset.
   * @param path The location in the file of the new dataset.
   * @return std::unique_ptr<BaseRecordingData> The unique pointer to the
   * created recording data.
   */
  std::unique_ptr<BaseRecordingData> createRecordingData(
      BaseDataType type,
      const SizeArray& size,
      const SizeArray& chunking,
      const std::string& path);

  /**
   * @brief Saves the specification files for the schema.
   * @param specPath The location in the file to store the spec information.
   * @param versionNumber The version number of the specification files.
   * @param specVariables The contents of the specification files.
   * These values are generated from the nwb schema by
   * `resources/generate_spec_files.py`
   */
  template<SizeType N>
  void cacheSpecifications(
      const std::string& specPath,
      const std::string& versionNumber,
      const std::array<std::pair<std::string_view, std::string_view>, N>&
          specVariables);

  inline const static std::string acquisitionPath = "/acquisition";
  static std::vector<SizeType> emptyContainerIndexes;

private:
  /**
   * @brief The ElectrodeTable for the file
   */
  std::unique_ptr<ElectrodeTable> m_electrodeTable;
  const std::string m_identifierText;
};

}  // namespace AQNWB::NWB