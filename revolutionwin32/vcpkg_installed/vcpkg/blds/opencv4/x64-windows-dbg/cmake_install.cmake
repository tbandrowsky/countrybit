# Install script for directory: D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/countrybit/revolution/vcpkg_installed/vcpkg/pkgs/opencv4_x64-windows/debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "licenses" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/opencv/licenses" TYPE FILE RENAME "opencl-headers-LICENSE.txt" FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/3rdparty/include/opencl/LICENSE.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2" TYPE FILE FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/cvconfig.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2" TYPE FILE FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/opencv2/opencv_modules.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/opencv4/OpenCVModules.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/opencv4/OpenCVModules.cmake"
         "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/CMakeFiles/Export/282d975f1a324973dbe4816f14a34c1b/OpenCVModules.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/opencv4/OpenCVModules-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/opencv4/OpenCVModules.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/opencv4" TYPE FILE FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/CMakeFiles/Export/282d975f1a324973dbe4816f14a34c1b/OpenCVModules.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/opencv4" TYPE FILE FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/CMakeFiles/Export/282d975f1a324973dbe4816f14a34c1b/OpenCVModules-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/opencv4" TYPE FILE FILES
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/unix-install/OpenCVConfig-version.cmake"
    "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/unix-install/OpenCVConfig.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "libs" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/src/4.8.0-2bf495557d.clean/LICENSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/include/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/calib3d/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/core/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/dnn/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/features2d/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/flann/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/gapi/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/highgui/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/imgcodecs/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/imgproc/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/java/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/js/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/ml/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/objc/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/objdetect/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/photo/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/python/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/stitching/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/ts/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/video/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/videoio/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/.firstpass/world/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/core/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/flann/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/imgproc/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/ml/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/photo/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/python_tests/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/dnn/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/features2d/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/imgcodecs/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/videoio/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/calib3d/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/highgui/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/objdetect/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/stitching/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/video/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/java_bindings_generator/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/js_bindings_generator/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/objc_bindings_generator/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/modules/python_bindings_generator/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/doc/cmake_install.cmake")
  include("D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/data/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/countrybit/revolution/vcpkg_installed/vcpkg/blds/opencv4/x64-windows-dbg/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
