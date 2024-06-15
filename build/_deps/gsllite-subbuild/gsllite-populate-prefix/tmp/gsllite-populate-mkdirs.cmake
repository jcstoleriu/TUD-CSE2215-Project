# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-src"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-build"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-subbuild/gsllite-populate-prefix"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-subbuild/gsllite-populate-prefix/tmp"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-subbuild/gsllite-populate-prefix/src/gsllite-populate-stamp"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-subbuild/gsllite-populate-prefix/src"
  "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-subbuild/gsllite-populate-prefix/src/gsllite-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-subbuild/gsllite-populate-prefix/src/gsllite-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Jasmine/Master year1/q3 3d Computer Graphics and Animation/TUD-CSE2215-Project/build/_deps/gsllite-subbuild/gsllite-populate-prefix/src/gsllite-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
