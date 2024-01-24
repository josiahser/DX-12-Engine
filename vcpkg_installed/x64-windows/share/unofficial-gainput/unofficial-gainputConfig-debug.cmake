get_filename_component(VCPKG_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../" ABSOLUTE)
#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::gainput::gainput" for configuration "Debug"
set_property(TARGET unofficial::gainput::gainput APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::gainput::gainput PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${VCPKG_IMPORT_PREFIX}/debug/lib/gainput-d.lib"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "Xinput9_1_0;ws2_32"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/gainput-d.dll"
  )

list(APPEND _cmake_import_check_targets unofficial::gainput::gainput )
list(APPEND _cmake_import_check_files_for_unofficial::gainput::gainput "${VCPKG_IMPORT_PREFIX}/debug/lib/gainput-d.lib" "${_IMPORT_PREFIX}/debug/bin/gainput-d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
