set (name "cc_demo1_server_example")

set (src
    "main.cpp"
    "ProgramOptions.cpp"
    "Server.cpp"
    "Session.cpp"
)

add_executable(${name} ${src})
target_include_directories(${name} PRIVATE "${DEMO1_PROT_OUTPUT_DIR}/include")
target_link_libraries(${name} cc::comms Boost::boost Boost::program_options Boost::system)

foreach (c ${extra_boost_components})
    target_link_libraries(${name} Boost::${c})
endforeach()

target_link_libraries(${name} ${CMAKE_THREAD_LIBS_INIT})

if (WIN32)
    target_link_libraries(
        ${name} Setupapi.lib Ws2_32.lib imm32.lib winmm.lib)
endif ()

add_dependencies(${name} ${PROT_OUTPUT_TGT})

install(TARGETS ${name}
    RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
)
