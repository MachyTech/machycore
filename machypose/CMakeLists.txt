cmake_minimum_required(VERSION 3.20)

project(machypose VERSION 0.0.1 DESCRIPTION "pose algorithms used in machytech projection systems")
add_library(machypose STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machypose.cpp)
set_target_properties(machypose PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machypose PROPERTIES SOVERSION 0.0.1)

find_package( OpenCV REQUIRED )

target_include_directories(machypose PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

if (NOT TARGET machycore)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore libs/machycore/machycore)
endif()
target_link_libraries(machypose PRIVATE Boost::boost machycore ${OpenCV_LIBS})

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machypose
        EXPORT machyposeTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
