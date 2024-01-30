# Install script for directory: C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/packages/poly2tri_x64-windows/debug")
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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/poly2tri/common" TYPE FILE FILES
    "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean/poly2tri/common/shapes.h"
    "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean/poly2tri/common/utils.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/poly2tri/sweep" TYPE FILE FILES
    "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean/poly2tri/sweep/advancing_front.h"
    "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean/poly2tri/sweep/cdt.h"
    "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean/poly2tri/sweep/sweep.h"
    "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean/poly2tri/sweep/sweep_context.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/x64-windows-dbg/poly2tri.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/poly2tri" TYPE FILE FILES "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/src/bd85c3c204-6194be63e9.clean/poly2tri/poly2tri.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/poly2tri/poly2triConfig.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/poly2tri/poly2triConfig.cmake"
         "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/x64-windows-dbg/CMakeFiles/Export/3084596002d37368b2a5a32bdd67cc30/poly2triConfig.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/poly2tri/poly2triConfig-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/poly2tri/poly2triConfig.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/poly2tri" TYPE FILE FILES "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/x64-windows-dbg/CMakeFiles/Export/3084596002d37368b2a5a32bdd67cc30/poly2triConfig.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/poly2tri" TYPE FILE FILES "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/x64-windows-dbg/CMakeFiles/Export/3084596002d37368b2a5a32bdd67cc30/poly2triConfig-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/buildtrees/poly2tri/x64-windows-dbg/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
