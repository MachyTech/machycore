cmake_minimum_required(VERSION 3.20.1)

# name of the project
project(machygl VERSION 0.0.1 DESCRIPTION "openGL util functions used by machytechs projecting system")
add_library(machygl STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machygl.cpp)
set_target_properties(machygl PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machygl PROPERTIES SOVERSION 0.0.1)

target_include_directories(machygl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (NOT TARGET glfw)
     add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/glfw vendor/glfw)
endif()

target_link_libraries(machygl glfw)

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machygl
        EXPORT machyglTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
