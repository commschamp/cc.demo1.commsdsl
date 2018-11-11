set (name "${PROJECT_NAME}_client_example")

find_package(Boost 
    COMPONENTS program_options system)

if (NOT Boost_FOUND)
    message (Warning "Cannot compile ${name} application due to missing boost libraries")
    return ()
endif()

find_package(Threads REQUIRED)

set (src
    "main.cpp"
    "ProgramOptions.cpp"
    "Client.cpp"
)

add_executable(${name} ${src})
target_include_directories(${name} PRIVATE "${OUTPUT_DIR}/include")
target_link_libraries(${name} Boost::boost)
#target_compile_definitions (${name} PRIVATE "-DBOOST_NO_CXX11_SCOPED_ENUMS")

if (WIN32)
    target_link_libraries(
        ${name} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
            Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
else ()    
    target_link_libraries(${name} 
        Boost::program_options Boost::system ${CMAKE_THREAD_LIBS_INIT})
endif ()

add_dependencies(${name} ${PROJECT_NAME})

if (TARGET ${COMMS_TGT})
    add_dependencies(${name} ${COMMS_TGT})
endif ()

install(TARGETS ${name}
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
)
