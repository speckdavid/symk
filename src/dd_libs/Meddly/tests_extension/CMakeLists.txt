cmake_minimum_required(VERSION 3.0)
set(CMAKE_AUTOMOC ON)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})


# Meddly includes
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../include
                    ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension)

# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests meddly_tests.cpp
               assignments.cpp
               randomTest.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/userOperations.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/evplus.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/minus.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/multiply.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/divide.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/unionmin.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/intersectionmax.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/greaterThan.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/greaterEquals.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/lessThan.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/lessEquals.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/equals.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/pow.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/partialcomplement.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/restrict.cc
               ${CMAKE_CURRENT_SOURCE_DIR}/../operations_extension/swapVar.cc)
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libmeddly.a)
