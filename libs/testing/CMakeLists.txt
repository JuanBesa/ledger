#
# F E T C H   T E S T I N G   L I B R A R Y
#
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(fetch-testing)

# CMake Configuration
include(${FETCH_ROOT_CMAKE_DIR}/BuildTools.cmake)

# Compiler Configuration
setup_compiler()

# ------------------------------------------------------------------------------
# Main Library Target
# ------------------------------------------------------------------------------

setup_library(fetch-testing)
add_test_target()
target_link_libraries(fetch-testing INTERFACE fetch-core fetch-logging)
