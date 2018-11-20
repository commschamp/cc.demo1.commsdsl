cmake_minimum_required (VERSION 3.5)
project ("cc.demo1.commsdsl")

option (DEMO1_NO_EXAMPLES "Disable examples compilation." OFF)
option (DEMO1_NO_WARN_AS_ERR "Don't treat compilation warnings as errors." OFF)

# Additional variables to be used if needed
# ---------------------------
# OUTPUT_DIR - Path to output directory. If not provided <build_dir>/output one is used.
# COMMSDSL2COMMS - Path to "commsdsl2comms" binary.
# COMMSDSL2COMMS_EXTRA_ARGS - Extra arguments to pass to "commsdsl2comms"
# COMMS_INSTALL_DIR - Directory where external COMMS library installed. If not specified
#    it will be checked out automatically.
# COMMS_TAG - Tag of the COMMS library to checkout (defaults to "master").

set (COMMSDSL_TGT "commsdsl_tgt")
if ("${COMMSDSL2COMMS}" STREQUAL "")
        include(ExternalProject)
        set (commsdsl2comms_tag "develop")
        set (commsdsl2comms_main_dir "${CMAKE_BINARY_DIR}/commsdsl")
        set (commsdsl2comms_src_dir "${commsdsl2comms_main_dir}/src")
        set (commsdsl2comms_bin_dir "${commsdsl2comms_main_dir}/build")
        set (commsdsl2comms_install_dir "${commsdsl2comms_main_dir}/install")
        
        set (boost_static_libs_param)
        if (Boost_USE_STATIC_LIBS)
        set (boost_static_libs_param "-DBoost_USE_STATIC_LIBS=ON")
        endif()

        ExternalProject_Add(
            "${COMMSDSL_TGT}"
            PREFIX "${commsdsl2comms_bin_dir}"
            STAMP_DIR "${commsdsl2comms_bin_dir}"
            GIT_REPOSITORY "https://github.com/arobenko/commsdsl.git"
            GIT_TAG "${commsdsl2comms_tag}"
            SOURCE_DIR "${commsdsl2comms_src_dir}"
            CMAKE_ARGS
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${commsdsl2comms_install_dir}
                -DCOMMSDSL_NO_TESTS=ON -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} 
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DBOOST_ROOT=${BOOST_ROOT}
                ${boost_static_libs_param}
            BINARY_DIR "${commsdsl2comms_bin_dir}"
        )
        
        set (COMMSDSL2COMMS "${commsdsl2comms_install_dir}/bin/commsdsl2comms")
        
        if (UNIX)
            set (COMMSDSL2COMMS "${COMMSDSL2COMMS}.sh")
        endif ()
endif ()

set (schema_file "${CMAKE_SOURCE_DIR}/dsl/schema.xml")
set (tmp_dir "${CMAKE_BINARY_DIR}/output.tmp")
#set (code_input_dir "${CMAKE_SOURCE_DIR}/src")

if ("${OUTPUT_DIR}" STREQUAL "")
    set (OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
endif ()


#file (GLOB_RECURSE src_files ${code_input_dir})

set (extra_dep)
if (TARGET ${COMMSDSL_TGT})
    set (extra_dep ${COMMSDSL_TGT})
endif ()

set (warn_as_error "--warn-as-err")
if (DEMO1_NO_WARN_AS_ERR)
    set (warn_as_error)
endif ()

add_custom_target(rm_tmp_tgt
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_dir}
)

add_custom_command(
    OUTPUT ${tmp_dir}
    DEPENDS ${schema_file} ${src_files} ${extra_dep} rm_tmp_tgt
    COMMAND ${COMMSDSL2COMMS} ${warn_as_error} -o ${tmp_dir} 
        ${COMMSDSL2COMMS_EXTRA_ARGS}  ${schema_file}
)

set (readme_file "${tmp_dir}/README.md")
add_custom_command(
    OUTPUT ${readme_file}
    DEPENDS ${tmp_dir} 
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/readme.generated ${readme_file}
)

set (output_tgt ${name}_output_tgt)
add_custom_target(${PROJECT_NAME} ALL
    COMMAND ${CMAKE_COMMAND}
        -DGENERATED="${tmp_dir}" -DOUTPUT="${OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"
    DEPENDS ${tmp_dir} ${readme_file} "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

if (DEMO1_NO_EXAMPLES)
    return()
endif()

add_subdirectory (examples)
