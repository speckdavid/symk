# --- Find GMP (C library) ---
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARY NAMES gmp)

# --- Find GMPXX (C++ library) ---
find_path(GMPXX_INCLUDE_DIR NAMES gmpxx.h)
find_library(GMPXX_LIBRARY NAMES gmpxx)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gmp
    REQUIRED_VARS GMP_LIBRARY GMPXX_LIBRARY GMP_INCLUDE_DIR
)

if(Gmp_FOUND)
    # Define Gmp::Gmp if it doesn't exist
    if(NOT TARGET Gmp::Gmp)
        add_library(Gmp::Gmp UNKNOWN IMPORTED)
        set_target_properties(Gmp::Gmp PROPERTIES
            IMPORTED_LOCATION "${GMP_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
        )
    endif()

    # Define Gmp::Gmpxx
    if(NOT TARGET Gmp::Gmpxx)
        add_library(Gmp::Gmpxx UNKNOWN IMPORTED)
        set_target_properties(Gmp::Gmpxx PROPERTIES
            IMPORTED_LOCATION "${GMPXX_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GMPXX_INCLUDE_DIR}"
            # This line is critical: it forces libgmp to be linked whenever libgmpxx is used
            INTERFACE_LINK_LIBRARIES Gmp::Gmp
        )
    endif()
endif()