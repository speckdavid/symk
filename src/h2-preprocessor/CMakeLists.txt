cmake_minimum_required(VERSION 2.8.3)

if(NOT FAST_DOWNWARD_MAIN_CMAKELISTS_READ)
    message(
        FATAL_ERROR
        "Run cmake on the CMakeLists.txt in the root directory, "
        "not the one in 'src'. Please delete CMakeCache.txt "
        "from the current directory and restart cmake.")
endif()

project(preprocess)
fast_downward_set_compiler_flags()
fast_downward_set_linker_flags()

set(PREPROCESS_SOURCES
    planner.cc
    axiom.cc
    causal_graph.cc
    domain_transition_graph.cc
    h2_mutexes.cc
    helper_functions.cc
    max_dag.cc
    mutex_group.cc
    operator.cc
    scc.cc
    state.cc
    successor_generator.cc
    variable.cc
)

add_executable(preprocess ${PREPROCESS_SOURCES})
