cmake_minimum_required(VERSION 3.10)

# name of the project
project(machycore VERSION 0.0.1 DESCRIPTION "machycore functions used by machytechs projecting system")

add_library(machycore STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/machycore.cpp)

set_target_properties(machycore PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(machycore PROPERTIES SOVERSION 0.0.1)

set(BOOST_URL "https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.tar.bz2")
set(BOOST_URL_SHA256 "475d589d51a7f8b3ba2ba4eda022b170e562ca3b760ee922c146b6c65856ef39")
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/machycore/machycore/libs/boost-build libs/machycore/boost-build)

find_package( OpenCV REQUIRED )
target_include_directories(machycore PUBLIC 
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(machycore PRIVATE Boost::boost ${OpenCV_LIBS})

# header files
# ---------------------------------------------------------
#   Install
# ---------------------------------------------------------
install(TARGETS machycore
        EXPORT machycoreTargets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
