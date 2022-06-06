cmake_minimum_required(VERSION 3.20.1)

# name of the project
project(machyapi VERSION 0.0.1 DESCRIPTION "machyapi functions used by machytechs projecting system")

add_library(machyapi SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/api.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/sync_api.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/http_async_api.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_async_client_api.cpp)

set_target_properties(machyapi PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machyapi PROPERTIES SOVERSION 0.0.1)

target_include_directories(machyapi PUBLIC ${CMAKE_SOURCE_DIR}/lib/api/include)

find_package( Boost 1.58 EXACT REQUIRED system )
include_directories(${Boost_INCLUDE_DIRS})

if (NOT TARGET machycore)
    add_subdirectory(${CMAKE_SOURCE_DIR}/lib/machycore lib/machycore)
endif()

target_link_libraries(machyapi machycore ${Boost_LIBRARIES})

# use boost enhanced functions(?)
if(BOOST_ENHANCED)
   add_compile_definitions(BOOST_ENHANCED)
endif()

# use VIR POSITION
if(VIR_POSITION)
    add_compile_definitions(VIR_POSITION)
endif()

# ---------------------------------------------------------
#   Install
# ---------------------------------------------------------
install(TARGETS machyapi
        EXPORT machyapiTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
