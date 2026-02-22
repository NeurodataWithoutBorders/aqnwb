#include <any>
#include <numeric>
#include <stdexcept>
#include <typeindex>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "io/ReadIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "testUtils.hpp"

using namespace AQNWB;
using namespace AQNWB::IO;

// Helper function to compute the mean of a vector
template<typename T>
inline double test_compute_mean(const T& data)
{
  if (data.empty()) {
    throw std::runtime_error("Data vector is empty");
  }
  double sum = std::accumulate(data.begin(), data.end(), 0.0);
  return sum / static_cast<double>(data.size());
}

// Function to compute the mean using std::visit
inline double test_compute_mean(
    const BaseDataType::BaseDataVectorVariant& variant)
{
  return std::visit(
      [](auto&& arg) -> double
      {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          throw std::runtime_error("Invalid data type");
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
          throw std::runtime_error("Cannot compute mean of string data");
        } else {
          return test_compute_mean(arg);
        }
      },
      variant);
}

// Templated function to test variant conversion
template<typename T>
inline void test_variant_conversion(const std::vector<T>& data,
                                    BaseDataType::Type baseType)
{
  SizeArray shape = {data.size()};
  BaseDataType baseDataType = {baseType};

  DataBlockGeneric genericBlock(std::any(data), shape, typeid(T), baseDataType);

  auto variant = genericBlock.as_variant();
  REQUIRE(std::holds_alternative<std::vector<T>>(variant));
  REQUIRE(std::get<std::vector<T>>(variant) == data);
}

TEST_CASE("DataBlock - Basic Functionality", "[DataBlock]")
{
  SECTION("Constructor and Accessors")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    SizeArray shape = {5};

    DataBlock<int> block(data, shape);

    REQUIRE(block.data == data);
    REQUIRE(block.shape == shape);
    REQUIRE(block.typeIndex == typeid(int));
  }

  SECTION("Modification")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    SizeArray shape = {5};

    DataBlock<int> block(data, shape);
    block.data[2] = 10;

    // cppcheck-suppress knownConditionTrueFalse
    REQUIRE(block.data[2] == 10);
  }

  SECTION("From Generic")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    SizeArray shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlock<int> block(data, shape);
    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto newBlock = DataBlock<int>::fromGeneric(genericBlock);

    REQUIRE(newBlock.data == block.data);
    REQUIRE(newBlock.shape == block.shape);
  }
}

TEST_CASE("DataBlockGeneric - Basic Functionality", "[DataBlockGeneric]")
{
  SECTION("Constructor and Accessors")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    SizeArray shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlock<int> block(data, shape);
    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    REQUIRE(genericBlock.shape == shape);
    REQUIRE(genericBlock.typeIndex == typeid(int));
    REQUIRE(std::any_cast<std::vector<int>>(genericBlock.data) == data);

    auto newBlock = DataBlock<int>::fromGeneric(genericBlock);
    REQUIRE(newBlock.data == data);
    REQUIRE(newBlock.shape == shape);
  }
}

TEST_CASE("DataBlock - Edge Cases", "[DataBlock]")
{
  SECTION("Empty Data Block")
  {
    std::vector<int> data;
    SizeArray shape;

    DataBlock<int> block(data, shape);

    REQUIRE(block.data.empty());
    REQUIRE(block.shape.empty());
  }

  SECTION("Multi-dimensional Data Block")
  {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    SizeArray shape = {2, 3};

    DataBlock<int> block(data, shape);

    REQUIRE(block.data == data);
    REQUIRE(block.shape == shape);
  }
}

TEST_CASE("DataBlockGeneric - as_variant Method", "[DataBlockGeneric]")
{
  SECTION("Variant Conversion")
  {
    test_variant_conversion<uint8_t>({1, 2, 3, 4, 5}, BaseDataType::T_U8);
    test_variant_conversion<uint16_t>({1, 2, 3, 4, 5}, BaseDataType::T_U16);
    test_variant_conversion<uint32_t>({1, 2, 3, 4, 5}, BaseDataType::T_U32);
    test_variant_conversion<uint64_t>({1, 2, 3, 4, 5}, BaseDataType::T_U64);
    test_variant_conversion<int8_t>({1, 2, 3, 4, 5}, BaseDataType::T_I8);
    test_variant_conversion<int16_t>({1, 2, 3, 4, 5}, BaseDataType::T_I16);
    test_variant_conversion<int32_t>({1, 2, 3, 4, 5}, BaseDataType::T_I32);
    test_variant_conversion<int64_t>({1, 2, 3, 4, 5}, BaseDataType::T_I64);
    test_variant_conversion<float>({1.0f, 2.0f, 3.0f, 4.0f, 5.0f},
                                   BaseDataType::T_F32);
    test_variant_conversion<double>({1.0, 2.0, 3.0, 4.0, 5.0},
                                    BaseDataType::T_F64);
    test_variant_conversion<std::string>({"a", "b", "c"}, BaseDataType::T_STR);
  }

  SECTION("Unsupported Type")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    SizeArray shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_STR};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto variant = genericBlock.as_variant();
    REQUIRE(std::holds_alternative<std::monostate>(variant));
  }
}

TEST_CASE("DataBlockGeneric - Compute Mean with std::visit",
          "[DataBlockGeneric]")
{
  SECTION("Compute Mean for Integer Data")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    SizeArray shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto variant = genericBlock.as_variant();

    double mean = test_compute_mean(variant);

    REQUIRE(mean == Catch::Approx(3.0));
  }

  SECTION("Compute Mean for Float Data")
  {
    std::vector<float> data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    SizeArray shape = {5};
    BaseDataType baseDataType = {BaseDataType::T_F32};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(float), baseDataType);

    auto variant = genericBlock.as_variant();

    double mean = test_compute_mean(variant);

    REQUIRE(mean == Catch::Approx(3.0));
  }

  SECTION("Compute Mean for Empty Data")
  {
    std::vector<int> data;
    SizeArray shape = {0};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto variant = genericBlock.as_variant();

    REQUIRE_THROWS_AS(test_compute_mean(variant), std::runtime_error);
  }

  SECTION("Compute Mean for String Data")
  {
    std::vector<std::string> data = {"a", "b", "c"};
    SizeArray shape = {3};
    BaseDataType baseDataType = {BaseDataType::T_STR};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(std::string), baseDataType);

    auto variant = genericBlock.as_variant();

    REQUIRE_THROWS_AS(test_compute_mean(variant), std::runtime_error);
  }
}

TEST_CASE("ConstMultiArrayView - Basic Functionality", "[ConstMultiArrayView]")
{
  SECTION("1D View")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::array<size_t, 1> shape = {5};
    std::array<size_t, 1> strides = {1};

    ConstMultiArrayView<int, 1> view(data.data(), shape, strides);

    REQUIRE(view.shape() == shape);
    for (size_t i = 0; i < 5; ++i) {
      REQUIRE(view[i] == data[i]);
    }

    // Test iterators
    size_t count = 0;
    for (auto it = view.begin(); it != view.end(); ++it) {
      REQUIRE(*it == data[count]);
      count++;
    }
    REQUIRE(count == 5);
  }

  SECTION("2D View")
  {
    // 2x3 array
    // 1 2 3
    // 4 5 6
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    std::array<size_t, 2> shape = {2, 3};
    std::array<size_t, 2> strides = {3, 1};

    ConstMultiArrayView<int, 2> view(data.data(), shape, strides);

    REQUIRE(view.shape() == shape);

    REQUIRE(view[0][0] == 1);
    REQUIRE(view[0][1] == 2);
    REQUIRE(view[0][2] == 3);
    REQUIRE(view[1][0] == 4);
    REQUIRE(view[1][1] == 5);
    REQUIRE(view[1][2] == 6);
  }

  SECTION("3D View")
  {
    // 2x2x2 array
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
    std::array<size_t, 3> shape = {2, 2, 2};
    std::array<size_t, 3> strides = {4, 2, 1};

    ConstMultiArrayView<int, 3> view(data.data(), shape, strides);

    REQUIRE(view.shape() == shape);

    REQUIRE(view[0][0][0] == 1);
    REQUIRE(view[0][0][1] == 2);
    REQUIRE(view[0][1][0] == 3);
    REQUIRE(view[0][1][1] == 4);
    REQUIRE(view[1][0][0] == 5);
    REQUIRE(view[1][0][1] == 6);
    REQUIRE(view[1][1][0] == 7);
    REQUIRE(view[1][1][1] == 8);
  }
}

TEST_CASE("DataBlock - as_multi_array", "[DataBlock]")
{
  SECTION("1D Array")
  {
    std::vector<int> data = {10, 20, 30};
    SizeArray shape = {3};
    DataBlock<int> block(data, shape);

    auto view = block.as_multi_array<1>();

    REQUIRE(view.shape()[0] == 3);
    REQUIRE(view[0] == 10);
    REQUIRE(view[1] == 20);
    REQUIRE(view[2] == 30);
  }

  SECTION("2D Array")
  {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    SizeArray shape = {2, 3};
    DataBlock<int> block(data, shape);

    auto view = block.as_multi_array<2>();

    REQUIRE(view.shape()[0] == 2);
    REQUIRE(view.shape()[1] == 3);

    REQUIRE(view[0][0] == 1);
    REQUIRE(view[0][1] == 2);
    REQUIRE(view[0][2] == 3);
    REQUIRE(view[1][0] == 4);
    REQUIRE(view[1][1] == 5);
    REQUIRE(view[1][2] == 6);
  }

  SECTION("Invalid Dimensions")
  {
    std::vector<int> data = {1, 2, 3, 4};
    SizeArray shape = {2, 2};
    DataBlock<int> block(data, shape);

    REQUIRE_THROWS_AS(block.as_multi_array<1>(), std::invalid_argument);
    REQUIRE_THROWS_AS(block.as_multi_array<3>(), std::invalid_argument);
  }

  SECTION("Data Size Mismatch")
  {
    std::vector<int> data = {1, 2, 3};  // Missing one element
    SizeArray shape = {2, 2};
    DataBlock<int> block(data, shape);

    REQUIRE_THROWS_AS(block.as_multi_array<2>(), std::invalid_argument);
  }
}

// Detection trait: checks whether T::toLinkArrayDataSetConfig() is callable
template<typename T, typename = void>
struct has_toLinkArrayDataSetConfig : std::false_type
{
};
template<typename T>
struct has_toLinkArrayDataSetConfig<
    T,
    std::void_t<decltype(std::declval<T>().toLinkArrayDataSetConfig())>>
    : std::true_type
{
};

// Compile-time assertions: toLinkArrayDataSetConfig must be available for
// Dataset wrappers and disabled for Attribute wrappers.
static_assert(has_toLinkArrayDataSetConfig<
                  ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset,
                                  int32_t>>::value,
              "toLinkArrayDataSetConfig must be callable for Dataset wrappers");
static_assert(
    !has_toLinkArrayDataSetConfig<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute,
                        int32_t>>::value,
    "toLinkArrayDataSetConfig must not be callable for Attribute wrappers");

TEST_CASE("ReadDataWrapper; introspection methods", "[ReadDataWrapper]")
{
  // Set up a single HDF5 file with all objects required by the sections below.
  std::string filePath = getTestFilePath("test_ReadDataWrapper.h5");
  auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(filePath);
  REQUIRE(hdf5io->open() == Status::Success);

  // 1D I32 dataset (chunk=1), data = {1,2,3,4,5}  — used for shape, values,
  // slicing
  const std::string dsI32_1dPath = "/ds_i32_1d";
  const std::vector<int32_t> dsI32_1dData = {1, 2, 3, 4, 5};
  {
    IO::ArrayDataSetConfig cfg(
        IO::BaseDataType::I32, SizeArray {0}, SizeArray {1});
    auto ds = hdf5io->createArrayDataSet(cfg, dsI32_1dPath);
    ds->writeDataBlock({5}, {0}, IO::BaseDataType::I32, dsI32_1dData.data());
  }

  // 2D I32 dataset (no chunking), shape 2×3, data = {1,2,3,4,5,6}
  const std::string dsI32_2dPath = "/ds_i32_2d";
  {
    IO::ArrayDataSetConfig cfg(
        IO::BaseDataType::I32, SizeArray {2, 3}, SizeArray {0, 0});
    auto ds = hdf5io->createArrayDataSet(cfg, dsI32_2dPath);
    std::vector<int32_t> data = {1, 2, 3, 4, 5, 6};
    ds->writeDataBlock({2, 3}, {0, 0}, IO::BaseDataType::I32, data.data());
  }

  // F32 dataset (chunk=5), 10 elements — used for getDataType, getChunking,
  // and toLinkArrayDataSetConfig
  const std::string dsF32Path = "/ds_f32_chunked";
  const SizeArray dsF32Chunking = {5};
  {
    IO::ArrayDataSetConfig cfg(
        IO::BaseDataType::F32, SizeArray {10}, dsF32Chunking);
    auto ds = hdf5io->createArrayDataSet(cfg, dsF32Path);
    std::vector<float> data(10, 1.0f);
    ds->writeDataBlock({10}, {0}, IO::BaseDataType::F32, data.data());
  }

  // Group + I32 attribute (3 elements) — used for attribute tests
  const std::string grpPath = "/grp";
  const std::string attrPath = "/grp/attr";
  auto grpStatus = hdf5io->createGroup(grpPath);
  REQUIRE(grpStatus == Status::Success);
  const std::vector<int32_t> attrData = {10, 20, 30};
  auto attrStatus = hdf5io->createAttribute(
      IO::BaseDataType::I32, attrData.data(), grpPath, "attr", 3);
  REQUIRE(attrStatus == Status::Success);

  // -----------------------------------------------------------------------

  SECTION("getPath and getIO return correct values")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t> wrapper(
        hdf5io, dsI32_1dPath);
    REQUIRE(wrapper.getPath() == dsI32_1dPath);
    REQUIRE(wrapper.getIO() == hdf5io);
  }

  SECTION("getStorageObjectType returns correct type")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>
        dsWrapper(hdf5io, dsI32_1dPath);
    REQUIRE(dsWrapper.getStorageObjectType()
            == AQNWB::Types::StorageObjectType::Dataset);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        attrWrapper(hdf5io, attrPath);
    REQUIRE(attrWrapper.getStorageObjectType()
            == AQNWB::Types::StorageObjectType::Attribute);
  }

  SECTION("isType compile-time check")
  {
    static_assert(ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset,
                                  float>::isType<float>(),
                  "isType<float>() must return true for VTYPE=float");
    static_assert(!ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset,
                                   float>::isType<int32_t>(),
                  "isType<int32_t>() must return false for VTYPE=float");
  }

  SECTION("getShape and getNumDimensions for a 1D dataset")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t> wrapper(
        hdf5io, dsI32_1dPath);
    SizeArray shape = wrapper.getShape();
    REQUIRE(shape.size() == 1);
    REQUIRE(shape[0] == 5);
    REQUIRE(wrapper.getNumDimensions() == 1);
  }

  SECTION("getShape and getNumDimensions for a 2D dataset")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t> wrapper(
        hdf5io, dsI32_2dPath);
    SizeArray shape = wrapper.getShape();
    REQUIRE(shape.size() == 2);
    REQUIRE(shape[0] == 2);
    REQUIRE(shape[1] == 3);
    REQUIRE(wrapper.getNumDimensions() == 2);
  }

  SECTION("getShape and getNumDimensions for an attribute")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        wrapper(hdf5io, attrPath);
    SizeArray shape = wrapper.getShape();
    REQUIRE(shape.size() == 1);
    REQUIRE(shape[0] == 3);
    REQUIRE(wrapper.getNumDimensions() == 1);
  }

  SECTION("getDataType returns correct type for a dataset")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, float> wrapper(
        hdf5io, dsF32Path);
    REQUIRE(wrapper.getDataType() == IO::BaseDataType::F32);
  }

  SECTION("getDataType returns correct type for an attribute")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        wrapper(hdf5io, attrPath);
    REQUIRE(wrapper.getDataType() == IO::BaseDataType::I32);
  }

  SECTION("getChunking returns correct chunking for a dataset")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, float> wrapper(
        hdf5io, dsF32Path);
    REQUIRE(wrapper.getChunking() == dsF32Chunking);
  }

  SECTION("getChunking returns empty for an attribute")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        wrapper(hdf5io, attrPath);
    REQUIRE(wrapper.getChunking().empty());
  }

  SECTION("exists returns true for existing objects, false for missing ones")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>
        presentDs(hdf5io, dsI32_1dPath);
    REQUIRE(presentDs.exists() == true);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>
        missingDs(hdf5io, "/nonexistent");
    REQUIRE(missingDs.exists() == false);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        presentAttr(hdf5io, attrPath);
    REQUIRE(presentAttr.exists() == true);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        missingAttr(hdf5io, "/grp/nonexistent");
    REQUIRE(missingAttr.exists() == false);
  }

  SECTION("valuesGeneric and values for a dataset")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t> wrapper(
        hdf5io, dsI32_1dPath);

    auto generic = wrapper.valuesGeneric();
    REQUIRE(generic.shape[0] == 5);

    auto typed = wrapper.values();
    REQUIRE(typed.data == dsI32_1dData);
    REQUIRE(typed.shape[0] == 5);
  }

  SECTION("valuesGeneric and values for an attribute")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        wrapper(hdf5io, attrPath);

    auto generic = wrapper.valuesGeneric();
    REQUIRE(generic.shape[0] == 3);

    auto typed = wrapper.values();
    REQUIRE(typed.data == attrData);
  }

  SECTION("valuesGeneric and values with hyperslab slicing for a dataset")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t> wrapper(
        hdf5io, dsI32_1dPath);

    // Read elements [2..4] (start=2, count=3) → {3,4,5}
    auto sliced = wrapper.valuesGeneric({2}, {3});
    auto slicedTyped = DataBlock<int32_t>::fromGeneric(sliced);
    REQUIRE(slicedTyped.shape[0] == 3);
    REQUIRE(slicedTyped.data == std::vector<int32_t>({3, 4, 5}));

    auto slicedValues = wrapper.values({2}, {3});
    REQUIRE(slicedValues.data == std::vector<int32_t>({3, 4, 5}));
  }

  SECTION("toLinkArrayDataSetConfig creates config with correct target path")
  {
    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, float> wrapper(
        hdf5io, dsF32Path);

    IO::LinkArrayDataSetConfig linkConfig = wrapper.toLinkArrayDataSetConfig();
    REQUIRE(linkConfig.getTargetPath() == dsF32Path);
    REQUIRE(linkConfig.isLink() == true);
    REQUIRE(linkConfig.targetExists(*hdf5io) == true);
    REQUIRE(linkConfig.getTargetShape(*hdf5io) == SizeArray {10});
    REQUIRE(linkConfig.getTargetChunking(*hdf5io) == dsF32Chunking);
    REQUIRE(linkConfig.getTargetDataType(*hdf5io) == IO::BaseDataType::F32);

    // Verify the link can be created in the file
    std::string linkPath = "/link_to_ds_f32";
    auto linkResult = hdf5io->createArrayDataSet(linkConfig, linkPath);
    REQUIRE(linkResult == nullptr);
    REQUIRE(hdf5io->objectExists(linkPath));
  }

  hdf5io->close();
}