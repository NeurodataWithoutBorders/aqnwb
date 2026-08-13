#include <memory>
#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "Utils.hpp"
#include "io/BaseIO.hpp"
#include "io/hdf5/HDF5IO.hpp"
#include "nwb/NWBFile.hpp"
#include "nwb/file/Subject.hpp"
#include "testUtils.hpp"

using namespace AQNWB;

TEST_CASE("Subject", "[file]")
{
  SECTION("is registered and always uses the canonical subject path")
  {
    auto registry = NWB::RegisteredType::getRegistry();
    REQUIRE(registry.find("core::Subject") != registry.end());

    auto io = createIO("HDF5", getTestFilePath("subject_constructor.nwb"));
    auto subject = NWB::Subject::create("/not/the/subject/path", io);

    REQUIRE(subject->getPath() == "/general/subject");
    REQUIRE(subject->getIO() == io);
  }

  SECTION("writes and reads every metadata field")
  {
    const std::string filename = getTestFilePath("subject_all_fields.nwb");
    auto io = std::make_shared<IO::HDF5::HDF5IO>(filename);
    io->open();
    io->createGroup("/general");

    NWB::Subject::SubjectSpec subjectSpec;
    subjectSpec.age = "P90D";
    subjectSpec.ageReference = "gestational";
    subjectSpec.dateOfBirth = "2024-01-15T00:00:00.000000+00:00";
    subjectSpec.description = "Test subject";
    subjectSpec.genotype = "wt/wt";
    subjectSpec.sex = "M";
    subjectSpec.species = "Mus musculus";
    subjectSpec.strain = "C57BL/6J";
    subjectSpec.subjectId = "subject-001";
    subjectSpec.weight = "25 g";

    auto subject = NWB::Subject::create("/general/subject", io);
    REQUIRE(subject->initialize(subjectSpec) == Status::Success);
    io->close();

    auto readIO = std::make_shared<IO::HDF5::HDF5IO>(filename);
    readIO->open(IO::FileMode::ReadOnly);
    auto readSubject = std::dynamic_pointer_cast<NWB::Subject>(
        NWB::RegisteredType::create("/general/subject", readIO));

    REQUIRE(readSubject != nullptr);
    REQUIRE(readSubject->readAge()->values().data
            == std::vector<std::string> {"P90D"});
    REQUIRE(readSubject->readAgeReference()->values().data
            == std::vector<std::string> {"gestational"});
    REQUIRE(readSubject->readDateOfBirth()->values().data
            == std::vector<std::string> {"2024-01-15T00:00:00.000000+00:00"});
    REQUIRE(readSubject->readDescription()->values().data
            == std::vector<std::string> {"Test subject"});
    REQUIRE(readSubject->readGenotype()->values().data
            == std::vector<std::string> {"wt/wt"});
    REQUIRE(readSubject->readSex()->values().data
            == std::vector<std::string> {"M"});
    REQUIRE(readSubject->readSpecies()->values().data
            == std::vector<std::string> {"Mus musculus"});
    REQUIRE(readSubject->readStrain()->values().data
            == std::vector<std::string> {"C57BL/6J"});
    REQUIRE(readSubject->readSubjectId()->values().data
            == std::vector<std::string> {"subject-001"});
    REQUIRE(readSubject->readWeight()->values().data
            == std::vector<std::string> {"25 g"});

    readIO->close();
  }

  SECTION("uses birth as the default age reference and omits unset fields")
  {
    const std::string filename = getTestFilePath("subject_optional_fields.nwb");
    auto io = std::make_shared<IO::HDF5::HDF5IO>(filename);
    io->open();
    io->createGroup("/general");

    NWB::Subject::SubjectSpec subjectSpec;
    subjectSpec.age = "P7D";
    auto subject = NWB::Subject::create("/general/subject", io);

    REQUIRE(subject->initialize(subjectSpec) == Status::Success);
    REQUIRE(subject->readAgeReference()->values().data
            == std::vector<std::string> {"birth"});
    REQUIRE_FALSE(subject->readSpecies()->exists());
    REQUIRE_FALSE(subject->readDateOfBirth()->exists());

    io->close();
  }

  SECTION("is created by NWBFile initialization when metadata is supplied")
  {
    const std::string filename = getTestFilePath("nwbfile_subject.nwb");
    auto io = std::make_shared<IO::HDF5::HDF5IO>(filename);
    io->open();

    NWB::Subject::SubjectSpec subjectSpec;
    subjectSpec.subjectId = "subject-from-nwbfile";
    auto nwbFile = NWB::NWBFile::create(io);

    REQUIRE(nwbFile->initialize(generateUuid(),
                                "Subject integration test",
                                "Subject test data collection",
                                "",
                                "",
                                std::optional(subjectSpec))
            == Status::Success);
    REQUIRE(io->objectExists("/general/subject"));
    REQUIRE(io->objectExists("/general/subject/subject_id"));

    io->close();
  }
}
