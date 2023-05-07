cmake_minimum_required(VERSION 3.12...3.25)

project(cwds
  LANGUAGES CXX
  DESCRIPTION "Application-side libcwd support code"
)

include(AICxxProject OPTIONAL RESULT_VARIABLE _module_path)
if (NOT _module_path)
  message(FATAL_ERROR
      "include could not find load file: AICxxProject\n"
      "1) Make sure that you have the git submodule https://github.com/CarloWood/cwm4 in the top-level directory.\n"
      "2) Use `include(cwm4/cmake/AICxxProject)` right below the `project(...)` command in your top-level CMakeLists.txt."
  )
endif ()

if (NOT AICxxSubmodules)
  message( FATAL_ERROR
      "Please do not use `add_subdirectory(...)` for any of the AICxx modules.\n"
      "Instead, use `include(AICxxSubmodules)` in your top-level CMakeLists.txt."
  )
endif ()

# Check if the compiler supports the benchmark assembly code.
try_compile(BENCHMARK_SUPPORTED
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cmake_benchmark_test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.cxx
  LOG_DESCRIPTION "Checking if benchmark asm is supported"
  CXX_STANDARD 20
  CMAKE_FLAGS "-DCMAKE_BUILD_TYPE=Release" "-DINCLUDE_DIRECTORIES=${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE OUTPUT)

if (BENCHMARK_SUPPORTED)
  message(STATUS "Benchmark code enabled")
else ()
  message(STATUS "Benchmark code disabled")
  message(DEBUG "Compiling cmake_benchmark_test.cpp failed with ${OUTPUT}")
endif ()

#==============================================================================
# DEPENDENCIES
#

# We depend on these even when utils is not part of the project.

# Compile with -pthread.
find_package(Threads REQUIRED)

# debug_ostream_operators.h: #include <boost/shared_ptr.hpp>
#                            #include <boost/weak_ptr.hpp>
# tracked.h:                 #include <boost/range/adaptor/reversed.hpp>
find_package(Boost REQUIRED)

# If utils is part of the project then we depend on it only if EnableDebugGlobal is ON.
# However, utils depends on cwds. To avoid a circular dependency we do never include
# the dependency on AICxx::utils. This is ok because utils is an OBJECT library and
# adding a dependency on it here would only cause its INTERFACE to be inherited,
# which happens to be Threads and Boost, which we depend on anyway, and the include
# directory of utils for sys.h and debug.h is the same, so we should get that too
# already (this means however that utils MUST be in project root, as does cwds).

#==============================================================================
# BUILD PROJECT
#

# This project is an OBJECT-library, used by other git submodules and the main project.
add_library(cwds_ObjLib OBJECT)

# Set SOURCE_DIR to the current directory.
set_property(
  TARGET cwds_ObjLib
  PROPERTY SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

# The list of source files.
target_sources(cwds_ObjLib
  PRIVATE
    "debug.cxx"
    "debug_ostream_operators.cxx"
    "signal_safe_printf.cxx"
    "UsageDetector.cxx"

    "sys.h"
    "debug.h"
    "debug_ostream_operators.h"
    "FrequencyCounter.h"
    "gnuplot_tools.h"
    "signal_safe_printf.h"
    "tracked.h"
    "tracked_intrusive_ptr.h"
    "UsageDetector.h"
)

if (BENCHMARK_SUPPORTED)

# Add the benchmark files to the project.
target_sources(cwds_ObjLib
  PRIVATE
    "benchmark.cxx"
    "benchmark.h"
)

# Always compile benchmark.cxx with -O3.
set_source_files_properties("benchmark.cxx" PROPERTIES COMPILE_OPTIONS "-O3")

else (BENCHMARK_SUPPORTED)

target_compile_definitions(cwds_ObjLib INTERFACE BENCHMARK_UNSUPPORTED)

endif (BENCHMARK_SUPPORTED)

# Required include search-paths.
target_include_directories(cwds_ObjLib
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."        # For local sys.h and debug.h.
    "${CMAKE_CURRENT_SOURCE_DIR}"           # For fall-back sys.h and debug.h.
)

# Require support for C++20.
target_compile_features(cwds_ObjLib
  PUBLIC cxx_std_20
)

# Add dependencies.
target_link_libraries(cwds_ObjLib
  PUBLIC
    Boost::boost
    Threads::Threads
)

if (OptionEnableLibcwd)
  target_link_libraries(cwds_ObjLib
    PUBLIC
      ${libcwd_r_TARGET}
  )
endif ()

# Create an ALIAS target.
add_library(AICxx::cwds ALIAS cwds_ObjLib)

# Prepend this object library to the list.
set(AICXX_OBJECTS_LIST AICxx::cwds ${AICXX_OBJECTS_LIST} CACHE INTERNAL "List of OBJECT libaries that this project uses.")
