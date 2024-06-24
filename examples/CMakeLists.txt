
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

set (extra_opts)
if (DEMO1_WARN_AS_ERR)
    list (APPEND extra_opts WARN_AS_ERR)
endif()

if (DEMO1_USE_CCACHE)
    list (APPEND extra_opts USE_CCACHE)

    if (NOT "${DEMO1_CCACHE_EXECUTABLE}" STREQUAL "")
        list (APPEND extra_opts CCACHE_EXECTUABLE "${DEMO1_CCACHE_EXECUTABLE}")
    endif ()    
endif()

cc_compile(${extra_opts})
cc_msvc_force_warn_opt("/W4")

add_subdirectory (server)
add_subdirectory (client)
