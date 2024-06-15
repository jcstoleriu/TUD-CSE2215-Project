# Install script for directory: D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/ComputerGraphics")
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
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/gsl-lite/gsl-lite-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/gsl-lite/gsl-lite-targets.cmake"
         "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-build/CMakeFiles/Export/95ba0980806bdd2d3ecb26c82a76a35f/gsl-lite-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/gsl-lite/gsl-lite-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/gsl-lite/gsl-lite-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/gsl-lite" TYPE FILE FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-build/CMakeFiles/Export/95ba0980806bdd2d3ecb26c82a76a35f/gsl-lite-targets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/gsl-lite" TYPE FILE FILES
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-build/gsl-lite-config.cmake"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-build/gsl-lite-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gsl" TYPE FILE FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-src/include/gsl/gsl-lite.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/gsl-lite" TYPE FILE FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-src/include/gsl-lite/gsl-lite.hpp")
endif()

