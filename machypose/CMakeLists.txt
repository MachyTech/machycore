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

if (NOT TARGET ORB_SLAM3)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machypose/libs/ORB_SLAM3/ libs/ORB_SLAM3/)
endif()
find_package(Pangolin REQUIRED)
if (NOT TARGET DBoW2)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machypose/libs/ORB_SLAM3/Thirdparty/DBoW2/ libs/ORB_SLAM3/Thirdparty/DBoW2/)
endif()
if (NOT TARGET g2o)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machypose/libs/ORB_SLAM3/Thirdparty/g2o/ libs/ORB_SLAM3/Thirdparty/g2o/)
endif()
if (NOT TARGET Sophus)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machypose/libs/ORB_SLAM3/Thirdparty/Sophus/ libs/ORB_SLAM3/Thirdparty/Sophus/)
endif()
if (NOT TARGET machycore)
    add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore libs/machycore/machycore)
endif()
target_link_libraries(machypose PRIVATE Boost::boost ${OpenCV_LIBS} machycore ORB_SLAM3 Pangolin Sophus DBoW2 g2o)

# ---------------------------------------------------------
# Install
# ---------------------------------------------------------
install(TARGETS machypose
        EXPORT machyposeTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
