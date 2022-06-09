cmake_minimum_required(VERSION 3.10)

# name of the project
project(machyscenes VERSION 0.0.1 DESCRIPTION "scenes used by machytechs projecting system")

add_library(machyscenes STATIC src/machyscene.cpp)

set_target_properties(machyscenes PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machyscenes PROPERTIES SOVERSION 0.0.1)

target_include_directories(machyscenes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# enable_language(CXX)
# set (CMAKE_CXX_STANDARD 11)
# pthread
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# if (NOT TARGET glfw)
#     add_subdirectory(../../vendor/glfw vendor/glfw)
# endif()
# if (NOT TARGET machyapi)    
#     add_subdirectory("${CMAKE_SOURCE_DIR}/lib/api")
# endif()
# if (NOT TARGET machycore)
#     add_subdirectory("${CMAKE_SOURCE_DIR}/lib/machycore")
# endif()
# if (NOT TARGET machygl)
#     add_subdirectory("${CMAKE_SOURCE_DIR}/lib/machygl")
# endif()

# include shaders
# file(COPY ../../shader DESTINATION .)

# header files
include_directories(${CMAKE_SOURCE_DIR}/vendor/glm)
include_directories(${CMAKE_SOURCE_DIR}/libs/machycore/machycore/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/machycore/machygl/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/machycore/api/include)

if (NOT TARGET glfw)
    add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/glfw vendor/glfw)
endif()

# link libraries
target_link_libraries(machyscenes -lX11)
# target_link_libraries(machyscenes -lpthread)
target_link_libraries(machyscenes -lXrandr)
# target_link_libraries(SceneModules -ljson-c)
# target_link_libraries(SceneModules -lcurl)
target_link_libraries(machyscenes glfw)
# # machytech libraries
target_link_libraries(machyscenes machygl)
target_link_libraries(machyscenes machyapi)
target_link_libraries(machyscenes machycore)
# ---------------------------------------------------------
#   Install
# ---------------------------------------------------------
install(TARGETS machyscenes
        EXPORT machyscenesTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
