# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-src"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-build"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-subbuild/assimp-populate-prefix"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-subbuild/assimp-populate-prefix/tmp"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-subbuild/assimp-populate-prefix/src/assimp-populate-stamp"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-subbuild/assimp-populate-prefix/src"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-subbuild/assimp-populate-prefix/src/assimp-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-subbuild/assimp-populate-prefix/src/assimp-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/assimp-subbuild/assimp-populate-prefix/src/assimp-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
