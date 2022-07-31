cmake_minimum_required (VERSION 3.5)
project ("cc.demo1.commsdsl")

option (DEMO1_NO_EXAMPLES "Disable examples compilation." OFF)
option (DEMO1_NO_WARN_AS_ERR "Don't treat compilation warnings as errors." OFF)
option (DEMO1_USE_CCACHE "Use ccache on UNIX." ON)

# Additional variables to be used if needed
# ---------------------------
# DEMO1_PROT_OUTPUT_DIR - Path to protocol output directory. If not provided <build_dir>/output one is used.
# DEMO1_TEST_OUTPUT_DIR - Path to test output directory. If not provided <build_dir>/output_tools one is used.
# DEMO1_TOOLS_OUTPUT_DIR - Path to tools output directory. If not provided <build_dir>/output_tools one is used.
# COMMSDSL_INSTALL_DIR - Path to "commsdsl" installation directory.
# COMMSDSL2COMMS_EXTRA_ARGS - Extra arguments to pass to "commsdsl2comms"

set (VERSION "1.1.0")

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")

find_program(COMMSDSL2COMMS commsdsl2comms PATH_SUFFIXES bin REQUIRED)        
find_program(COMMSDSL2TEST commsdsl2test PATH_SUFFIXES bin REQUIRED)        
find_program(COMMSDSL2TOOLS_QT commsdsl2tools_qt PATH_SUFFIXES bin REQUIRED)      

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
    COMMAND ${COMMSDSL2TEST} ${warn_as_error}
        -o ${test_tmp_dir} ${schema_file}
)

add_custom_command(
    OUTPUT ${tools_tmp_dir}
    DEPENDS ${schema_file} ${src_files} rm_tmp_tgt
    COMMAND ${COMMSDSL2TOOLS_QT} ${warn_as_error}
        -o ${tools_tmp_dir} ${schema_file}
)

set (readme_file "${prot_tmp_dir}/README.md")
add_custom_command(
    OUTPUT ${readme_file}
    DEPENDS ${prot_tmp_dir} 
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/readme.generated ${readme_file}
)

set (prot_output_tgt ${name}_prot_output_tgt)
add_custom_target(${PROJECT_NAME} ALL
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

if (DEMO1_NO_EXAMPLES)
    return()
endif()

add_subdirectory (examples)
