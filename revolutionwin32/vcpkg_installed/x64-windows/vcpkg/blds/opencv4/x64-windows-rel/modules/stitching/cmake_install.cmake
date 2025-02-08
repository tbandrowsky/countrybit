# Install script for directory: D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/pkgs/opencv4_x64-windows")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "OFF")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/x64-windows-rel/lib/opencv_stitching4.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libs" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/x64-windows-rel/bin/opencv_stitching4.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "pdb")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/x64-windows-rel/bin/opencv_stitching4.pdb")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/autocalib.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/blenders.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/camera.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/exposure_compensate.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/matchers.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/motion_estimators.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/seam_finders.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/timelapsers.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/util.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/util_inl.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/warpers.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching/detail" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/detail/warpers_inl.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/stitching" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x64-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/stitching/include/opencv2/stitching/warpers.hpp")
endif()

