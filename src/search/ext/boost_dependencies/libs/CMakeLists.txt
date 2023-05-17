project( MiniBoost )
file( GLOB_RECURSE LIBSRCS *.c *.cpp *.cc *.h *.hpp )

# Ignore Warning
set_source_files_properties(
  ${LIBSRCS}
  PROPERTIES
  COMPILE_FLAGS "-w"
)

add_library( ${PROJECT_NAME} STATIC ${LIBSRCS} )