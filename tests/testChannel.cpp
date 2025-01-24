#include <catch2/catch_approx.hpp>

#include "Channel.hpp"
#include "testUtils.hpp"

TEST_CASE("Test Channel Construction and Basic Operations", "[channel]")
{
  SECTION("Default constructor parameters")
  {
    Channel ch("test_channel", "test_group", 1, 2, 3);
    REQUIRE(ch.getName() == "test_channel");
    REQUIRE(ch.getGroupName() == "test_group");
    REQUIRE(ch.getGroupIndex() == 1);
    REQUIRE(ch.getLocalIndex() == 2);
    REQUIRE(ch.getGlobalIndex() == 3);
    REQUIRE(ch.getConversion()
            == Catch::Approx(0.05e-6f).epsilon(
                0.001));  // bitVolts/conversion = 0.05/1e6
    REQUIRE(ch.getSamplingRate() == Catch::Approx(30000.f).epsilon(0.001));
    REQUIRE(ch.getBitVolts() == Catch::Approx(0.05f).epsilon(0.001));
    const auto& pos = ch.getPosition();
    REQUIRE(pos[0] == Catch::Approx(0.f).epsilon(0.001));
    REQUIRE(pos[1] == Catch::Approx(0.f).epsilon(0.001));
    REQUIRE(pos[2] == Catch::Approx(0.f).epsilon(0.001));
    REQUIRE(ch.getComments() == "no comments");
  }

  SECTION("Custom constructor parameters")
  {
    std::array<float, 3> pos = {1.f, 2.f, 3.f};
    Channel ch("test_channel",
               "test_group",
               1,
               2,
               3,
               2e6f,  // custom conversion
               44100.f,  // custom sampling rate
               0.1f,  // custom bitVolts
               pos,  // custom position
               "test comment"  // custom comment
    );

    REQUIRE(ch.getName() == "test_channel");
    REQUIRE(ch.getGroupName() == "test_group");
    REQUIRE(ch.getGroupIndex() == 1);
    REQUIRE(ch.getLocalIndex() == 2);
    REQUIRE(ch.getGlobalIndex() == 3);
    REQUIRE(ch.getConversion() == Catch::Approx(0.1f / 2e6f).epsilon(0.001));
    REQUIRE(ch.getSamplingRate() == Catch::Approx(44100.f).epsilon(0.001));
    REQUIRE(ch.getBitVolts() == Catch::Approx(0.1f).epsilon(0.001));
    const auto& actualPos = ch.getPosition();
    REQUIRE(actualPos[0] == Catch::Approx(pos[0]).epsilon(0.001));
    REQUIRE(actualPos[1] == Catch::Approx(pos[1]).epsilon(0.001));
    REQUIRE(actualPos[2] == Catch::Approx(pos[2]).epsilon(0.001));
    REQUIRE(ch.getComments() == "test comment");
  }
}

TEST_CASE("Test Channel Setters", "[channel]")
{
  Channel ch("test_channel", "test_group", 1, 2, 3);

  SECTION("Set comments")
  {
    ch.setComments("new comment");
    REQUIRE(ch.getComments() == "new comment");
  }

  SECTION("Set position")
  {
    std::array<float, 3> newPos = {4.f, 5.f, 6.f};
    ch.setPosition(newPos);
    const auto& actualPos = ch.getPosition();
    REQUIRE(actualPos[0] == Catch::Approx(newPos[0]).epsilon(0.001));
    REQUIRE(actualPos[1] == Catch::Approx(newPos[1]).epsilon(0.001));
    REQUIRE(actualPos[2] == Catch::Approx(newPos[2]).epsilon(0.001));
  }

  SECTION("Set name")
  {
    ch.setName("new_channel");
    REQUIRE(ch.getName() == "new_channel");
  }
}

TEST_CASE("Test Channel Copy and Move Operations", "[channel]")
{
  std::array<float, 3> pos = {1.f, 2.f, 3.f};
  Channel original("test_channel",
                   "test_group",
                   1,
                   2,
                   3,
                   2e6f,
                   44100.f,
                   0.1f,
                   pos,
                   "test comment");

  SECTION("Copy constructor")
  {
    Channel copy(original);
    REQUIRE(copy.getName() == original.getName());
    REQUIRE(copy.getGroupName() == original.getGroupName());
    REQUIRE(copy.getGroupIndex() == original.getGroupIndex());
    REQUIRE(copy.getLocalIndex() == original.getLocalIndex());
    REQUIRE(copy.getGlobalIndex() == original.getGlobalIndex());
    REQUIRE(copy.getConversion()
            == Catch::Approx(original.getConversion()).epsilon(0.001));
    REQUIRE(copy.getSamplingRate()
            == Catch::Approx(original.getSamplingRate()).epsilon(0.001));
    REQUIRE(copy.getBitVolts()
            == Catch::Approx(original.getBitVolts()).epsilon(0.001));
    const auto& origPos = original.getPosition();
    const auto& copyPos = copy.getPosition();
    REQUIRE(copyPos[0] == Catch::Approx(origPos[0]).epsilon(0.001));
    REQUIRE(copyPos[1] == Catch::Approx(origPos[1]).epsilon(0.001));
    REQUIRE(copyPos[2] == Catch::Approx(origPos[2]).epsilon(0.001));
    REQUIRE(copy.getComments() == original.getComments());
  }

  SECTION("Copy assignment")
  {
    Channel copy("other", "other_group", 0, 0, 0);
    copy = original;
    REQUIRE(copy.getName() == original.getName());
    REQUIRE(copy.getGroupName() == original.getGroupName());
    REQUIRE(copy.getGroupIndex() == original.getGroupIndex());
    REQUIRE(copy.getLocalIndex() == original.getLocalIndex());
    REQUIRE(copy.getGlobalIndex() == original.getGlobalIndex());
    REQUIRE(copy.getConversion()
            == Catch::Approx(original.getConversion()).epsilon(0.001));
    REQUIRE(copy.getSamplingRate()
            == Catch::Approx(original.getSamplingRate()).epsilon(0.001));
    REQUIRE(copy.getBitVolts()
            == Catch::Approx(original.getBitVolts()).epsilon(0.001));
    const auto& origPos = original.getPosition();
    const auto& copyPos = copy.getPosition();
    REQUIRE(copyPos[0] == Catch::Approx(origPos[0]).epsilon(0.001));
    REQUIRE(copyPos[1] == Catch::Approx(origPos[1]).epsilon(0.001));
    REQUIRE(copyPos[2] == Catch::Approx(origPos[2]).epsilon(0.001));
    REQUIRE(copy.getComments() == original.getComments());
  }

  SECTION("Move constructor")
  {
    Channel moved(std::move(Channel(original)));
    REQUIRE(moved.getName() == original.getName());
    REQUIRE(moved.getGroupName() == original.getGroupName());
    REQUIRE(moved.getGroupIndex() == original.getGroupIndex());
    REQUIRE(moved.getLocalIndex() == original.getLocalIndex());
    REQUIRE(moved.getGlobalIndex() == original.getGlobalIndex());
    REQUIRE(moved.getConversion()
            == Catch::Approx(original.getConversion()).epsilon(0.001));
    REQUIRE(moved.getSamplingRate()
            == Catch::Approx(original.getSamplingRate()).epsilon(0.001));
    REQUIRE(moved.getBitVolts()
            == Catch::Approx(original.getBitVolts()).epsilon(0.001));
    const auto& origPos = original.getPosition();
    const auto& movedPos = moved.getPosition();
    REQUIRE(movedPos[0] == Catch::Approx(origPos[0]).epsilon(0.001));
    REQUIRE(movedPos[1] == Catch::Approx(origPos[1]).epsilon(0.001));
    REQUIRE(movedPos[2] == Catch::Approx(origPos[2]).epsilon(0.001));
    REQUIRE(moved.getComments() == original.getComments());
  }

  SECTION("Move assignment")
  {
    Channel moved("other", "other_group", 0, 0, 0);
    moved = std::move(Channel(original));
    REQUIRE(moved.getName() == original.getName());
    REQUIRE(moved.getGroupName() == original.getGroupName());
    REQUIRE(moved.getGroupIndex() == original.getGroupIndex());
    REQUIRE(moved.getLocalIndex() == original.getLocalIndex());
    REQUIRE(moved.getGlobalIndex() == original.getGlobalIndex());
    REQUIRE(moved.getConversion()
            == Catch::Approx(original.getConversion()).epsilon(0.001));
    REQUIRE(moved.getSamplingRate()
            == Catch::Approx(original.getSamplingRate()).epsilon(0.001));
    REQUIRE(moved.getBitVolts()
            == Catch::Approx(original.getBitVolts()).epsilon(0.001));
    const auto& origPos = original.getPosition();
    const auto& movedPos = moved.getPosition();
    REQUIRE(movedPos[0] == Catch::Approx(origPos[0]).epsilon(0.001));
    REQUIRE(movedPos[1] == Catch::Approx(origPos[1]).epsilon(0.001));
    REQUIRE(movedPos[2] == Catch::Approx(origPos[2]).epsilon(0.001));
    REQUIRE(moved.getComments() == original.getComments());
  }
}
