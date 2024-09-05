#pragma once

#include <memory>
#include <string>

#include "io/BaseIO.hpp"

namespace AQNWB::NWB
{
/**
 * @brief Abstract data type for a group storing collections of data and
 * metadata
 */
class Container
{
public:
  /**
   * @brief Constructor.
   * @param path The path of the container.
   * @param io A shared pointer to the IO object.
   */
  Container(const std::string& path, std::shared_ptr<IO::BaseIO> io);

  /**
   * @brief Destructor.
   */
  virtual ~Container();

  /**
   * @brief Initialize the container.
   */
  void initialize();

  /**
   * @brief Gets the path of the container.
   * @return The path of the container.
   */
  std::string getPath() const;

  template<typename T>
  inline static std::unique_ptr<T> create(const std::string& path,
                                          std::shared_ptr<IO::BaseIO> io)
  {
    static_assert(std::is_base_of<Container, T>::value,
                  "T must be a derived class of Container");
    return std::unique_ptr<T>(new T(path, io));
  }

protected:
  /**
   * @brief The path of the container.
   */
  std::string path;

  /**
   * @brief A shared pointer to the IO object.
   */
  std::shared_ptr<IO::BaseIO> io;
};
}  // namespace AQNWB::NWB
