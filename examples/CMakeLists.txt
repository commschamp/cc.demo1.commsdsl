
include(GNUInstallDirs)

set (CMAKE_CXX_STANDARD 14)

find_package(LibComms REQUIRED)

find_package(Boost REQUIRED)

set (extra_boost_components)
if (WIN32)
    # For some reason windows build requires extra boost components
    set (extra_boost_components date_time regex)
endif ()

find_package(Boost REQUIRED
    COMPONENTS program_options system ${extra_boost_components})

find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common/include)

set (cc_compile_file "${LibComms_DIR}/CC_Compile.cmake")
include (${cc_compile_file})

set (warn_as_err_opt)
if (NOT DEMO1_NO_WARN_AS_ERR)
    set (warn_as_err_opt WARN_AS_ERR)
endif()

set (ccache_opt)
if (DEMO1_USE_CCACHE)
    set (ccache_opt USE_CCACHE)
endif()

cc_compile(${warn_as_err_opt} ${ccache_opt})
cc_msvc_force_warn_opt("/W4")

add_subdirectory (server)
add_subdirectory (client)
