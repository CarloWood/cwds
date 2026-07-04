# cwds submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing standard-ish support code for applications that use [libcwd](https://github.com/CarloWood/libcwd).

It provides the following features,
* defines all libcwd debug macros as empty (with the exception of `LibcwDoutFatal`) when not compiling with debug support.
* defines the extra helper macros:
  * `CWDEBUG_ONLY(...)`
  * `COMMA_CWDEBUG_ONLY(...)`
  * `DEBUG_ONLY(...)`
  * `COMMA_DEBUG_ONLY(...)`
  * `ASSERT(expr)`
* Defines a class tracked::Tracked<&name> that can be used to
  track proper use of move/copy constructors and assignment operators.
* Provides code for benchmarking (declared in `cwds/benchmark.h`).
* Support for plotting graphs (using gnuplot).
* Provides a function to print simple variables from a signal handler (`cwds/signal_safe_printf.h`).
* Defines a streambuf class that can be used to turn background color of all debug output green.
* Defines ostream serializers for many types to pretty-print them easily to a debug stream, like
  * `timeval`
  * `boost::shared_ptr<T>`
  * `boost::weak_ptr<T>`
  * `std::pair<T1, T2>`
  * `std::map<T1, T2, T3>`

## Checking out a project that uses the cwds submodule

To clone a project example-project that uses cwds simply run:

```
git clone --recursive <*URL-to-project*>/example-project.git
cd example-project
./autogen.sh
```

The `--recursive` is optional because `./autogen.sh` will fix
it when you forgot it.

## Adding the cwds submodule to a project

To add this submodule to a project, that project should already
be set up to use [cmake-aicxx](https://github.com/CarloWood/cmake-aicxx).

Then simply execute the following in the root of that project:

```
git submodule add https://github.com/CarloWood/cwds.git
```

This should clone cwds into the subdirectory `cwds`, or
if you already cloned it there, it should add it.

Typically you should use [gitache](https://github.com/CarloWood/gitache) to install libcwd itself.
In that case, set the `GITACHE_ROOT` environment variable as described in the documentation
of gitache and add the following to the root `CMakeLists.txt` file of your project:

```
 ... [cmake_minimum_required, project and CMAKE_CXX_STANDARD]
# Begin of gitache configuration.
set(GITACHE_PACKAGES libcwd)
include(cmake/aicxx/cmake/StableGitache)
# End of gitache configuration.

# This project uses aicxx modules.
include(cmake/aicxx/Project NO_POLICY_SCOPE)

include(AICxxSubmodules)
 ...
```

And add `${AICXX_OBJECTS_LIST}` to every target that uses cwds (aka, debugging).
For example,

```
add_executable(sum_first_n_primes sum_first_n_primes.cxx)
target_link_libraries(sum_first_n_primes PRIVATE ${AICXX_OBJECTS_LIST})
```

To use gitache for libcwd you also have to create the file `<project_root>/cmake/gitache-configs/libcwd.cmake`
with normally the following content (see [gitache](https://github.com/CarloWood/gitache) for more information):

```
gitache_config(
  GIT_REPOSITORY
    "https://github.com/CarloWood/libcwd.git"
  GIT_TAG
    "master"
  CMAKE_ARGS
    "-DEnableLibcwdLocation:BOOL=ON"
)
```

There is a tutorial video on youtube that shows how to set up a project with
cwds and libcwd [here](https://www.youtube.com/watch?v=9tQnhOGK0zo). Or just look at the [example-project](https://github.com/CarloWood/libcwd/tree/master/example-project) in the source tree of libcwd.
