cmake_minimum_required(VERSION 3.20)

project(machyvision VERSION 0.0.1 DESCRIPTION "vision algorithms used in machytech projection systems")
add_library(machyvision STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machyvision.cpp)
set_target_properties(machyvision PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machyvision PROPERTIES SOVERSION 0.0.1)

find_package( OpenCV REQUIRED )

target_include_directories(machyvision PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

file(COPY weights DESTINATION ${CMAKE_SOURCE_DIR})
if (NOT TARGET machycore)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore libs/machycore/machycore)
endif()
if (NOT TARGET dark)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machyvision/libs/darknet/ libs/darknet/)
endif()
target_link_libraries(machyvision PRIVATE Boost::boost machycore ${OpenCV_LIBS} dark)

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machyvision
        EXPORT machyvisionTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
