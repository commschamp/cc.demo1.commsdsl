set (name "${PROJECT_NAME}_client_example")

find_package(Boost 
    COMPONENTS program_options system date_time)

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
target_link_libraries(${name} 
    Boost::boost Boost::program_options Boost::system Boost::date_time ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(${name} PRIVATE "${OUTPUT_DIR}/include")

add_dependencies(${name} ${PROJECT_NAME})

if (TARGET ${COMMS_TGT})
    add_dependencies(${name} ${COMMS_TGT})
endif ()

install(TARGETS ${name}
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
)
