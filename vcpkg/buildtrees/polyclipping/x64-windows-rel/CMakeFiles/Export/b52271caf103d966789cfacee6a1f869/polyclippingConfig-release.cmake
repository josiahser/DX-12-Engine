#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "polyclipping::polyclipping" for configuration "Release"
set_property(TARGET polyclipping::polyclipping APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(polyclipping::polyclipping PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/packages/polyclipping_x64-windows/lib/polyclipping.lib"
  )

list(APPEND _cmake_import_check_targets polyclipping::polyclipping )
list(APPEND _cmake_import_check_files_for_polyclipping::polyclipping "C:/Users/jo_serravalle/source/repos/LearningAttempt2/vcpkg/packages/polyclipping_x64-windows/lib/polyclipping.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
