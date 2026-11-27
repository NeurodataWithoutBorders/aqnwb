#include "nwb/RegisterTypes.hpp"

#include "nwb/NWBFile.hpp"
#include "nwb/base/BaseImage.hpp"
#include "nwb/base/Images.hpp"
#include "nwb/base/NWBContainer.hpp"
#include "nwb/base/NWBData.hpp"
#include "nwb/base/NWBDataInterface.hpp"
#include "nwb/base/ProcessingModule.hpp"
#include "nwb/base/TimeSeries.hpp"
#include "nwb/device/Device.hpp"
#include "nwb/ecephys/ElectricalSeries.hpp"
#include "nwb/ecephys/SpikeEventSeries.hpp"
#include "nwb/epoch/TimeIntervals.hpp"
#include "nwb/event/DurationVectorData.hpp"
#include "nwb/event/EventsTable.hpp"
#include "nwb/event/TimestampVectorData.hpp"
#include "nwb/file/ElectrodeGroup.hpp"
#include "nwb/file/ElectrodesTable.hpp"
#include "nwb/file/Subject.hpp"
#include "nwb/hdmf/base/Container.hpp"
#include "nwb/hdmf/base/Data.hpp"
#include "nwb/hdmf/table/DynamicTable.hpp"
#include "nwb/hdmf/table/DynamicTableRegion.hpp"
#include "nwb/hdmf/table/ElementIdentifiers.hpp"
#include "nwb/hdmf/table/MeaningsTable.hpp"
#include "nwb/hdmf/table/VectorData.hpp"
#include "nwb/hdmf/table/VectorIndex.hpp"
#include "nwb/misc/AnnotationSeries.hpp"

#ifdef AQNWB_HAS_OPHYS
#  include "nwb/image/GrayscaleImage.hpp"
#  include "nwb/image/ImageMaskSeries.hpp"
#  include "nwb/image/ImageSeries.hpp"
#  include "nwb/image/IndexSeries.hpp"
#  include "nwb/image/OpticalSeries.hpp"
#  include "nwb/image/RGBAImage.hpp"
#  include "nwb/image/RGBImage.hpp"
#  include "nwb/ophys/CorrectedImageStack.hpp"
#  include "nwb/ophys/DfOverF.hpp"
#  include "nwb/ophys/Fluorescence.hpp"
#  include "nwb/ophys/ImageSegmentation.hpp"
#  include "nwb/ophys/ImagingPlane.hpp"
#  include "nwb/ophys/MotionCorrection.hpp"
#  include "nwb/ophys/OnePhotonSeries.hpp"
#  include "nwb/ophys/OpticalChannel.hpp"
#  include "nwb/ophys/PlaneSegmentation.hpp"
#  include "nwb/ophys/RoiResponseSeries.hpp"
#  include "nwb/ophys/TwoPhotonSeries.hpp"
#endif

#ifdef AQNWB_HAS_OGEN
#  include "nwb/ogen/OptogeneticSeries.hpp"
#  include "nwb/ogen/OptogeneticStimulusSite.hpp"
#endif

void AQNWB::NWB::registerAllTypes()
{
  // Calling registerSubclass references the object file of each class, which
  // ensures that the linker includes the class when linking statically
  // hdmf-common
  Container::registerSubclass();
  Data::registerSubclass();
  DynamicTable::registerSubclass();
  DynamicTableRegion::registerSubclass();
  ElementIdentifiers::registerSubclass();
  MeaningsTable::registerSubclass();
  VectorData::registerSubclass();
  VectorIndex::registerSubclass();
  // core
  AnnotationSeries::registerSubclass();
  BaseImage::registerSubclass();
  Image::registerSubclass();
  ImageReferences::registerSubclass();
  Images::registerSubclass();
  Device::registerSubclass();
  DurationVectorData::registerSubclass();
  ElectricalSeries::registerSubclass();
  ElectrodeGroup::registerSubclass();
  ElectrodesTable::registerSubclass();
  EventsTable::registerSubclass();
  NWBContainer::registerSubclass();
  NWBData::registerSubclass();
  NWBDataInterface::registerSubclass();
  NWBFile::registerSubclass();
  ProcessingModule::registerSubclass();
  SpikeEventSeries::registerSubclass();
  Subject::registerSubclass();
  TimeIntervals::registerSubclass();
  TimeSeries::registerSubclass();
  TimestampVectorData::registerSubclass();
#ifdef AQNWB_HAS_OPHYS
  // core: optical physiology module
  CorrectedImageStack::registerSubclass();
  DfOverF::registerSubclass();
  GrayscaleImage::registerSubclass();
  ImageMaskSeries::registerSubclass();
  IndexSeries::registerSubclass();
  MotionCorrection::registerSubclass();
  OpticalSeries::registerSubclass();
  RGBAImage::registerSubclass();
  RGBImage::registerSubclass();
  Fluorescence::registerSubclass();
  ImageSeries::registerSubclass();
  ImageSegmentation::registerSubclass();
  ImagingPlane::registerSubclass();
  OnePhotonSeries::registerSubclass();
  OpticalChannel::registerSubclass();
  PlaneSegmentation::registerSubclass();
  RoiResponseSeries::registerSubclass();
  TwoPhotonSeries::registerSubclass();
#endif
#ifdef AQNWB_HAS_OGEN
  // core: optogenetics module
  OptogeneticSeries::registerSubclass();
  OptogeneticStimulusSite::registerSubclass();
#endif
}
