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
  std::vector<SizeType> shape = {data.size()};
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
    std::vector<SizeType> shape = {5};

    DataBlock<int> block(data, shape);

    REQUIRE(block.data == data);
    REQUIRE(block.shape == shape);
    REQUIRE(block.typeIndex == typeid(int));
  }

  SECTION("Modification")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};

    DataBlock<int> block(data, shape);
    block.data[2] = 10;

    REQUIRE(block.data[2] == 10);
  }

  SECTION("From Generic")
  {
    std::vector<int> data = {1, 2, 3, 4, 5};
    std::vector<SizeType> shape = {5};
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
    std::vector<SizeType> shape = {5};
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
    std::vector<SizeType> shape;

    DataBlock<int> block(data, shape);

    REQUIRE(block.data.empty());
    REQUIRE(block.shape.empty());
  }

  SECTION("Multi-dimensional Data Block")
  {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    std::vector<SizeType> shape = {2, 3};

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
    std::vector<SizeType> shape = {5};
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
    std::vector<SizeType> shape = {5};
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
    std::vector<SizeType> shape = {5};
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
    std::vector<SizeType> shape = {0};
    BaseDataType baseDataType = {BaseDataType::T_I32};

    DataBlockGeneric genericBlock(
        std::any(data), shape, typeid(int), baseDataType);

    auto variant = genericBlock.as_variant();

    REQUIRE_THROWS_AS(test_compute_mean(variant), std::runtime_error);
  }

  SECTION("Compute Mean for String Data")
  {
    std::vector<std::string> data = {"a", "b", "c"};
    std::vector<SizeType> shape = {3};
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
    std::vector<SizeType> shape = {3};
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
    std::vector<SizeType> shape = {2, 3};
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
    std::vector<SizeType> shape = {2, 2};
    DataBlock<int> block(data, shape);

    REQUIRE_THROWS_AS(block.as_multi_array<1>(), std::invalid_argument);
    REQUIRE_THROWS_AS(block.as_multi_array<3>(), std::invalid_argument);
  }

  SECTION("Data Size Mismatch")
  {
    std::vector<int> data = {1, 2, 3};  // Missing one element
    std::vector<SizeType> shape = {2, 2};
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
static_assert(
    has_toLinkArrayDataSetConfig<
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
  SECTION("getDataType returns correct type for a dataset")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_dtype.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/float32dataset";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::F32, SizeArray {10}, SizeArray {5});
    auto dataset = hdf5io->createArrayDataSet(config, dataPath);
    std::vector<float> testData(10, 1.0f);
    dataset->writeDataBlock({10}, {0}, IO::BaseDataType::F32, testData.data());

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, float>>(
        hdf5io, dataPath);
    REQUIRE(wrapper->getDataType() == IO::BaseDataType::F32);

    hdf5io->close();
  }

  SECTION("getDataType returns correct type for an attribute")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_attr_dtype.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    hdf5io->createGroup("/mygroup");
    std::vector<int32_t> attrData = {42};
    hdf5io->createAttribute(
        IO::BaseDataType::I32, attrData.data(), "/mygroup", "myattr", 1);

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>>(
        hdf5io, "/mygroup/myattr");
    REQUIRE(wrapper->getDataType() == IO::BaseDataType::I32);
    // Attributes are not chunked; getChunking() should return an empty array
    REQUIRE(wrapper->getChunking().empty());

    hdf5io->close();
  }

  SECTION("getChunking returns correct chunking for a dataset")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_chunking.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/chunked_dataset";
    SizeArray chunking = {5};
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {0}, chunking);
    auto dataset = hdf5io->createArrayDataSet(config, dataPath);
    std::vector<int32_t> testData(10);
    dataset->writeDataBlock({10}, {0}, IO::BaseDataType::I32, testData.data());

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>>(
        hdf5io, dataPath);
    SizeArray resultChunking = wrapper->getChunking();
    REQUIRE(resultChunking == chunking);

    hdf5io->close();
  }

  SECTION("toLinkArrayDataSetConfig creates config with correct target path")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_toLink.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/original_data";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::F64, SizeArray {20}, SizeArray {10});
    auto dataset = hdf5io->createArrayDataSet(config, dataPath);
    std::vector<double> testData(20, 3.14);
    dataset->writeDataBlock({20}, {0}, IO::BaseDataType::F64, testData.data());

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, double>>(
        hdf5io, dataPath);

    // Create LinkArrayDataSetConfig from the wrapper
    IO::LinkArrayDataSetConfig linkConfig = wrapper->toLinkArrayDataSetConfig();
    REQUIRE(linkConfig.getTargetPath() == dataPath);
    REQUIRE(linkConfig.isLink() == true);
    REQUIRE(linkConfig.targetExists(*hdf5io) == true);

    // Verify introspection via the link config
    REQUIRE(linkConfig.getTargetShape(*hdf5io) == SizeArray {20});
    REQUIRE(linkConfig.getTargetChunking(*hdf5io) == SizeArray {10});
    REQUIRE(linkConfig.getTargetDataType(*hdf5io) == IO::BaseDataType::F64);

    // Use link config to create an actual link in the file
    std::string linkPath = "/link_to_original";
    auto linkResult = hdf5io->createArrayDataSet(linkConfig, linkPath);
    REQUIRE(linkResult == nullptr);
    REQUIRE(hdf5io->objectExists(linkPath));

    hdf5io->close();
  }

  SECTION("getPath and getIO return correct values")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_path.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/my_dataset";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {5}, SizeArray {0});
    hdf5io->createArrayDataSet(config, dataPath);

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>>(
        hdf5io, dataPath);
    REQUIRE(wrapper->getPath() == dataPath);
    REQUIRE(wrapper->getIO() == hdf5io);

    hdf5io->close();
  }

  SECTION("getStorageObjectType returns correct type")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_otype.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {5}, SizeArray {0});
    hdf5io->createArrayDataSet(config, "/ds");
    hdf5io->createGroup("/grp");
    std::vector<int32_t> val = {1};
    hdf5io->createAttribute(
        IO::BaseDataType::I32, val.data(), "/grp", "attr", 1);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>
        dsWrapper(hdf5io, "/ds");
    REQUIRE(dsWrapper.getStorageObjectType()
            == AQNWB::Types::StorageObjectType::Dataset);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        attrWrapper(hdf5io, "/grp/attr");
    REQUIRE(attrWrapper.getStorageObjectType()
            == AQNWB::Types::StorageObjectType::Attribute);

    hdf5io->close();
  }

  SECTION("isType compile-time check")
  {
    // isType<>() is a static constexpr function — check at compile time
    static_assert(
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset,
                        float>::isType<float>(),
        "isType<float>() must return true for VTYPE=float");
    static_assert(
        !ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset,
                         float>::isType<int32_t>(),
        "isType<int32_t>() must return false for VTYPE=float");
  }

  SECTION("getShape and getNumDimensions for a 1D dataset")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_shape1d.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/data1d";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {0}, SizeArray {1});
    auto dataset = hdf5io->createArrayDataSet(config, dataPath);
    std::vector<int32_t> testData = {1, 2, 3, 4, 5};
    dataset->writeDataBlock({5}, {0}, IO::BaseDataType::I32, testData.data());

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>>(
        hdf5io, dataPath);
    SizeArray shape = wrapper->getShape();
    REQUIRE(shape.size() == 1);
    REQUIRE(shape[0] == 5);
    REQUIRE(wrapper->getNumDimensions() == 1);

    hdf5io->close();
  }

  SECTION("getShape and getNumDimensions for a 2D dataset")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_shape2d.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/data2d";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {2, 3}, SizeArray {0, 0});
    auto dataset = hdf5io->createArrayDataSet(config, dataPath);
    std::vector<int32_t> testData = {1, 2, 3, 4, 5, 6};
    dataset->writeDataBlock({2, 3}, {0, 0}, IO::BaseDataType::I32,
                            testData.data());

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>>(
        hdf5io, dataPath);
    SizeArray shape = wrapper->getShape();
    REQUIRE(shape.size() == 2);
    REQUIRE(shape[0] == 2);
    REQUIRE(shape[1] == 3);
    REQUIRE(wrapper->getNumDimensions() == 2);

    hdf5io->close();
  }

  SECTION("getShape for an attribute")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_attrshape.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    hdf5io->createGroup("/grp");
    std::vector<int32_t> attrData = {10, 20, 30};
    hdf5io->createAttribute(
        IO::BaseDataType::I32, attrData.data(), "/grp", "attr", 3);

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>>(
        hdf5io, "/grp/attr");
    SizeArray shape = wrapper->getShape();
    REQUIRE(shape.size() == 1);
    REQUIRE(shape[0] == 3);
    REQUIRE(wrapper->getNumDimensions() == 1);

    hdf5io->close();
  }

  SECTION("exists returns true for existing dataset and attribute")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_exists.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/existing_ds";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {5}, SizeArray {0});
    hdf5io->createArrayDataSet(config, dataPath);
    hdf5io->createGroup("/grp");
    std::vector<int32_t> val = {1};
    hdf5io->createAttribute(
        IO::BaseDataType::I32, val.data(), "/grp", "attr", 1);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>
        dsWrapper(hdf5io, dataPath);
    REQUIRE(dsWrapper.exists() == true);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>
        missingDsWrapper(hdf5io, "/nonexistent");
    REQUIRE(missingDsWrapper.exists() == false);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        attrWrapper(hdf5io, "/grp/attr");
    REQUIRE(attrWrapper.exists() == true);

    ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>
        missingAttrWrapper(hdf5io, "/grp/nonexistent");
    REQUIRE(missingAttrWrapper.exists() == false);

    hdf5io->close();
  }

  SECTION("valuesGeneric and values for a dataset")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_values_ds.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/int_data";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {0}, SizeArray {1});
    auto dataset = hdf5io->createArrayDataSet(config, dataPath);
    std::vector<int32_t> testData = {10, 20, 30};
    dataset->writeDataBlock({3}, {0}, IO::BaseDataType::I32, testData.data());

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>>(
        hdf5io, dataPath);

    auto generic = wrapper->valuesGeneric();
    REQUIRE(generic.shape[0] == 3);

    auto typed = wrapper->values();
    REQUIRE(typed.data == testData);
    REQUIRE(typed.shape[0] == 3);

    hdf5io->close();
  }

  SECTION("valuesGeneric and values for an attribute")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_values_attr.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    hdf5io->createGroup("/grp");
    std::vector<int32_t> attrData = {7, 8, 9};
    hdf5io->createAttribute(
        IO::BaseDataType::I32, attrData.data(), "/grp", "attr", 3);

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Attribute, int32_t>>(
        hdf5io, "/grp/attr");

    auto generic = wrapper->valuesGeneric();
    REQUIRE(generic.shape[0] == 3);

    auto typed = wrapper->values();
    REQUIRE(typed.data == attrData);

    hdf5io->close();
  }

  SECTION("valuesGeneric with hyperslab slicing for a dataset")
  {
    std::string path = getTestFilePath("test_ReadDataWrapper_slice.h5");
    auto hdf5io = std::make_shared<IO::HDF5::HDF5IO>(path);
    hdf5io->open();

    std::string dataPath = "/sliceable";
    IO::ArrayDataSetConfig config(
        IO::BaseDataType::I32, SizeArray {0}, SizeArray {1});
    auto dataset = hdf5io->createArrayDataSet(config, dataPath);
    std::vector<int32_t> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    dataset->writeDataBlock({10}, {0}, IO::BaseDataType::I32, testData.data());

    auto wrapper = std::make_unique<
        ReadDataWrapper<AQNWB::Types::StorageObjectType::Dataset, int32_t>>(
        hdf5io, dataPath);

    // Read elements [2..4] (start=2, count=3)
    auto sliced = wrapper->valuesGeneric({2}, {3});
    auto slicedTyped = DataBlock<int32_t>::fromGeneric(sliced);
    REQUIRE(slicedTyped.shape[0] == 3);
    REQUIRE(slicedTyped.data == std::vector<int32_t>({3, 4, 5}));

    // Same via values(start, count)
    auto slicedValues = wrapper->values({2}, {3});
    REQUIRE(slicedValues.data == std::vector<int32_t>({3, 4, 5}));

    hdf5io->close();
  }
}