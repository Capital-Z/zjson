cmake_minimum_required(VERSION 3.15)

project(zjson VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ZJSON_BUILD_SHARED "Build zjson as shared library" OFF)

if(ZJSON_BUILD_SHARED)
    add_library(zjson SHARED zjson.cpp)
else()
    add_library(zjson STATIC zjson.cpp)
endif()

target_include_directories(zjson PUBLIC ${PROJECT_SOURCE_DIR})

# Try to locate jansson (header + library). Fail early with a clear message if not found.
find_path(JANSSON_INCLUDE_DIR NAMES jansson.h)
find_library(JANSSON_LIBRARY NAMES jansson)

if(NOT JANSSON_INCLUDE_DIR OR NOT JANSSON_LIBRARY)
    message(FATAL_ERROR "jansson library not found. Please install jansson development files and try again (pkg: jansson, header jansson.h).")
endif()

target_include_directories(zjson PRIVATE ${JANSSON_INCLUDE_DIR})
target_link_libraries(zjson PRIVATE ${JANSSON_LIBRARY})

# Tester executable
add_executable(tester tester/tester.cpp)
target_link_libraries(tester PRIVATE zjson)
target_include_directories(tester PRIVATE ${PROJECT_SOURCE_DIR})

install(TARGETS zjson EXPORT zjsonTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install(FILES zjson.h DESTINATION include)
