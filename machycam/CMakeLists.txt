cmake_minimum_required(VERSION 3.10)

project(machycam VERSION 0.0.1 DESCRIPTION "camera functions used in machytech projection systems")
add_library(machycam STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machycam.cpp)
set_target_properties(machycam PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machycam PROPERTIES SOVERSION 0.0.1)

find_package( OpenCV REQUIRED )

find_package( realsense2 REQUIRED)

target_include_directories(machycam PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

#if (NOT TARGET opencv)
#    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/vendor/opencv vendor/opencv)
#\endif()
if (NOT TARGET machycore)
   add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore libs/machycore/machycore)
endif()
if (NOT TARGET dark)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machyvision/libs/darknet/ libs/darknet/)
endif()
if (NOT TARGET machyvision)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machyvision libs/machycore/machyvision)
endif()

target_link_libraries(machycam PRIVATE dark Boost::boost -lX11 machycore machyvision ${OpenCV_LIBS} ${realsense2_LIBRARY})

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machycam
        EXPORT machycamTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
