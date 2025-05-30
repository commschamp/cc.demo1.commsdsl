cmake_minimum_required (VERSION 3.10)
project ("cc.demo1.commsdsl")

option (DEMO1_WARN_AS_ERR "Treat compilation warnings as errors." ON)
option (DEMO1_USE_CCACHE "Use ccache." OFF)
option (DEMO1_GEN_PROTOCOL "Use commsdsl2comms to generate protocol definition" ON)
option (DEMO1_BUILD_PROTOCOL "Build and install protocol definition generated by the commsdsl2comms" ${DEMO1_GEN_PROTOCOL})
option (DEMO1_BUILD_PROT_DOC "Build documentation for generated protocol" OFF)
option (DEMO1_GEN_TEST "Use commsdsl2test to generate test project." OFF)
option (DEMO1_BUILD_TEST "Build and install project generated by the commsdsl2test." ${DEMO1_GEN_TEST})
option (DEMO1_GEN_TOOLS "Use commsdsl2tools_qt to generate cc_tools_qt plugin project." OFF)
option (DEMO1_BUILD_TOOLS "Build and install project generated by the commsdsl2tools_qt." ${DEMO1_GEN_TOOLS})
option (DEMO1_GEN_SWIG "Use commsdsl2swig to generate swig bindings project." OFF)
option (DEMO1_BUILD_SWIG "Build and install project generated by the commsdsl2swig." ${DEMO1_GEN_SWIG})
option (DEMO1_GEN_EMSCRIPTEN "Use commsdsl2emscripten to generate emscripten bindings project." OFF)
option (DEMO1_BUILD_EMSCRIPTEN "Build and install project generated by the commsdsl2emscripten." ${DEMO1_GEN_EMSCRIPTEN})
option (DEMO1_BUILD_EXAMPLES "Build examples." ON)

# Additional variables to be used if needed
# ---------------------------
# DEMO1_PROT_OUTPUT_DIR - Path to protocol output directory. If not provided <build_dir>/output one is used.
# DEMO1_TEST_OUTPUT_DIR - Path to test output directory. If not provided <build_dir>/output_tools one is used.
# DEMO1_TOOLS_OUTPUT_DIR - Path to tools output directory. If not provided <build_dir>/output_tools one is used.
# DEMO1_TOOLS_QT_VER - Major Qt version to be used when building tools output. Defaults to 5
# DEMO1_SWIG_OUTPUT_DIR - Path to swig output directory. If not provided <build_dir>/output_swig one is used.
# DEMO1_SWIG_SRC_DIR - Path to the commsdsl2swig sources. If not provided local "src/swig" dir is used.
# DEMO1_SWIG_LANGUAGES - Languages list to support by swig. If not provided "python java csharp" are chosen
# DEMO1_EMSCRIPTEN_OUTPUT_DIR - Path to emscripten output directory. If not provided <build_dir>/output_emscripten one is used.
# DEMO1_CCACHE_EXECUTABLE - Custom ccache executable
# COMMSDSL2COMMS_EXTRA_ARGS - Extra arguments to pass to "commsdsl2comms"
# COMMSDSL2TEST_EXTRA_ARGS - Extra arguments to pass to "commsdsl2test"
# COMMSDSL2TOOLS_QT_EXTRA_ARGS - Extra arguments to pass to "commsdsl2tools_qt"
# COMMSDSL2SWIG_EXTRA_ARGS - Extra arguments to pass to "commsdsl2swig"
# COMMSDSL2EMSCRIPTEN_EXTRA_ARGS - Extra arguments to pass to "commsdsl2emscripten"

set (DEMO1_VERSION "3.0.3")
set (DEMO1_MIN_COMMSDSL_VERSION "7.1")

if (("${CMAKE_VERSION}" VERSION_GREATER_EQUAL "3.30") AND
    (NOT DEFINED CMAKE_POLICY_DEFAULT_CMP0167))
    # Find boost cmake configuration from the boost installation
    cmake_policy(SET CMP0167 NEW)
endif ()

if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
    set(CMAKE_CXX_STANDARD 17)
endif ()

set (EXTERNALS_DIR "${PROJECT_SOURCE_DIR}/externals")

set (PROT_OUTPUT_TGT "prot_output_tgt")

set (schema_file "${CMAKE_SOURCE_DIR}/dsl/schema.xml")

if ("${DEMO1_PROT_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_PROT_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
endif ()

if ("${DEMO1_TEST_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_test")
endif ()

if ("${DEMO1_TOOLS_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_TOOLS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_tools")
endif ()

if ("${DEMO1_SWIG_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_SWIG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_swig")
endif ()

if ("${DEMO1_SWIG_SRC_DIR}" STREQUAL "")
    set (DEMO1_SWIG_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/swig)
endif ()

if ("${DEMO1_SWIG_LANGUAGES}" STREQUAL "")
    set (DEMO1_SWIG_LANGUAGES python java csharp)
endif ()

if ("${DEMO1_EMSCRIPTEN_OUTPUT_DIR}" STREQUAL "")
    set (DEMO1_EMSCRIPTEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output_emscripten")
endif ()

if ("${DEMO1_TOOLS_QT_VER}" STREQUAL "")
    set (DEMO1_TOOLS_QT_VER 5)
endif ()

set (warn_as_error "--warn-as-err")
if (NOT DEMO1_WARN_AS_ERR)
    set (warn_as_error)
endif ()

include (ExternalProject)
if (DEMO1_GEN_PROTOCOL)
    find_program(COMMSDSL2COMMS commsdsl2comms PATH_SUFFIXES bin REQUIRED)        
    execute_process(COMMAND "${COMMSDSL2COMMS}" --version OUTPUT_VARIABLE commsdsl2comms_version)
    if ("${commsdsl2comms_version}" VERSION_LESS "${DEMO1_MIN_COMMSDSL_VERSION}")
        message (FATAL_ERROR "Version of commsdsl2comms is too old, required minimum is v${DEMO1_MIN_COMMSDSL_VERSION}.")
    endif ()

    set (prot_tmp_dir "${CMAKE_BINARY_DIR}/output.tmp")

    add_custom_target(${PROT_OUTPUT_TGT} ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${prot_tmp_dir}
        COMMAND ${COMMSDSL2COMMS} ${warn_as_error} -V ${DEMO1_VERSION}
            -o ${prot_tmp_dir} -c ${PROJECT_SOURCE_DIR}/src/protocol
            ${COMMSDSL2COMMS_EXTRA_ARGS}  ${schema_file}        
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${prot_tmp_dir}" -DOUTPUT="${DEMO1_PROT_OUTPUT_DIR}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

    if (DEMO1_BUILD_PROTOCOL)
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
            CMAKE_ARGS
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_GENERATOR=${CMAKE_GENERATOR} -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
                -DCMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}           
                -DOPT_REQUIRE_COMMS_LIB=OFF -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        )

        find_package(Doxygen)
        if (DEMO1_BUILD_PROT_DOC AND DOXYGEN_FOUND)
            ExternalProject_Add_Step(
                "prot_tgt" 
                "doc"
                COMMAND ${CMAKE_COMMAND} --build ${prot_build_dir} --target doc_cc_demo1
                DEPENDEES "build"
                DEPENDERS "install"
            )
        endif ()
    endif ()
endif ()  

set (combined_prefix_path ${CMAKE_INSTALL_PREFIX} ${CMAKE_PREFIX_PATH})
string (REPLACE ";" "|" combined_prefix_path_str "${combined_prefix_path}")

if (DEMO1_GEN_TEST)
    find_program(COMMSDSL2TEST commsdsl2test PATH_SUFFIXES bin REQUIRED)        
    execute_process(COMMAND "${COMMSDSL2TEST}" --version OUTPUT_VARIABLE commsdsl2test_version)
    if ("${commsdsl2test_version}" VERSION_LESS "${DEMO1_MIN_COMMSDSL_VERSION}")
        message (FATAL_ERROR "Version of commsdsl2test is too old, required minimum is v${DEMO1_MIN_COMMSDSL_VERSION}.")
    endif ()    

    set (test_tmp_dir "${CMAKE_BINARY_DIR}/output_test.tmp")

    add_custom_target("test_output_tgt" ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${test_tmp_dir}
        COMMAND ${COMMSDSL2TEST} ${warn_as_error} ${COMMSDSL2TEST_EXTRA_ARGS}
            -o ${test_tmp_dir} -c ${PROJECT_SOURCE_DIR}/src/test ${schema_file}
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${test_tmp_dir}" -DOUTPUT="${DEMO1_TEST_OUTPUT_DIR}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

    if (DEMO1_BUILD_TEST)
        if ((NOT DEMO1_GEN_PROTOCOL) OR (NOT DEMO1_BUILD_PROTOCOL))
            message (FATAL_ERROR "DEMO1_GEN_PROTOCOL and DEMO1_BUILD_PROTOCOL are required for DEMO1_BUILD_TEST")
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
            DEPENDS "prot_tgt" "test_output_tgt"
            SOURCE_DIR ${DEMO1_TEST_OUTPUT_DIR}
            BINARY_DIR ${test_build_dir}
            LIST_SEPARATOR |
            CMAKE_ARGS
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_GENERATOR=${CMAKE_GENERATOR} -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
                -DCMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}                
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}           
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                -DCMAKE_PREFIX_PATH=${combined_prefix_path_str}
                -DOPT_USE_CCACHE=${DEMO1_USE_CCACHE}
                -DOPT_CCACHE_EXECUTABLE=${DEMO1_CCACHE_EXECUTABLE}                
        )
    endif ()
endif ()

if (DEMO1_GEN_TOOLS)
    find_program(COMMSDSL2TOOLS_QT commsdsl2tools_qt PATH_SUFFIXES bin REQUIRED)    
    execute_process(COMMAND "${COMMSDSL2TOOLS_QT}" --version OUTPUT_VARIABLE commsdsl2tools_qt_version)
    if ("${commsdsl2tools_qt_version}" VERSION_LESS "${DEMO1_MIN_COMMSDSL_VERSION}")
        message (FATAL_ERROR "Version of commsdsl2tools_qt is too old, required minimum is v${DEMO1_MIN_COMMSDSL_VERSION}.")
    endif ()    

    set (tools_tmp_dir "${CMAKE_BINARY_DIR}/output_tools.tmp")

    add_custom_target("tools_output_tgt" ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${tools_tmp_dir}
        COMMAND ${COMMSDSL2TOOLS_QT} ${warn_as_error} ${COMMSDSL2TOOLS_QT_EXTRA_ARGS}
            --protocol "::CC Demo1:Synthenic demo protocol to demonstrate definition of various fields as well as simple protocol framing.:cc.Demo1ProtocolPlugin"
            -o ${tools_tmp_dir} -c ${PROJECT_SOURCE_DIR}/src/tools ${schema_file}        
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${tools_tmp_dir}" -DOUTPUT="${DEMO1_TOOLS_OUTPUT_DIR}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

    if (DEMO1_BUILD_TOOLS)
        if ((NOT DEMO1_GEN_PROTOCOL) OR (NOT DEMO1_BUILD_PROTOCOL))
            message (FATAL_ERROR "DEMO1_GEN_PROTOCOL and DEMO1_BUILD_PROTOCOL are required for DEMO1_BUILD_TOOLS")
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
            DEPENDS "prot_tgt" "tools_output_tgt"
            SOURCE_DIR ${DEMO1_TOOLS_OUTPUT_DIR}
            BINARY_DIR ${tools_build_dir}
            LIST_SEPARATOR |
            CMAKE_ARGS
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_GENERATOR=${CMAKE_GENERATOR} -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
                -DCMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}           
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_PREFIX_PATH=${combined_prefix_path_str}
                -DOPT_QT_MAJOR_VERSION=${DEMO1_TOOLS_QT_VER} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                -DOPT_USE_CCACHE=${DEMO1_USE_CCACHE}
                -DOPT_CCACHE_EXECUTABLE=${DEMO1_CCACHE_EXECUTABLE}
        )   
    endif ()
endif ()

if (DEMO1_GEN_SWIG)
    find_program(COMMSDSL2SWIG commsdsl2swig PATH_SUFFIXES bin REQUIRED)    
    execute_process(COMMAND "${COMMSDSL2SWIG}" --version OUTPUT_VARIABLE commsdsl2swig_version)
    if ("${commsdsl2swig_version}" VERSION_LESS "${DEMO1_MIN_COMMSDSL_VERSION}")
        message (FATAL_ERROR "Version of commsdsl2swig is too old, required minimum is v${DEMO1_MIN_COMMSDSL_VERSION}.")
    endif ()      

    set (swig_tmp_dir "${CMAKE_BINARY_DIR}/output_swig.tmp")

    add_custom_target("swig_output_tgt" ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${swig_tmp_dir}
        COMMAND ${COMMSDSL2SWIG} ${warn_as_error} ${COMMSDSL2SWIG_EXTRA_ARGS}
            -c ${DEMO1_SWIG_SRC_DIR} -o ${swig_tmp_dir} ${schema_file}
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${swig_tmp_dir}" -DOUTPUT="${DEMO1_SWIG_OUTPUT_DIR}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

    if (DEMO1_BUILD_SWIG)
        if ((NOT DEMO1_GEN_PROTOCOL) OR (NOT DEMO1_BUILD_PROTOCOL))
            message (FATAL_ERROR "DEMO1_GEN_PROTOCOL and DEMO1_BUILD_PROTOCOL are required for DEMO1_BUILD_SWIG")
        endif ()
            
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DEMO1_SWIG_OUTPUT_DIR}
        )    

        execute_process(
            COMMAND ${CMAKE_COMMAND} -E touch ${DEMO1_SWIG_OUTPUT_DIR}/dummy.txt
        )

        string (REPLACE ";" "|" combined_languages_str "${DEMO1_SWIG_LANGUAGES}")

        set (swig_build_dir ${CMAKE_CURRENT_BINARY_DIR}/swig_tgt_build)
        ExternalProject_Add(
            "swig_tgt"
            BUILD_ALWAYS TRUE
            DEPENDS "prot_tgt" "swig_output_tgt"
            SOURCE_DIR ${DEMO1_SWIG_OUTPUT_DIR}
            BINARY_DIR ${swig_build_dir}
            LIST_SEPARATOR |
            CMAKE_ARGS
                -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_GENERATOR=${CMAKE_GENERATOR} -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
                -DCMAKE_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET} -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}        
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                -DCMAKE_PREFIX_PATH=${combined_prefix_path_str}
                -DOPT_SWIG_LANGUAGES=${combined_languages_str}
                -DOPT_USE_CCACHE=${DEMO1_USE_CCACHE}
                -DOPT_CCACHE_EXECUTABLE=${DEMO1_CCACHE_EXECUTABLE}
            INSTALL_COMMAND
                ${CMAKE_COMMAND} -E echo "Skipping install"            
        )   
    endif ()
endif ()

if (DEMO1_GEN_EMSCRIPTEN)
    find_program(COMMSDSL2EMSCRIPTEN commsdsl2emscripten PATH_SUFFIXES bin REQUIRED)    
    find_program(EMCMAKE_EXECUTABLE emcmake REQUIRED)    
    execute_process(COMMAND "${COMMSDSL2EMSCRIPTEN}" --version OUTPUT_VARIABLE commsdsl2emscripten_version)
    if ("${commsdsl2emscripten_version}" VERSION_LESS "${DEMO1_MIN_COMMSDSL_VERSION}")
        message (FATAL_ERROR "Version of commsdsl2emscripten is too old, required minimum is v${DEMO1_MIN_COMMSDSL_VERSION}.")
    endif ()    

    set (emscripten_tmp_dir "${CMAKE_BINARY_DIR}/output_emscripten.tmp")

    add_custom_target("emscripten_output_tgt" ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${emscripten_tmp_dir}
        COMMAND ${COMMSDSL2EMSCRIPTEN} ${warn_as_error} ${COMMSDSL2EMSCRIPTEN_EXTRA_ARGS}
            -o ${emscripten_tmp_dir} -c ${PROJECT_SOURCE_DIR}/src/emscripten ${schema_file}
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED="${emscripten_tmp_dir}" -DOUTPUT="${DEMO1_EMSCRIPTEN_OUTPUT_DIR}"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake"        
        DEPENDS "${CMAKE_CURRENT_LIST_DIR}/CopyGenerated.cmake" ${schema_file})

    if (DEMO1_BUILD_EMSCRIPTEN)
        if ((NOT DEMO1_GEN_PROTOCOL) OR (NOT DEMO1_BUILD_PROTOCOL))
            message (FATAL_ERROR "DEMO1_GEN_PROTOCOL and DEMO1_BUILD_PROTOCOL are required for DEMO1_BUILD_EMSCRIPTEN")
        endif ()
            
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DEMO1_EMSCRIPTEN_OUTPUT_DIR}
        )    

        execute_process(
            COMMAND ${CMAKE_COMMAND} -E touch ${DEMO1_EMSCRIPTEN_OUTPUT_DIR}/dummy.txt
        )

        find_package(LibComms REQUIRED)
        get_target_property(comms_include_dirs cc::comms INTERFACE_INCLUDE_DIRECTORIES)
        set (prot_include_dir "${CMAKE_INSTALL_PREFIX}/include")
        set (extra_includes_path ${prot_include_dir} ${comms_include_dirs})
        string (REPLACE ";" "|" extra_includes_path_str "${extra_includes_path}")    


        set (emscripten_build_dir ${CMAKE_CURRENT_BINARY_DIR}/emscripten_tgt_build)
        ExternalProject_Add(
            "emscripten_tgt"
            BUILD_ALWAYS TRUE
            DEPENDS "prot_tgt" "emscripten_output_tgt"
            SOURCE_DIR ${DEMO1_EMSCRIPTEN_OUTPUT_DIR}
            BINARY_DIR ${emscripten_build_dir}
            CMAKE_COMMAND ${EMCMAKE_EXECUTABLE} ${CMAKE_COMMAND}
            LIST_SEPARATOR |
            CMAKE_ARGS
                -DCMAKE_GENERATOR=${CMAKE_GENERATOR} -DCMAKE_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} 
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
                -DOPT_MODULARIZE=ON
                -DOPT_EXTRA_INCLUDE_DIRS=${extra_includes_path_str}
                -DOPT_USE_CCACHE=${DEMO1_USE_CCACHE}
                -DOPT_CCACHE_EXECUTABLE=${DEMO1_CCACHE_EXECUTABLE}
        )   
    endif ()
endif ()

if (NOT DEMO1_BUILD_EXAMPLES)
    return()
endif()

add_subdirectory (examples)
