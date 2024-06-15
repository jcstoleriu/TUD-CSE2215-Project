# Install script for directory: D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code

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
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build/code/Debug/assimp-vc143-mtd.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build/code/Release/assimp-vc143-mt.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build/code/MinSizeRel/assimp-vc143-mt.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build/code/RelWithDebInfo/assimp-vc143-mt.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/anim.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/aabb.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/ai_assert.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/camera.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/color4.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/color4.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build/code/../include/assimp/config.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/defs.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Defines.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/cfileio.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/light.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/material.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/material.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/matrix3x3.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/matrix3x3.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/matrix4x4.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/matrix4x4.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/mesh.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/pbrmaterial.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/postprocess.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/quaternion.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/quaternion.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/scene.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/metadata.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/texture.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/types.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/vector2.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/vector2.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/vector3.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/vector3.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/version.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/cimport.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/importerdesc.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Importer.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/DefaultLogger.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/ProgressHandler.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/IOStream.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/IOSystem.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Logger.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/LogStream.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/NullLogger.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/cexport.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Exporter.hpp"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/DefaultIOStream.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/DefaultIOSystem.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/ZipArchiveIOSystem.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/SceneCombiner.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/fast_atof.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/qnan.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/BaseImporter.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Hash.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/MemoryIOWrapper.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/ParsingUtils.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/StreamReader.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/StreamWriter.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/StringComparison.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/StringUtils.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/SGSpatialSort.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/GenericProperty.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/SpatialSort.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/SkeletonMeshBuilder.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/SmoothingGroups.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/SmoothingGroups.inl"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/StandardShapes.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/RemoveComments.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Subdivision.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Vertex.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/LineSplitter.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/TinyFormatter.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Profiler.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/LogAux.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Bitmap.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/XMLTools.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/IOStreamBuffer.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/CreateAnimMesh.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/irrXMLWrapper.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/BlobIOSystem.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/MathFunctions.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Macros.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Exceptional.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/ByteSwapper.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "assimp-dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Compiler/pushpack1.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Compiler/poppack1.h"
    "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src/code/../include/assimp/Compiler/pstdint.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build/code/Debug/assimp-vc143-mtd.pdb")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build/code/RelWithDebInfo/assimp-vc143-mt.pdb")
  endif()
endif()

