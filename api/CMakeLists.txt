cmake_minimum_required(VERSION 3.20)

# name of the project
project(machyapi VERSION 0.0.1 DESCRIPTION "machyapi functions used by machytechs projecting system")

add_library(machyapi STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machyapi.cpp)

set_target_properties(machyapi PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machyapi PROPERTIES SOVERSION 0.0.1)

target_include_directories(machyapi PUBLIC ${CMAKE_SOURCE_DIR}/libs/machycore/api/include)

if (NOT TARGET machycore)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore libs/machycore/machycore)
endif()
if (NOT TARGET manualcontrol)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/manualcontrol libs/machycore/manualcontrol)
endif()
if (NOT TARGET nlohmann_json)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/api/libs/json libs/api/json)
endif()
target_link_libraries(machyapi PUBLIC Boost::boost nlohmann_json machycore manualcontrol)

# ---------------------------------------------------------
#   Install
# ---------------------------------------------------------
install(TARGETS machyapi
        EXPORT machyapiTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
