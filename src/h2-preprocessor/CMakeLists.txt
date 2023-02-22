if(NOT FAST_DOWNWARD_MAIN_CMAKELISTS_READ)
    message(
        FATAL_ERROR
        "Run cmake on the CMakeLists.txt in the 'src' directory. "
        "Please delete CMakeCache.txt from the current directory "
        "and restart cmake.")
endif()

project(preprocess)
fast_downward_set_compiler_flags()
fast_downward_set_linker_flags()

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
