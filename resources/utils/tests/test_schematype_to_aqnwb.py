"""
Tests for the schematype_to_aqnwb utility.

The tests generate code for a small test extension and check the generated Spec structs,
initialize methods, and AQNWB-TODO markers. Run via:

    uv run --with pytest --with hdmf --with pynwb --with ruamel.yaml pytest resources/utils/tests
"""

import argparse
import re
import sys
from pathlib import Path

import pytest

UTILS_DIR = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(UTILS_DIR))

import schematype_to_aqnwb as generator  # noqa: E402

AQNWB_SRC_DIR = UTILS_DIR.parent.parent / "src"

NAMESPACE_YAML = """namespaces:
- author:
  - Test Author
  contact:
  - test@example.com
  doc: Test extension for the schematype_to_aqnwb utility
  name: ndx-test
  schema:
  - namespace: core
    neurodata_types:
    - Device
    - TimeSeries
    - DynamicTable
    - NWBContainer
  - source: ndx-test.extensions.yaml
  version: 0.1.0
"""

EXTENSIONS_YAML = """groups:
- neurodata_type_def: TestDevice
  neurodata_type_inc: Device
  doc: A test device
  attributes:
  - name: asset_tag
    dtype: text
    doc: The asset tag
  - name: gain
    dtype: float32
    doc: The gain
    required: false
  - name: fixed_label
    dtype: text
    value: fixed
    doc: A fixed label
  - name: channels
    dtype: int32
    shape:
    - null
    default_value:
    - 1
    - 2
    doc: The channels
    required: false
  datasets:
  - name: firmware
    dtype: text
    doc: The firmware version
  - name: temperature
    dtype: float32
    doc: The temperature
    quantity: '?'
  - name: calibration
    dtype: float32
    shape:
    - null
    doc: Calibration values
    quantity: '?'
    attributes:
    - name: unit
      dtype: text
      default_value: volts
      doc: The unit
      required: false
    - name: source
      dtype: text
      doc: The source of the calibration
  groups:
  - neurodata_type_inc: NWBContainer
    doc: Components of the device
    quantity: '*'
  links:
  - name: parent_device
    target_type: Device
    doc: The parent device
- neurodata_type_def: TestSeries
  neurodata_type_inc: TimeSeries
  doc: A test time series
  attributes:
  - name: exposure_time
    dtype: float32
    doc: The exposure time
    required: false
  links:
  - name: device
    target_type: TestDevice
    doc: The device used
    quantity: '?'
- neurodata_type_def: TestTable
  neurodata_type_inc: DynamicTable
  doc: A test table
  datasets:
  - name: value
    neurodata_type_inc: VectorData
    dtype: float32
    doc: The values
"""


def run_generator(tmp_path: Path, extra_args=None) -> Path:
    """Write the test extension, run the generator, and return the output directory."""
    schema_dir = tmp_path / "schema"
    schema_dir.mkdir()
    (schema_dir / "ndx-test.namespace.yaml").write_text(NAMESPACE_YAML)
    (schema_dir / "ndx-test.extensions.yaml").write_text(EXTENSIONS_YAML)
    output_dir = tmp_path / "output"
    parser = argparse.ArgumentParser()
    generator.setup_parser(parser)
    args = parser.parse_args(
        [str(schema_dir / "ndx-test.namespace.yaml"), str(output_dir), "--generate-test-app",
         "--aqnwb-src-dir", str(AQNWB_SRC_DIR)] + (extra_args or []))
    generator.main(args)
    return output_dir


@pytest.fixture(scope="module")
def generated(tmp_path_factory) -> Path:
    return run_generator(tmp_path_factory.mktemp("generated"))


def read(output_dir: Path, type_name: str, ext: str) -> str:
    matches = list(output_dir.rglob(f"{type_name}.{ext}"))
    assert len(matches) == 1, f"Expected one {type_name}.{ext} but found {matches}"
    return matches[0].read_text()


def test_scan_aqnwb_types():
    aqnwb_types = generator.scan_aqnwb_types(AQNWB_SRC_DIR)
    time_series = aqnwb_types[("core", "TimeSeries")]
    assert time_series["cpp_class"] == "AQNWB::NWB::TimeSeries"
    assert time_series["include"] == "nwb/base/TimeSeries.hpp"
    assert time_series["has_spec"]
    assert "withData" in time_series["setters"]
    assert "withUnit" in time_series["setters"]
    assert ("hdmf-common", "DynamicTable") in aqnwb_types
    # Inherited setters are re-exposed in the derived spec
    assert "withData" in aqnwb_types[("core", "ElectricalSeries")]["setters"]


def test_existing_aqnwb_types_are_reused(generated):
    # Types implemented in AqNWB are not generated
    assert not list(generated.rglob("Device.hpp"))
    assert not list(generated.rglob("TimeSeries.hpp"))
    header = read(generated, "TestDevice", "hpp")
    assert '#include "nwb/device/Device.hpp"' in header
    assert "class TestDevice : public AQNWB::NWB::Device" in header
    assert "struct Spec : public AQNWB::NWB::Device::Spec" in header
    # The test app only includes the generated types
    main_cpp = (generated / "test_app" / "main.cpp").read_text()
    assert "NDX_TEST::TestDevice::Spec" in main_cpp
    assert "AQNWB::NWB::Device>" not in main_cpp


def test_spec_fields(generated):
    header = read(generated, "TestDevice", "hpp")
    assert "REGISTER_SPEC(TestDevice, AQNWB::NWB::Device::Spec)" in header
    # Required attribute, optional attribute, and array attribute with a default value
    assert "DEFINE_SPEC_FIELD(Spec, std::string, assetTag, withAssetTag, \"\")" in header
    assert "DEFINE_SPEC_FIELD(Spec, std::optional<float>, gain, withGain, std::nullopt)" in header
    assert "DEFINE_SPEC_FIELD(Spec, std::vector<int32_t>, channels, withChannels, {1, 2})" in header
    # Fixed values are not part of the Spec
    assert "fixedLabel" not in header
    # Scalar text dataset and optional array dataset
    assert "DEFINE_SPEC_FIELD(Spec, std::string, firmware, withFirmware, \"\")" in header
    assert ("DEFINE_SPEC_FIELD(Spec, std::shared_ptr<const AQNWB::IO::BaseArrayDataSetConfig>, "
            "calibration, withCalibration, nullptr)") in header
    # Link
    assert ("DEFINE_SPEC_FIELD(Spec, std::shared_ptr<AQNWB::NWB::Device>, parentDevice, "
            "withParentDevice, nullptr)") in header
    assert "readParentDevice" in header
    # Inherited setters of the AqNWB Device::Spec
    assert "INHERIT_SPEC_SETTER(Spec, AQNWB::NWB::Device::Spec, withDescription)" in header
    assert "INHERIT_SPEC_SETTER(Spec, AQNWB::NWB::Device::Spec, withManufacturer)" in header
    assert "Status initialize(const Spec& spec);" in header


def test_initialize_and_validate(generated):
    impl = read(generated, "TestDevice", "cpp")
    assert "Status status = AQNWB::NWB::Device::Spec::validate();" in impl
    assert "if (assetTag.empty())" in impl
    assert "if (parentDevice == nullptr)" in impl
    assert "Status status = AQNWB::NWB::Device::initialize(spec);" in impl
    assert 'ioPtr->createAttribute(spec.assetTag, m_path, "asset_tag")' in impl
    assert "if (spec.gain.has_value())" in impl
    assert 'ioPtr->createAttribute(fixedLabelValue, m_path, "fixed_label")' in impl
    assert 'spec.channels.data(), m_path, "channels", spec.channels.size()' in impl
    assert 'ioPtr->createStringDataSet(AQNWB::mergePaths(m_path, "firmware"), spec.firmware)' in impl
    # Numeric scalar dataset
    assert ("ioPtr->createScalarDataSet(AQNWB::IO::BaseDataType::F32, &spec.temperature.value(), "
            'AQNWB::mergePaths(m_path, "temperature"))') in impl
    # Required attributes of optional datasets are only required if the dataset is set
    assert "if (calibration != nullptr && calibrationSource.empty())" in impl
    # The attribute of the optional dataset is only created if the dataset is created
    assert re.search(r"if \(spec\.calibration != nullptr\) \{\s*status = status && "
                     r"ioPtr->createAttribute\(spec\.calibrationUnit", impl)
    assert 'ioPtr->createLink(AQNWB::mergePaths("/" + m_path, "parent_device")' in impl
    assert "AQNWB-TODO(child): Optional unnamed NWBContainer children" in impl
    assert '#include "nwb/device/Device.hpp"' in impl


def test_timeseries_subtype(generated):
    header = read(generated, "TestSeries", "hpp")
    assert "struct Spec : public AQNWB::NWB::TimeSeries::Spec" in header
    assert "INHERIT_SPEC_SETTER(Spec, AQNWB::NWB::TimeSeries::Spec, withData)" in header
    assert "std::shared_ptr<NDX_TEST::TestDevice>, device, withDevice, nullptr" in header
    for category in ("nwbfile-helper", "write-helper", "chunking"):
        assert f"AQNWB-TODO({category})" in header
    impl = read(generated, "TestSeries", "cpp")
    # The link is optional and therefore not validated
    assert "device == nullptr" not in impl


def test_dynamic_table_subtype(generated):
    header = read(generated, "TestTable", "hpp")
    assert "struct Spec : public AQNWB::NWB::DynamicTable::Spec" in header
    assert "static std::vector<AQNWB::NWB::DynamicTable::DataSpecPtr> createDefaultDataSpecs();" in header
    assert "validateDataSpecs(" in header
    impl = read(generated, "TestTable", "cpp")
    assert "tableSpec.columns = createDefaultDataSpecs();" in impl
    assert "AQNWB-TODO(table-columns): Add the specs of the required columns: value (VectorData)" in impl
    # Columns are not reported as children
    assert "AQNWB-TODO(child)" not in impl


def test_todo_markers_use_known_categories(generated):
    pattern = re.compile(r"AQNWB-TODO\(([\w-]+)\): .* -- see ")
    found = set()
    for path in generated.rglob("*.?pp"):
        for line in path.read_text().splitlines():
            if "AQNWB-TODO(" in line:
                match = pattern.search(line)
                assert match, f"Malformed AQNWB-TODO in {path}: {line}"
                found.add(match.group(1))
    assert found
    assert found <= set(generator.AQNWB_TODO_REFERENCES)


def test_no_reuse_aqnwb_types(tmp_path):
    output_dir = run_generator(tmp_path, ["--no-reuse-aqnwb-types"])
    assert list(output_dir.rglob("Device.hpp"))
    header = read(output_dir, "TestDevice", "hpp")
    assert "class TestDevice : public CORE::Device" in header
    assert "struct Spec : public CORE::Device::Spec" in header
