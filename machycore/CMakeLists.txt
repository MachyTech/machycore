cmake_minimum_required(VERSION 3.10)

# name of the project
project(machycore VERSION 0.0.1 DESCRIPTION "machycore functions used by machytechs projecting system")

add_library(machycore SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/machycore.cpp)

set_target_properties(machycore PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machycore PROPERTIES SOVERSION 0.0.1)

target_include_directories(machycore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# header files
# ---------------------------------------------------------
#   Install
# ---------------------------------------------------------
install(TARGETS machycore
        EXPORT machycoreTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
