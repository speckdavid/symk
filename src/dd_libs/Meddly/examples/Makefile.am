## $Id$

## Makefile.am -- Process this file with automake to produce Makefile.in

## compile library

## bin_PROGRAMS = \
noinst_PROGRAMS = \
testmddlib \
graphs_to_mdds \
test_union_mdd \
test_union_mxd \
test_mtmdd \
test_mtmxd \
test_evmdd \
build_expression \
dining_phils \
rubiks_cube \
rubiks_cube_orient \
nqueens \
queen_cover \
qc2 \
kanban \
slot \
life \
libinfo \
square1 \
swaps \
rubiks_rings \
rebuild_mdd \
test_reorder_mtmxd \
test_reorder_evmdd \
rubiks_cube_phase \
slot_phase

# test_user_operation 

AM_CPPFLAGS = -I../src -I../src/operations
AM_CXXFLAGS = -Wall

## testmddlib_SOURCES = `$(CXX) testmddlib.cc -MM`
testmddlib_SOURCES = testmddlib.cc 
testmddlib_LDADD = ../src/libmeddly.la

graphs_to_mdds_SOURCES = graphs_to_mdds.cc
graphs_to_mdds_LDADD = ../src/libmeddly.la

test_union_mdd_SOURCES = test_union_mdd.cc
test_union_mdd_LDADD = ../src/libmeddly.la

test_union_mxd_SOURCES = test_union_mxd.cc
test_union_mxd_LDADD = ../src/libmeddly.la

test_mtmdd_SOURCES = test_mtmdd.cc
test_mtmdd_LDADD = ../src/libmeddly.la

test_mtmxd_SOURCES = test_mtmxd.cc
test_mtmxd_LDADD = ../src/libmeddly.la

build_expression_SOURCES = build_expression.cc
build_expression_LDADD = ../src/libmeddly.la

dining_phils_SOURCES = dining_phils.cc
dining_phils_LDADD = ../src/libmeddly.la

rubiks_cube_SOURCES = rubiks_cube.cc
rubiks_cube_LDADD = ../src/libmeddly.la

rubiks_cube_orient_SOURCES = rubiks_cube_orient.cc
rubiks_cube_orient_LDADD = ../src/libmeddly.la

test_evmdd_SOURCES = test_evmdd.cc
test_evmdd_LDADD = ../src/libmeddly.la

nqueens_SOURCES = nqueens.cc
nqueens_LDADD = ../src/libmeddly.la

queen_cover_SOURCES = queen_cover.cc
queen_cover_LDADD = ../src/libmeddly.la

qc2_SOURCES = qc2.cc
qc2_LDADD = ../src/libmeddly.la

kanban_SOURCES = kanban.cc simple_model.h simple_model.cc
kanban_LDADD = ../src/libmeddly.la

slot_SOURCES = slot.cc simple_model.h simple_model.cc
slot_LDADD = ../src/libmeddly.la

life_SOURCES = life.cc
life_LDADD = ../src/libmeddly.la

libinfo_SOURCES = libinfo.cc
libinfo_LDADD = ../src/libmeddly.la

square1_SOURCES = square1.cc
square1_LDADD = ../src/libmeddly.la

swaps_SOURCES = swaps.cc
swaps_LDADD = ../src/libmeddly.la

rubiks_rings_SOURCES = rubiks_rings.cc
rubiks_rings_LDADD = ../src/libmeddly.la

rebuild_mdd_SOURCES = rebuild_mdd.cc reorder.h
rebuild_mdd_LDADD = ../src/libmeddly.la

test_reorder_mtmxd_SOURCES = test_reorder_mtmxd.cc
test_reorder_mtmxd_LDADD = ../src/libmeddly.la

test_reorder_evmdd_SOURCES = test_reorder_evmdd.cc
test_reorder_evmdd_LDADD = ../src/libmeddly.la

rubiks_cube_phase_SOURCES = rubiks_cube_phase.cc
rubiks_cube_phase_LDADD = ../src/libmeddly.la

slot_phase_SOURCES = slot_phase.cc simple_model.h simple_model.cc
slot_phase_LDADD = ../src/libmeddly.la

# test_user_operation_SOURCES = test_user_operation.cc
# test_user_operation_LDADD = ../src/libmeddly.la

