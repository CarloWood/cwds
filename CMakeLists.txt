cmake_minimum_required( VERSION 3.12...3.16 )

project( cwds
         LANGUAGES CXX
         DESCRIPTION "Application-side libcwd support code" )

include( AICxxProject OPTIONAL RESULT_VARIABLE _module_path )
if ( NOT _module_path )
  message( FATAL_ERROR
      "include could not find load file: AICxxProject\n"
      "1) Make sure that you have the git submodule https://github.com/CarloWood/cwm4 in the top-level directory.\n"
      "2) Use `include(cwm4/cmake/AICxxProject)` right below the `project(...)` command in your top-level CMakeLists.txt."
  )
endif ()

if ( NOT AICxxSubmodules )
  message( FATAL_ERROR
      "Please do not use `add_subdirectory(...)` for any of the AICxx modules.\n"
      "Instead, use `include(AICxxSubmodules)` in your top-level CMakeLists.txt."
  )
endif ()

#==============================================================================
# DEPENDENCIES
#

# We depend on these even when utils is not part of the project.

# Compile with -pthread.
find_package( Threads REQUIRED )

# debug_ostream_operators.h: #include <boost/shared_ptr.hpp>
#                            #include <boost/weak_ptr.hpp>
# tracked.h:                 #include <boost/range/adaptor/reversed.hpp>
hunter_add_package( Boost )
find_package( Boost REQUIRED )

# If utils is part of the project then we depend on it only if EnableDebugGlobal is ON.
# However, utils depends on cwds. To avoid a circular dependency we do never include
# the dependcy on AICxx::utils. This is ok because utils is an OBJECT library and
# adding a dependency on it here would only cause its INTERFACE to be inherited,
# which happens to be Threads and Boost, which we depend on anyway, which the include
# directory of utils is the same as that for sys.h and debug.h, so we should get that
# too already (this means however that utils MUST be in project root, as does cwds).

#==============================================================================
# BUILD PROJECT
#

# This project is an OBJECT-library, used by other git submodules and the main project.
add_library( cwds_ObjLib OBJECT )

# Set SOURCE_DIR to the current directory.
set_property(
    TARGET     cwds_ObjLib
    PROPERTY   SOURCE_DIR       "${CMAKE_CURRENT_SOURCE_DIR}"
)

# The list of source files.
target_sources( cwds_ObjLib
    PRIVATE
        "benchmark.cxx"
        "debug.cxx"
        "debug_ostream_operators.cxx"
        "signal_safe_printf.cxx"
        "sys.h"

        "benchmark.h"
        "debug.h"
        "debug_ostream_operators.h"
        "FrequencyCounter.h"
        "gnuplot_tools.h"
        "signal_safe_printf.h"
        "tracked.h"
)

# Always compile benchmark.cxx with -O3.
set_source_files_properties( "benchmark.cxx" PROPERTIES COMPILE_OPTIONS "-O3")

# Required include search-paths.
target_include_directories( cwds_ObjLib
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.."        # For local sys.h and debug.h.
        "${CMAKE_CURRENT_SOURCE_DIR}"           # For fall-back sys.h and debug.h.
)

# Require support for C++17.
target_compile_features( cwds_ObjLib
    PUBLIC cxx_std_17
)

# Add dependencies.
target_link_libraries( cwds_ObjLib
    PUBLIC
        Boost::boost
        Threads::Threads
)

if (OptionEnableLibcwd)
  target_link_libraries( cwds_ObjLib
      PUBLIC
          PkgConfig::Libcwd_r
  )
endif ()

# Create an ALIAS target.
add_library( AICxx::cwds ALIAS cwds_ObjLib )

# Prepend this object library to the list.
set( AICXX_OBJECTS_LIST AICxx::cwds ${AICXX_OBJECTS_LIST} CACHE INTERNAL "List of OBJECT libaries that this project uses." )
