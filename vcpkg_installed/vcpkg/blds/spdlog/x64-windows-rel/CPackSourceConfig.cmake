# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/spdlog/src/v1.13.0-f032863ee1.clean;C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/spdlog/x64-windows-rel")
set(CPACK_CMAKE_GENERATOR "Ninja")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEBIAN_FILE_NAME "spdlog-1.13.0.deb")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Very fast, header-only/compiled, C++ logging library.")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/gabime/spdlog")
set(CPACK_DEBIAN_PACKAGE_NAME "spdlog")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "C:/Users/josia/AppData/Local/vcpkg/downloads/tools/cmake-3.27.1-windows/cmake-3.27.1-windows-i386/share/cmake-3.27/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "spdlog built using CMake")
set(CPACK_GENERATOR "7Z;ZIP")
set(CPACK_GENERATOR "7Z;ZIP")
set(CPACK_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp\$;\\.#;/#")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY "0")
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INSTALLED_DIRECTORIES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/spdlog/src/v1.13.0-f032863ee1.clean;/")
set(CPACK_INSTALL_CMAKE_PROJECTS "")
set(CPACK_INSTALL_PREFIX "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/pkgs/spdlog_x64-windows")
set(CPACK_MODULE_PATH "")
set(CPACK_NSIS_DISPLAY_NAME "spdlog 1.13.0")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_NSIS_PACKAGE_NAME "spdlog 1.13.0")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OUTPUT_CONFIG_FILE "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/spdlog/x64-windows-rel/CPackConfig.cmake")
set(CPACK_PACKAGE_CONTACT "Gabi Melman <gmelman1@gmail.com>")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "C:/Users/josia/AppData/Local/vcpkg/downloads/tools/cmake-3.27.1-windows/cmake-3.27.1-windows-i386/share/cmake-3.27/Templates/CPack.GenericDescription.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast C++ logging library")
set(CPACK_PACKAGE_FILE_NAME "spdlog-1.13.0-Source")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "spdlog 1.13.0")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "spdlog 1.13.0")
set(CPACK_PACKAGE_NAME "spdlog")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "Gabi Melman")
set(CPACK_PACKAGE_VERSION "1.13.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "13")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PROJECT_URL "https://github.com/gabime/spdlog")
set(CPACK_RESOURCE_FILE_LICENSE "C:/Users/josia/AppData/Local/vcpkg/downloads/tools/cmake-3.27.1-windows/cmake-3.27.1-windows-i386/share/cmake-3.27/Templates/CPack.GenericLicense.txt")
set(CPACK_RESOURCE_FILE_README "C:/Users/josia/AppData/Local/vcpkg/downloads/tools/cmake-3.27.1-windows/cmake-3.27.1-windows-i386/share/cmake-3.27/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "C:/Users/josia/AppData/Local/vcpkg/downloads/tools/cmake-3.27.1-windows/cmake-3.27.1-windows-i386/share/cmake-3.27/Templates/CPack.GenericWelcome.txt")
set(CPACK_RPM_FILE_NAME "spdlog-1.13.0.rpm")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Very fast, header-only/compiled, C++ logging library.")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_NAME "spdlog")
set(CPACK_RPM_PACKAGE_SOURCES "ON")
set(CPACK_RPM_PACKAGE_URL "https://github.com/gabime/spdlog")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_7Z "ON")
set(CPACK_SOURCE_GENERATOR "7Z;ZIP")
set(CPACK_SOURCE_IGNORE_FILES "/CVS/;/\\.svn/;/\\.bzr/;/\\.hg/;/\\.git/;\\.swp\$;\\.#;/#")
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/spdlog/src/v1.13.0-f032863ee1.clean;/")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/spdlog/x64-windows-rel/CPackSourceConfig.cmake")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "spdlog-1.13.0-Source")
set(CPACK_SOURCE_TOPLEVEL_TAG "win64-Source")
set(CPACK_SOURCE_ZIP "ON")
set(CPACK_STRIP_FILES "")
set(CPACK_SYSTEM_NAME "win64")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "win64-Source")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "C:/Users/josia/source/repos/DX-12-Engine/vcpkg_installed/vcpkg/blds/spdlog/x64-windows-rel/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
