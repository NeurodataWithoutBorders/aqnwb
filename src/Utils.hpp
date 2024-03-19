#include <iostream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/** Generate a new uuid string*/
inline std::string generateUuid()
{
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::uuids::to_string(uuid);

  return uuidStr;
}

inline int showError(const char* error)
{
  std::cerr << error << std::endl;
  return -1;
}

inline void checkError(int output)
{
  if (output)
    std::cerr << "Error at " << __FILE__ " " << __LINE__ << std::endl;
}
