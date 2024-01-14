#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "polyclipping::polyclipping" for configuration "Debug"
set_property(TARGET polyclipping::polyclipping APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(polyclipping::polyclipping PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/pkgs/polyclipping_x64-windows/debug/lib/polyclipping.lib"
  )

list(APPEND _cmake_import_check_targets polyclipping::polyclipping )
list(APPEND _cmake_import_check_files_for_polyclipping::polyclipping "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/pkgs/polyclipping_x64-windows/debug/lib/polyclipping.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
