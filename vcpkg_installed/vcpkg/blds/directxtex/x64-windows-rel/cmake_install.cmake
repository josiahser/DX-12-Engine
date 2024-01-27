# Install script for directory: C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/src/dec2023-86b6e9ae7f.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/pkgs/directxtex_x64-windows")
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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/bin/CMake/DirectXTex.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtex/DirectXTex-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtex/DirectXTex-targets.cmake"
         "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/CMakeFiles/Export/b4b8ccd0ed55b608a430a1ab561f6d94/DirectXTex-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtex/DirectXTex-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxtex/DirectXTex-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxtex" TYPE FILE FILES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/CMakeFiles/Export/b4b8ccd0ed55b608a430a1ab561f6d94/DirectXTex-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxtex" TYPE FILE FILES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/CMakeFiles/Export/b4b8ccd0ed55b608a430a1ab561f6d94/DirectXTex-targets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/src/dec2023-86b6e9ae7f.clean/DirectXTex/DirectXTex.h"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/src/dec2023-86b6e9ae7f.clean/DirectXTex/DirectXTex.inl"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxtex" TYPE FILE FILES
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/directxtex-config.cmake"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/directxtex-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/DirectXTex.pc")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxtex/x64-windows-rel/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
