# Install script for directory: D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/pkgs/opencv4_x86-windows")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/x86-windows-rel/lib/opencv_objdetect4.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libs" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/x86-windows-rel/bin/opencv_objdetect4.dll")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "pdb")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/x86-windows-rel/bin/opencv_objdetect4.pdb")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/aruco_board.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/aruco_detector.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/aruco_dictionary.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/barcode.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/charuco_detector.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/detection_based_tracker.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/face.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/graphical_code_detector.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/objdetect" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/x86-windows/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/modules/objdetect/include/opencv2/objdetect/objdetect.hpp")
endif()

