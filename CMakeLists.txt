cmake_minimum_required (VERSION 3.5)
project ("cc.demo1.commsdsl")

option (DEMO1_NO_EXAMPLES "Disable examples compilation." OFF)
option (DEMO1_NO_WARN_AS_ERR "Don't treat compilation warnings as errors." OFF)
option (DEMO1_USE_CCACHE "Use ccache on UNIX." ON)
option (DEMO1_BUILD_GEN_PROTOCOL "Build and install generated protocol" ON)
option (DEMO1_BUILD_GEN_DOC "Build documentation for generated protocol" ${DEMO1_BUILD_GEN_PROTOCOL})
option (DEMO1_BUILD_GEN_TEST "Build and install generated test application" OFF)
option (DEMO1_BUILD_GEN_TOOLS "Build and install generated plugin for tools" OFF)

# Additional variables to be used if needed
# ---------------------------
# DEMO1_PROT_OUTPUT_DIR - Path to protocol output directory. If not provided <build_dir>/output one is used.
# DEMO1_TEST_OUTPUT_DIR - Path to test output directory. If not provided <build_dir>/output_tools one is used.
# DEMO1_TOOLS_OUTPUT_DIR - Path to tools output directory. If not provided <build_dir>/output_tools one is used.
# COMMSDSL2COMMS_EXTRA_ARGS - Extra arguments to pass to "commsdsl2comms"
# COMMSDSL2TEST_EXTRA_ARGS - Extra arguments to pass to "commsdsl2test"
# COMMSDSL2TOOLS_QT_EXTRA_ARGS - Extra arguments to pass to "commsdsl2tools_qt"

set (VERSION "2.0.0")

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")

find_program(COMMSDSL2COMMS commsdsl2comms PATH_SUFFIXES bin REQUIRED)        
find_program(COMMSDSL2TEST commsdsl2test PATH_SUFFIXES bin REQUIRED)        
find_program(COMMSDSL2TOOLS_QT commsdsl2tools_qt PATH_SUFFIXES bin REQUIRED)      

set (PROT_OUTPUT_TGT "${PROJECT_NAME}_prot_output_tgt")

set (schema_file "${CMAKE_SOURCE_DIR}/dsl/schema.xml")
set (prot_tmp_dir "${CMAKE_BINARY_DIR}/output.tmp")
set (test_tmp_dir "${CMAKE_BINARY_DIR}/output_test.tmp")
set (tools_tmp_dir "${CMAKE_BINARY_DIR}/output_tools.tmp")

if ("${DEMO1_PROT_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_PROT_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
endif ()

if ("${DEMO1_TEST_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_test")
endif ()

if ("${DEMO1_TOOLS_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_TOOLS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_tools")
endif ()

set (warn_as_error "--warn-as-err")
if (DEMO1_NO_WARN_AS_ERR)
    set (warn_as_error)
endif ()

add_custom_target(rm_tmp_tgt
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${prot_tmp_dir}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${test_tmp_dir}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${tools_tmp_dir}
)

add_custom_command(
    OUTPUT ${prot_tmp_dir}
    DEPENDS ${schema_file} ${src_files} rm_tmp_tgt
    COMMAND ${COMMSDSL2COMMS} ${warn_as_error} -V ${VERSION}
        -o ${prot_tmp_dir} 
        ${COMMSDSL2COMMS_EXTRA_ARGS}  ${schema_file}
)

add_custom_command(
    OUTPUT ${test_tmp_dir}
    DEPENDS ${schema_file} ${src_files} rm_tmp_tgt
    COMMAND ${COMMSDSL2TEST} ${warn_as_error} ${COMMSDSL2TEST_EXTRA_ARGS}
        -o ${test_tmp_dir} ${schema_file}
)

add_custom_command(
    OUTPUT ${tools_tmp_dir}
    DEPENDS ${schema_file} ${src_files} rm_tmp_tgt
    COMMAND ${COMMSDSL2TOOLS_QT} ${warn_as_error} ${COMMSDSL2TOOLS_QT_EXTRA_ARGS}
        -o ${tools_tmp_dir} ${schema_file}
)

set (readme_file "${prot_tmp_dir}/README.md")
add_custom_command(
    OUTPUT ${readme_file}
    DEPENDS ${prot_tmp_dir} 
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/readme.generated ${readme_file}
)

add_custom_target(${PROT_OUTPUT_TGT} ALL
    COMMAND ${CMAKE_COMMAND}
        -DGENERATED="${prot_tmp_dir}" -DOUTPUT="${DEMO1_PROT_OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"
    COMMAND ${CMAKE_COMMAND}
        -DGENERATED="${test_tmp_dir}" -DOUTPUT="${DEMO1_TEST_OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
    COMMAND ${CMAKE_COMMAND}
        -DGENERATED="${tools_tmp_dir}" -DOUTPUT="${DEMO1_TOOLS_OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
    DEPENDS ${prot_tmp_dir} ${test_tmp_dir} ${tools_tmp_dir} ${readme_file} "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

include (ExternalProject)
if (DEMO1_BUILD_GEN_PROTOCOL)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEMO1_PROT_OUTPUT_DIR}
    )  

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E touch ${DEMO1_PROT_OUTPUT_DIR}/dummy.txt
    )

    set (prot_build_dir ${CMAKE_CURRENT_BINARY_DIR}/prot_tgt_build)
    ExternalProject_Add(
        "prot_tgt"
        BUILD_ALWAYS TRUE
        DEPENDS ${PROT_OUTPUT_TGT}
        SOURCE_DIR ${DEMO1_PROT_OUTPUT_DIR}
        BINARY_DIR ${prot_build_dir}
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        CMAKE_ARGS
            -DOPT_REQUIRE_COMMS_LIB=OFF
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    )

    find_package(Doxygen)
    if (DEMO1_BUILD_GEN_DOC AND DOXYGEN_FOUND)
        ExternalProject_Add_Step(
            "prot_tgt" 
            "doc"
            COMMAND ${CMAKE_COMMAND} --build ${prot_build_dir} --target doc_cc_demo1
            DEPENDEES "build"
            DEPENDERS "install"
        )
    endif ()
endif ()  

set (combined_prefix_path ${CMAKE_INSTALL_PREFIX} ${CMAKE_PREFIX_PATH})
string (REPLACE ";" "|" combined_prefix_path_str "${combined_prefix_path}")

if (DEMO1_BUILD_GEN_TEST)
    if (NOT DEMO1_BUILD_GEN_PROTOCOL)
        message (FATAL_ERROR "DEMO1_BUILD_GEN_PROTOCOL is required for DEMO1_BUILD_GEN_TEST")
    endif ()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEMO1_TEST_OUTPUT_DIR}
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E touch ${DEMO1_TEST_OUTPUT_DIR}/dummy.txt
    )

    set (test_build_dir ${CMAKE_CURRENT_BINARY_DIR}/test_tgt_build)
    ExternalProject_Add(
        "test_tgt"
        BUILD_ALWAYS TRUE
        DEPENDS "prot_tgt"
        SOURCE_DIR ${DEMO1_TEST_OUTPUT_DIR}
        BINARY_DIR ${test_build_dir}
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        LIST_SEPARATOR |
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${combined_prefix_path_str}
    )   
endif ()


if (DEMO1_BUILD_GEN_TOOLS)
    if (NOT DEMO1_BUILD_GEN_PROTOCOL)
        message (FATAL_ERROR "DEMO1_BUILD_GEN_PROTOCOL is required for DEMO1_BUILD_GEN_TOOLS")
    endif ()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEMO1_TOOLS_OUTPUT_DIR}
    )    

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E touch ${DEMO1_TOOLS_OUTPUT_DIR}/dummy.txt
    )

    set (tools_build_dir ${CMAKE_CURRENT_BINARY_DIR}/tools_tgt_build)
    ExternalProject_Add(
        "tools_tgt"
        BUILD_ALWAYS TRUE
        DEPENDS "prot_tgt"
        SOURCE_DIR ${DEMO1_TOOLS_OUTPUT_DIR}
        BINARY_DIR ${tools_build_dir}
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_GENERATOR ${CMAKE_GENERATOR}
        LIST_SEPARATOR |
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_PREFIX_PATH=${combined_prefix_path_str}
    )   
endif ()

if (DEMO1_NO_EXAMPLES)
    return()
endif()

add_subdirectory (examples)
