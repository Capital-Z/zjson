cmake_minimum_required(VERSION 3.15)

project(zjson VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ZJSON_BUILD_SHARED "Build zjson as shared library" OFF)

if(ZJSON_BUILD_SHARED)
	add_library(zjson SHARED zjson.cpp)
	target_compile_definitions(zjson PUBLIC ZJSON_BUILD_SHARED)
else()
	add_library(zjson STATIC zjson.cpp)
endif()

target_include_directories(zjson PUBLIC ${PROJECT_SOURCE_DIR})

# Provide the jansson installation prefix (or include/lib paths)
set(JANSSON_ROOT "" CACHE PATH "Root directory (prefix) of the jansson installation. If set, its include/ and lib/ subdirs will be searched first.")
find_path(JANSSON_INCLUDE_DIR NAMES jansson.h HINTS ${JANSSON_ROOT}/include ${JANSSON_ROOT})
find_library(JANSSON_LIBRARY_RELEASE NAMES jansson HINTS ${JANSSON_ROOT}/lib ${JANSSON_ROOT}/lib64 ${JANSSON_ROOT})
find_library(JANSSON_LIBRARY_DEBUG   NAMES jansson_d jansson HINTS ${JANSSON_ROOT}/lib ${JANSSON_ROOT}/lib64 ${JANSSON_ROOT})
set(JANSSON_LIB_RELEASE ${JANSSON_LIBRARY_RELEASE})
set(JANSSON_LIB_DEBUG ${JANSSON_LIBRARY_DEBUG})
# Fallback: use jansson release if debug build is not found, but not the other way around.
if(NOT JANSSON_LIB_DEBUG AND JANSSON_LIB_RELEASE)
	set(JANSSON_LIB_DEBUG ${JANSSON_LIB_RELEASE})
endif()

if(NOT JANSSON_INCLUDE_DIR OR (NOT JANSSON_LIB_RELEASE AND NOT JANSSON_LIB_DEBUG))
	message(FATAL_ERROR "jansson library not found. Please adjust JANSSON_ROOT to point where the include and lib jansson dirs are located.")
endif()

target_include_directories(zjson PRIVATE ${JANSSON_INCLUDE_DIR})

target_link_libraries(zjson PRIVATE
	$<$<CONFIG:Debug>:${JANSSON_LIB_DEBUG}>
	$<$<NOT:$<CONFIG:Debug>>:${JANSSON_LIB_RELEASE}>
)

# Demo executable
add_executable(demo demo/demo.cpp)
target_link_libraries(demo PRIVATE zjson)
target_include_directories(demo PRIVATE ${PROJECT_SOURCE_DIR})

# Probe whether the compiler accepts common C++20 flags. If a flag is supported
# add it to the target compile options. This is optional: if the check fails we
# do not force the standard.
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("/std:c++20" CXX_HAS_STD_CXX20_MSVC)
check_cxx_compiler_flag("-std=c++20" CXX_HAS_STD_CXX20_GNU)

if(CXX_HAS_STD_CXX20_MSVC)
	target_compile_options(zjson PRIVATE /std:c++20)
	target_compile_options(demo PRIVATE /std:c++20)
	message("CXX_HAS_STD_CXX20")
elseif(CXX_HAS_STD_CXX20_GNU)
	target_compile_options(zjson PRIVATE -std=c++20)
	target_compile_options(demo PRIVATE -std=c++20)
	message("NOT CXX_HAS_STD_CXX20")
endif()

install(TARGETS zjson EXPORT zjsonTargets LIBRARY DESTINATION lib ARCHIVE DESTINATION lib RUNTIME DESTINATION bin)
install(FILES zjson.h DESTINATION include)
