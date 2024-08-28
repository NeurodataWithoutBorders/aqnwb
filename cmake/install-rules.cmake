if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/aqnwb-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(package aqnwb)

install(
    DIRECTORY
    src/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT aqnwb_Development
)

install(
    TARGETS aqnwb_aqnwb
    EXPORT aqnwbTargets
    RUNTIME #
    COMPONENT aqnwb_Runtime
    LIBRARY #
    COMPONENT aqnwb_Runtime
    NAMELINK_COMPONENT aqnwb_Development
    ARCHIVE #
    COMPONENT aqnwb_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    aqnwb_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE aqnwb_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(aqnwb_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${aqnwb_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT aqnwb_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${aqnwb_INSTALL_CMAKEDIR}"
    COMPONENT aqnwb_Development
)

install(
    EXPORT aqnwbTargets
    NAMESPACE aqnwb::
    DESTINATION "${aqnwb_INSTALL_CMAKEDIR}"
    COMPONENT aqnwb_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()