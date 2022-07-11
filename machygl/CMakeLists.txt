cmake_minimum_required(VERSION 3.10)

# name of the project
project(machygl VERSION 0.0.1 DESCRIPTION "openGL util functions used by machytechs projecting system")
add_library(machygl STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machygl.cpp 
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/glad.cpp)
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
   add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycontrol libs/machycore/machycontrol)
endif()
if (NOT TARGET nlohmann_json)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/api/libs/json libs/json)
endif()

target_link_libraries(machygl PRIVATE Boost::boost -lpthread -lXrandr
                                       glfw nlohmann_json machycore machyapi machycontrol)

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machygl
        EXPORT machyglTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
