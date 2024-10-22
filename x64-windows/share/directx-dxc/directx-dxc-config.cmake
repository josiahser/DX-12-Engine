
get_filename_component(_dxc_root "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_dxc_root "${_dxc_root}" PATH)
get_filename_component(_dxc_root "${_dxc_root}" PATH)

set(_dxc_exe "${_dxc_root}/tools/directx-dxc/dxc.exe")
if (EXISTS "${_dxc_exe}")

   add_library(Microsoft::DirectXShaderCompiler SHARED IMPORTED)
   set_target_properties(Microsoft::DirectXShaderCompiler PROPERTIES
      IMPORTED_LOCATION_RELEASE            "${_dxc_root}/bin/dxcompiler.dll; ${_dxc_root}/bin/dxil.dll"
      IMPORTED_LOCATION_DEBUG              "${_dxc_root}/bin/dxcompiler.dll; ${_dxc_root}/bin/dxil.dll"
      IMPORTED_IMPLIB_RELEASE              "${_dxc_root}/lib/dxcompiler.lib"
      IMPORTED_IMPLIB_DEBUG                "${_dxc_root}/lib/dxcompiler.lib"
      IMPORTED_SONAME_RELEASE              "dxcompiler.lib"
      IMPORTED_SONAME_DEBUG                "dxcompiler.lib"
      INTERFACE_INCLUDE_DIRECTORIES        "${_dxc_root}/include/directx-dxc"
      IMPORTED_CONFIGURATIONS              "Debug;Release"
      IMPORTED_LINK_INTERFACE_LANGUAGES    "C")

    set(directx-dxc_FOUND TRUE)
    set(DIRECTX_DXC_TOOL ${_dxc_exe})

else()

    set(directx-dxc_FOUND FALSE)

endif()

unset(_dxc_exe)
unset(_dxc_root)
