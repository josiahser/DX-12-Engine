#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::gainput::gainput" for configuration "Release"
set_property(TARGET unofficial::gainput::gainput APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::gainput::gainput PROPERTIES
  IMPORTED_IMPLIB_RELEASE "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/pkgs/gainput_x64-windows/lib/gainput.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "Xinput9_1_0;ws2_32"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/gainput.dll"
  )

list(APPEND _cmake_import_check_targets unofficial::gainput::gainput )
list(APPEND _cmake_import_check_files_for_unofficial::gainput::gainput "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/pkgs/gainput_x64-windows/lib/gainput.lib" "${_IMPORT_PREFIX}/bin/gainput.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
