project(preprocess LANGUAGES CXX)

set(PREPROCESS_SOURCES
    planner.cc
    axiom.cc
    causal_graph.cc
    h2_mutexes.cc
    helper_functions.cc
    max_dag.cc
    mutex_group.cc
    operator.cc
    scc.cc
    state.cc
    variable.cc
)

add_executable(preprocess ${PREPROCESS_SOURCES})

macro(check_and_set_compiler_flag FLAG)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag( "${FLAG}" FLAG_FOUND )
    if(NOT FLAG_FOUND)
        message(FATAL_ERROR "${CMAKE_CXX_COMPILER} does not support ${FLAG}")
    endif()
    message("Flag '${FLAG}' set for '${CMAKE_CXX_COMPILER}'")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endmacro()

macro(fast_downward_set_compiler_flags)
    set(non_windows_compilers "GNU" "Clang" "AppleClang")
    if(${CMAKE_CXX_COMPILER_ID} IN_LIST non_windows_compilers)
        check_and_set_compiler_flag( "-std=c++20" )

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wnon-virtual-dtor")

        if (CMAKE_COMPILER_IS_GNUCXX
            AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12
            AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
            ## We ignore the warning "restrict" because of a bug in GCC 12:
            ## https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105651
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-restrict")
        endif()

        ## Configuration-specific flags
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fomit-frame-pointer")
        set(CMAKE_CXX_FLAGS_DEBUG "-O3")
        if(USE_GLIBCXX_DEBUG)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
        endif()
        set(CMAKE_CXX_FLAGS_PROFILE "-O3 -pg")
    elseif(MSVC)
        check_and_set_compiler_flag( "/std:c++20" )

        # Enable exceptions.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

        # Use warning level 4 (/W4).
        # /Wall currently detects too many warnings outside of our code to be useful.
        string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

        # Disable warnings that currently trigger in the code until we fix them.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800") # forcing value to bool
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4512") # assignment operator could not be generated
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4706") # assignment within conditional expression (in tree.hh)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4100") # unreferenced formal parameter (in OSI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4127") # conditional expression is constant (in tree.hh and in our code)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244") # conversion with possible loss of data
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4309") # truncation of constant value (in OSI, see issue857)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4702") # unreachable code
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4239") # nonstandard extension used
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996") # function call with parameters that may be unsafe
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4456") # declaration hides previous local declaration
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4458") # declaration hides class member
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267") # conversion from size_t to int with possible loss of data

        # The following are disabled because of what seems to be compiler bugs.
        # "unreferenced local function has been removed";
        # see http://stackoverflow.com/questions/3051992/compiler-warning-at-c-template-base-class
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4505")
    else()
        message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER}")
    endif()
endmacro()

macro(fast_downward_set_linker_flags)
    if(UNIX)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g")
    endif()
endmacro()

fast_downward_set_compiler_flags()
fast_downward_set_linker_flags()