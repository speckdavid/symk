cmake_minimum_required(VERSION 2.8.3)

if(NOT FAST_DOWNWARD_MAIN_CMAKELISTS_READ)
    message(
        FATAL_ERROR
        "Run cmake on the CMakeLists.txt in the 'src' directory, "
        "not the one in 'src/search'. Please delete CMakeCache.txt "
        "from the current directory and restart cmake.")
endif()


## == Project ==

project(downward)
fast_downward_set_compiler_flags()
fast_downward_set_linker_flags()

# Collect source files needed for the active plugins.
include("${CMAKE_CURRENT_SOURCE_DIR}/DownwardFiles.cmake")
add_executable(downward ${PLANNER_SOURCES})

## == Includes ==

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext)

## == Libraries ==

# On Linux, find the rt library for clock_gettime().
if(UNIX AND NOT APPLE)
    target_link_libraries(downward rt)
endif()

# On Windows, find the psapi library for determining peak memory.
if(WIN32)
    target_link_libraries(downward psapi)
endif()

# If any enabled plugin requires an LP solver, compile with all
# available LP solvers. If no solvers are installed, the planner will
# still compile, but using heuristics that depend on an LP solver will
# cause an error. This behavior can be overwritten by setting the
# option USE_LP to false.
option(
  USE_LP
  "Compile with support for all LP solvers installed on this system."
  TRUE)

if(PLUGIN_LP_SOLVER_ENABLED AND USE_LP)
    find_package(OSI OPTIONAL_COMPONENTS Cpx Clp Grb Spx)
    if(OSI_FOUND AND (OSI_Cpx_FOUND OR OSI_Clp_FOUND OR OSI_Grb_FOUND OR OSI_Spx_FOUND))
        foreach(SOLVER Cpx Clp Grb Spx)
            if(OSI_${SOLVER}_FOUND)
                string(TOUPPER ${SOLVER} TMP_SOLVER_UPPER_CASE)
                mark_as_advanced(TMP_SOLVER_UPPER_CASE)
                add_definitions("-D COIN_HAS_${TMP_SOLVER_UPPER_CASE}")
                include_directories(${OSI_${SOLVER}_INCLUDE_DIRS})
                target_link_libraries(downward ${OSI_${SOLVER}_LIBRARIES})
            endif()
        endforeach()

        # Note that basic OSI libs must be added after (!) all OSI solver libs.
        add_definitions("-D USE_LP")
        include_directories(${OSI_INCLUDE_DIRS})
        target_link_libraries(downward ${OSI_LIBRARIES})

        find_package(ZLIB REQUIRED)
        if(ZLIB_FOUND)
            include_directories(${ZLIB_INCLUDE_DIRS})
            target_link_libraries(downward ${ZLIB_LIBRARIES})
        endif()
    endif()

    if(OSI_Cpx_FOUND AND CPLEX_RUNTIME_LIBRARY)
        add_custom_command(TARGET downward POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CPLEX_RUNTIME_LIBRARY}
            $<TARGET_FILE_DIR:downward>
        )
    endif()
endif()

### Link Libraries
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CUDD_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
elseif(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CUDD_CXX_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")
else()
    message(FATAL_ERROR, "Could not determine build type.")
endif()
include(ExternalProject)

### Boost
message(STATUS "Building MiniBoost.")
include_directories(SYSTEM sdac_parser/boost_dependencies)
add_subdirectory(sdac_parser/boost_dependencies/libs)

### Cudd
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/cudd)
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/cplusplus)

if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    message(STATUS "Building Cudd with 32-bit.")
    ExternalProject_add(
        libcudd
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/
	CONFIGURE_COMMAND autoreconf ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/ && ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/configure --enable-obj --enable-silent-rules "CFLAGS=-m32 -g -O3 -w" "CXXFLAGS=-m32 -std=c++0x -g ${CUDD_CXX_FLAGS}" "LDFLAGS=-m32"
        BUILD_COMMAND make
        INSTALL_COMMAND ""
        BUILD_IN_SOURCE 0
    )
elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    message(STATUS "Building Cudd with 64-bit.")
    ExternalProject_add(
        libcudd
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/
        CONFIGURE_COMMAND autoreconf ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/ && ${CMAKE_CURRENT_SOURCE_DIR}/../dd_libs/cudd-3.0.0/configure --enable-obj --enable-silent-rules "CFLAGS=-m64 -g -O3 -w" "CXXFLAGS=-m64 -std=c++0x -g ${CUDD_CXX_FLAGS}" "LDFLAGS=-m64"
        BUILD_COMMAND make
        INSTALL_COMMAND ""
        BUILD_IN_SOURCE 0
    )
else()
    message(FATAL_ERROR, "Could not determine bitwidth for Cudd.")
endif()

add_dependencies(libcudd MiniBoost)
add_dependencies(translate libcudd)
add_dependencies(preprocess libcudd)
add_dependencies(downward libcudd)
target_link_libraries(downward ${downward_BINARY_DIR}/libcudd-prefix/src/libcudd-build/cudd/.libs/libcudd.a)
