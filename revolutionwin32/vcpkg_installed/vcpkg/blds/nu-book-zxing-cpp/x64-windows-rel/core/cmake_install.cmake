# Install script for directory: D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/countrybit/revolution/vcpkg_installed/vcpkg/pkgs/nu-book-zxing-cpp_x64-windows")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/ZXing.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ZXing" TYPE FILE FILES
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/BarcodeFormat.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/BitHacks.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/ByteArray.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/CharacterSet.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Flags.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/GTIN.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/TextUtfEncoding.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/ZXAlgorithms.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/ZXConfig.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Content.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/DecodeHints.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Error.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/ImageView.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Point.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Quadrilateral.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/ReadBarcode.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/ReaderOptions.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Result.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/StructuredAppend.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/BitMatrix.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/BitMatrixIO.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Matrix.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/MultiFormatWriter.h"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/src/v2.2.1-e06c450fca.clean/core/src/Range.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/ZXing" TYPE FILE FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/ZXVersion.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg]|[Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE OPTIONAL FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/ZXing.pdb")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets.cmake"
         "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/CMakeFiles/Export/f9e04a807b27a41299a115186893fdf1/ZXingTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing/ZXingTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing" TYPE FILE FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/CMakeFiles/Export/f9e04a807b27a41299a115186893fdf1/ZXingTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing" TYPE FILE FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/CMakeFiles/Export/f9e04a807b27a41299a115186893fdf1/ZXingTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ZXing" TYPE FILE FILES
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/ZXingConfig.cmake"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/nu-book-zxing-cpp/x64-windows-rel/core/ZXingConfigVersion.cmake"
    )
endif()

