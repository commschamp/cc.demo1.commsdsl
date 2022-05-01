cmake_minimum_required (VERSION 3.5)
project ("cc.demo1.commsdsl")

option (DEMO1_NO_EXAMPLES "Disable examples compilation." OFF)
option (DEMO1_NO_WARN_AS_ERR "Don't treat compilation warnings as errors." OFF)
option (DEMO1_USE_CCACHE "Use ccache on UNIX." ON)

# Additional variables to be used if needed
# ---------------------------
# PROT_OUTPUT_DIR - Path to protocol output directory. If not provided <build_dir>/output one is used.
# TEST_OUTPUT_DIR - Path to test output directory. If not provided <build_dir>/output_tools one is used.
# TOOLS_OUTPUT_DIR - Path to tools output directory. If not provided <build_dir>/output_tools one is used.
# COMMSDSL_INSTALL_DIR - Path to "commsdsl" installation directory.
# COMMSDSL2COMMS_EXTRA_ARGS - Extra arguments to pass to "commsdsl2comms"
# COMMS_INSTALL_DIR - Directory where external COMMS library installed. If not specified
#    it will be checked out automatically.
# COMMSDSL_TAG - Tag of the commsdsl project to checkout if COMMSDSL2COMMS
#     is not provided.
# COMMS_TAG - Tag of the COMMS library to checkout if COMMS_INSTALL_DIR
#     is not provided and examples are not disabled.

if ("${COMMSDSL_TAG}" STREQUAL "")
    set(COMMSDSL_TAG "split")
endif ()

if ("${COMMS_TAG}" STREQUAL "")
    set(COMMS_TAG "master")
endif ()

set (VERSION "1.0")

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")

set (COMMSDSL_TGT "commsdsl_tgt")
if ("${COMMSDSL_INSTALL_DIR}" STREQUAL "")
        execute_process (
            COMMAND ${CMAKE_COMMAND} -E make_directory "${EXTERNALS_DIR}"
        )   
        
        include(ExternalProject)
        set (commsdsl2comms_main_dir "${PROJECT_BINARY_DIR}/commsdsl")
        set (commsdsl2comms_src_dir "${EXTERNALS_DIR}/commsdsl")
        set (commsdsl2comms_bin_dir "${commsdsl2comms_main_dir}/build")
        set (commsdsl2comms_install_dir "${commsdsl2comms_main_dir}/install")
        
        ExternalProject_Add(
            "${COMMSDSL_TGT}"
            PREFIX "${commsdsl2comms_bin_dir}"
            STAMP_DIR "${commsdsl2comms_bin_dir}"
            GIT_REPOSITORY "https://github.com/commschamp/commsdsl.git"
            GIT_TAG "${COMMSDSL_TAG}"
            SOURCE_DIR "${commsdsl2comms_src_dir}"
            CMAKE_ARGS
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${commsdsl2comms_install_dir}
                -DCOMMSDSL_BUILD_UNIT_TESTS=OFF -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} 
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} 
                -DCOMMSDSL_EXTERNALS_DIR=${EXTERNALS_DIR} 
            BINARY_DIR "${commsdsl2comms_bin_dir}"
        )
        
        set (COMMSDSL2COMMS "${commsdsl2comms_install_dir}/bin/commsdsl2comms")
        set (COMMSDSL2TEST "${commsdsl2comms_install_dir}/bin/commsdsl2test")
        set (COMMSDSL2TOOLS_QT "${commsdsl2comms_install_dir}/bin/commsdsl2tools_qt")
else ()
    find_program(COMMSDSL2COMMS commsdsl2comms PATHS ${COMMSDSL_INSTALL_DIR} REQUIRED)        
    find_program(COMMSDSL2TEST commsdsl2test PATHS ${COMMSDSL_INSTALL_DIR} REQUIRED)        
    find_program(COMMSDSL2TOOLS_QT commsdsl2tools_qt PATHS ${COMMSDSL_INSTALL_DIR} REQUIRED)        
endif ()

set (schema_file "${CMAKE_SOURCE_DIR}/dsl/schema.xml")
set (prot_tmp_dir "${CMAKE_BINARY_DIR}/output.tmp")
set (test_tmp_dir "${CMAKE_BINARY_DIR}/output_test.tmp")
set (tools_tmp_dir "${CMAKE_BINARY_DIR}/output_tools.tmp")

if ("${PROT_OUTPUT_DIR}" STREQUAL "")
    set (PROT_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
endif ()

if ("${TEST_OUTPUT_DIR}" STREQUAL "")
    set (TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_test")
endif ()

if ("${TOOLS_OUTPUT_DIR}" STREQUAL "")
    set (TOOLS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_tools")
endif ()


set (extra_dep)
if (TARGET ${COMMSDSL_TGT})
    set (extra_dep ${COMMSDSL_TGT})
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
    DEPENDS ${schema_file} ${src_files} ${extra_dep} rm_tmp_tgt
    COMMAND ${COMMSDSL2COMMS} ${warn_as_error} -V ${VERSION}
        --comms-tag ${COMMS_TAG} -o ${prot_tmp_dir} 
        ${COMMSDSL2COMMS_EXTRA_ARGS}  ${schema_file}
)

add_custom_command(
    OUTPUT ${test_tmp_dir}
    DEPENDS ${schema_file} ${src_files} ${extra_dep} rm_tmp_tgt
    COMMAND ${COMMSDSL2TEST} ${warn_as_error}
        -o ${test_tmp_dir} ${schema_file}
)

add_custom_command(
    OUTPUT ${tools_tmp_dir}
    DEPENDS ${schema_file} ${src_files} ${extra_dep} rm_tmp_tgt
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
        -DGENERATED="${prot_tmp_dir}" -DOUTPUT="${PROT_OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"
    COMMAND ${CMAKE_COMMAND}
        -DGENERATED="${test_tmp_dir}" -DOUTPUT="${TEST_OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
    COMMAND ${CMAKE_COMMAND}
        -DGENERATED="${tools_tmp_dir}" -DOUTPUT="${TOOLS_OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
    DEPENDS ${prot_tmp_dir} ${test_tmp_dir} ${tools_tmp_dir} ${readme_file} "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

if (DEMO1_NO_EXAMPLES)
    return()
endif()

add_subdirectory (examples)
