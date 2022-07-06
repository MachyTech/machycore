cmake_minimum_required(VERSION 3.10)

# name of the project
project(machygl VERSION 0.0.1 DESCRIPTION "openGL util functions used by machytechs projecting system")
add_library(machygl STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machygl.cpp)
set_target_properties(machygl PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machygl PROPERTIES SOVERSION 0.0.1)

target_include_directories(machygl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (NOT TARGET glfw)
     add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/vendor/glfw vendor/glfw)
endif()
if (NOT TARGET machyapi)
   add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/api libs/machycore/api)
endif()
if (NOT TARGET machycore)
   add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore libs/machycore/machycore)
endif()
if (NOT TARGET machycontrol)
   add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycontrol libs/machycore/machycontrol0)
endif()
if (NOT TARGET nlohmann_json)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/api/libs/json libs/json)
endif()

file(GLOB SOURCES "src/glad.cpp")
ADD_LIBRARY(LibsModule ${SOURCES})
target_link_libraries(LibsModule Boost::boost)
target_link_libraries(LibsModule -lpthread)
target_link_libraries(LibsModule -lXrandr)
target_link_libraries(LibsModule glfw)
target_link_libraries(LibsModule machygl)
target_link_libraries(LibsModule nlohmann_json)
target_link_libraries(LibsModule machycore)
target_link_libraries(LibsModule machyapi)
target_link_libraries(LibsModule machycontrol)

target_link_libraries(machygl PRIVATE LibsModule)

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machygl
        EXPORT machyglTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
