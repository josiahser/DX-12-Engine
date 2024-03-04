# Install script for directory: C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/pkgs/directxmath_x64-windows")
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxmath/DirectXMath-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxmath/DirectXMath-targets.cmake"
         "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/x64-windows-rel/CMakeFiles/Export/be317da9b36e3fc86099e24848d0d665/DirectXMath-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxmath/DirectXMath-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/directxmath/DirectXMath-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxmath" TYPE FILE FILES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/x64-windows-rel/CMakeFiles/Export/be317da9b36e3fc86099e24848d0d665/DirectXMath-targets.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/directxmath" TYPE FILE FILES
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXCollision.h"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXCollision.inl"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXColors.h"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXMath.h"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXMathConvert.inl"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXMathMatrix.inl"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXMathMisc.inl"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXMathVector.inl"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXPackedVector.h"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/src/dec2023-00bc4f4dee.clean/Inc/DirectXPackedVector.inl"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/directxmath" TYPE FILE FILES
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/x64-windows-rel/directxmath-config.cmake"
    "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/x64-windows-rel/directxmath-config-version.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/x64-windows-rel/DirectXMath.pc")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/directxmath/x64-windows-rel/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
