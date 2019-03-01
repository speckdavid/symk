## $Id$

## Makefile.am -- Process this file with automake to produce Makefile.in

## compile library

##
## This version: don't use recursive make
##

## SUBDIRS = forests operations storage .

lib_LTLIBRARIES = libmeddly.la

libmeddly_la_SOURCES = \
  revision.h \
  defines.h hash_stream.h heap.h timer.h unique_table.h \
  meddly.h meddly.hh meddly.cc \
  meddly_expert.h meddly_expert.hh \
  io.cc \
  node_wrappers.cc \
  ops.cc \
  dd_edge.cc \
  enumerator.cc \
  domain.cc \
  forest.cc \
  compute_table.cc \
  unique_table.cc \
  loggers.h loggers.cc \
  global_rebuilder.cc \
  \
  reordering/reordering_factory.h \
  reordering/bring_up_reordering.h \
  reordering/highest_inversion_reordering.h \
  reordering/larc_reordering.h \
  reordering/lowest_cost_reordering.h \
  reordering/lowest_inversion_reordering.h \
  reordering/lowest_memory_reordering.h \
  reordering/random_reordering.h \
  reordering/reordering_base.h \
  reordering/sink_down_reordering.h \
  \
  forests/mt.h                forests/mt.cc \
  forests/mtmdd.h             forests/mtmdd.cc \
  forests/mtmddbool.h         forests/mtmddbool.cc \
  forests/mtmddint.h          forests/mtmddint.cc \
  forests/mtmddreal.h         forests/mtmddreal.cc \
  forests/mtmxd.h             forests/mtmxd.cc \
  forests/mtmxdbool.h         forests/mtmxdbool.cc \
  forests/mtmxdint.h          forests/mtmxdint.cc \
  forests/mtmxdreal.h         forests/mtmxdreal.cc \
  forests/ev.h                forests/ev.cc \
  forests/evmdd.h             forests/evmdd.cc \
  forests/evmdd_plusint.h     forests/evmdd_plusint.cc \
  forests/evmdd_plusreal.h     forests/evmdd_plusreal.cc \
  forests/evmxd.h             forests/evmxd.cc \
  forests/evmdd_timesreal.h   \
  forests/evmxd_timesreal.h   forests/evmxd_timesreal.cc \
  forests/init_forests.h      forests/init_forests.cc \
  \
  operations/apply_base.h     operations/apply_base.cc   \
  operations/mpz_object.h     operations/mpz_object.cc   \
  operations/copy.h           operations/copy.cc         \
  operations/cardinality.h    operations/cardinality.cc  \
  operations/complement.h     operations/complement.cc   \
  operations/maxmin_range.h   operations/maxmin_range.cc \
  operations/mdd2index.h      operations/mdd2index.cc    \
  operations/union.h          operations/union.cc        \
  operations/intersection.h   operations/intersection.cc \
  operations/difference.h     operations/difference.cc   \
  operations/cross.h          operations/cross.cc        \
  operations/maxmin.h         operations/maxmin.cc       \
  operations/plus.h           operations/plus.cc         \
  operations/minus.h          operations/minus.cc        \
  operations/multiply.h       operations/multiply.cc     \
  operations/divide.h         operations/divide.cc       \
  operations/modulo.h         operations/modulo.cc       \
  operations/comp_eq.h        operations/comp_eq.cc      \
  operations/comp_ne.h        operations/comp_ne.cc      \
  operations/comp_lt.h        operations/comp_lt.cc      \
  operations/comp_le.h        operations/comp_le.cc      \
  operations/comp_gt.h        operations/comp_gt.cc      \
  operations/comp_ge.h        operations/comp_ge.cc      \
  operations/prepostimage.h   operations/prepostimage.cc \
  operations/reach_bfs.h      operations/reach_bfs.cc    \
  operations/reach_dfs.h      operations/reach_dfs.cc    \
  operations/sat_pregen.h     operations/sat_pregen.cc   \
  operations/sat_otf.h        operations/sat_otf.cc      \
  operations/vect_matr.h      operations/vect_matr.cc    \
  operations/mm_mult.h        operations/mm_mult.cc      \
  operations/init_builtin.h   operations/init_builtin.cc \
  \
  storage/bytepack.h  \
  storage/holeman.h         storage/holeman.cc \
  storage/hm_array.h        storage/hm_array.cc \
  storage/hm_grid.h         storage/hm_grid.cc \
  storage/hm_heap.h         storage/hm_heap.cc \
  storage/hm_none.h         storage/hm_none.cc \
  storage/old_scheme.h      storage/old_scheme.cc \
  storage/simple.h          storage/simple.cc \
  storage/compact.h         storage/compact.cc \
  storage/ct_classic.h      storage/ct_classic.cc \
  storage/init_storage.h    storage/init_storage.cc

  
## compile apps

include_HEADERS = meddly.h meddly_expert.h meddly.hh meddly_expert.hh

AM_CXXFLAGS = -Wall
