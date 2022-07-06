cmake_minimum_required( VERSION 3.20 )

project(manualcontrol VERSION 0.0.1 DESCRIPTION "manualcontrol")

add_library(manualcontrol STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/xcontroller.cpp)

set_target_properties(manualcontrol PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(manualcontrol PROPERTIES SOVERSION 0.0.1)

target_include_directories(manualcontrol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (NOT TARGET machycore)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore libs/machycore/machycore)
endif()
if (WIN32)
    target_link_libraries(manualcontrol PUBLIC Boost::boost XInput machycore)
endif (WIN32)
if (UNIX)
    target_link_libraries(manualcontrol PUBLIC Boost::boost machycore)
endif()
# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS manualcontrol
        EXPORT manualcontrol
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")