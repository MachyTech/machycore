cmake_minimum_required(VERSION 3.20)

project( Camera-Tester)

enable_language(CXX)

if (NOT TARGET glfw)
    add_subdirectory(libs/machycore/vendor/glfw)
endif()
if (NOT TARGET machyapi)
    add_subdirectory(libs/machycore/api)
endif()
if (NOT TARGET machycore)
    add_subdirectory(libs/machycore/machycore)
endif()
if (NOT TARGET machygl)
    add_subdirectory(libs/machycore/machygl)
endif()
if (NOT TARGET machycontrol)
    add_subdirectory(libs/machycore/machycontrol)
endif()
if (NOT TARGET manualcontrol)
    add_subdirectory(libs/machycore/manualcontrol)
endif()
if (NOT TARGET machycam)
    add_subdirectory(libs/machycore/machycam)
endif()
if (NOT TARGET machyvision)
    add_subdirectory(libs/machycore/machyvision)
endif()
if (NOT TARGET nlohmann_json)
    add_subdirectory(libs/machycore/api/libs/json)
endif()

add_definitions(-D SHOW_FPS)

file(COPY libs/machyshaders/shaders DESTINATION .)
file(COPY media DESTINATION .)

add_executable(app ${SOURCES} "main.cpp")
target_link_libraries(app PRIVATE machycontrol manualcontrol machyapi machygl glfw machycam machyvision)