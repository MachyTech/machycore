cmake_minimum_required(VERSION 3.10)

project(machycontrol VERSION 0.0.1 DESCRIPTION "control functions written by machytech")
add_library(machycontrol STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machycontrol.cpp)
set_target_properties(machycontrol PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machycontrol PROPERTIES SOVERSION ${PROJECT_VERSION})

target_include_directories(machycontrol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machycontrol
        EXPORT machycontrolTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
