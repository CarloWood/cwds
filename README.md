# cwds submodule

This repository is a [git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
providing standard-ish support code for applications that use [libcwd](https://github.com/CarloWood/libcwd).

It provides the following features,
* defines all libcwd debug macros as empty (with the exception of <tt>LibcwDoutFatal</tt>) when not compiling with debug support.
* defines the extra helper macros:
  * <tt>CWDEBUG_ONLY(...)</tt>
  * <tt>COMMA_CWDEBUG_ONLY(...)</tt>
  * <tt>DEBUG_ONLY(...)</tt>
  * <tt>COMMA_DEBUG_ONLY(...)</tt>
  * <tt>ASSERT(expr)</tt>
* and when compiled with debugging, the macros,
  * <tt>NAMESPACE_DEBUG</tt>
  * <tt>NAMESPACE_DEBUG_START</tt>
  * <tt>NAMESPACE_DEBUG_END</tt>
  * <tt>NAMESPACE_CHANNELS</tt>
  * <tt>DEBUGCHANNELS</tt>
  * <tt>NAMESPACE_DEBUG_CHANNELS_START</tt>
  * <tt>NAMESPACE_DEBUG_CHANNELS_END</tt>
  * <tt>DoutEntering</tt>
* Declares initialization functions for libcwd to be called from the top of <tt>main</tt> and the start of threads.
* Defines a global mutex to be used for <tt>std::cout</tt> (if you want output to std::cout not to interfer with debug output).
* Defines a class tracked::Tracked<&name> that can be used to
  track proper use of move/copy constructors and assignment operators.
* Provides a function to turn an address into a string with filename and line number.
* Provides code for benchmarking (declared in <tt>cwds/benchmark.h</tt>).
* Support for plotting graphs (using gnuplot).
* Defines exception safe struct for indenting debug output (<tt>Indent</tt>) and making temporarily making allocation invisible (<tt>InvisibleAllocations</tt>).
* Provides a function to print simple variables from a signal handler (<tt>cwds/signal_safe_printf.h</tt>).
* Defines a streambuf class that can be used to turn background color of all debug output green.
* Defines ostream serializers for many types to pretty-print them easily to a debug stream, like
  * <tt>timeval</tt>
  * <tt>boost::shared_ptr&lt;T&gt;</tt>
  * <tt>boost::weak_ptr&lt;T&gt;</tt>
  * <tt>std::pair&lt;T1, T2&gt;</tt>
  * <tt>std::map&lt;T1, T2, T3&gt;</tt>

## Checking out a project that uses the cwds submodule

To clone a project example-project that uses cwds simply run:

<pre>
<b>git clone --recursive</b> &lt;<i>URL-to-project</i>&gt;<b>/example-project.git</b>
<b>cd example-project</b>
<b>./autogen.sh</b>
</pre>

The <tt>--recursive</tt> is optional because <tt>./autogen.sh</tt> will fix
it when you forgot it.

## Adding the cwds submodule to a project

To add this submodule to a project, that project should already
be set up to use [cmake-aicxx](https://github.com/CarloWood/cmake-aicxx).

Then simply execute the following in the root of that project:

<pre>
git submodule add https://github.com/CarloWood/cwds.git
</pre>

This should clone cwds into the subdirectory <tt>cwds</tt>, or
if you already cloned it there, it should add it.

Typically you should use [gitache](https://github.com/CarloWood/gitache) to install libcwd itself.
In that case, set the <tt>GITACHE_ROOT</tt> environment variable as described in the documentation
of gitache and add the following to the root <tt>CMakeLists.txt</tt> file of your project:

<pre>
 ... [cmake_minimum_required, project and CMAKE_CXX_STANDARD]
# Begin of gitache configuration.
set(GITACHE_PACKAGES libcwd)
include(cmake/aicxx/cmake/StableGitache)
# End of gitache configuration.

# This project uses aicxx modules.
include(cmake/aicxx/Project NO_POLICY_SCOPE)

include(AICxxSubmodules)
 ...
</pre>

And add <tt>${AICXX_OBJECTS_LIST}</tt> to every target that uses cwds (aka, debugging).
For example,

<pre>
add_executable(sum_first_n_primes sum_first_n_primes.cxx)
target_link_libraries(sum_first_n_primes PRIVATE ${AICXX_OBJECTS_LIST})
</pre>

To use gitache for libcwd you also have to create the file <tt>`<project_root>/`cmake/gitache-configs/libcwd.cmake</tt>
with normally the following content (see [gitache](https://github.com/CarloWood/gitache) for more information):

<pre>
gitache_config(
  GIT_REPOSITORY
    "https://github.com/CarloWood/libcwd.git"
  GIT_TAG
    "master"
  CMAKE_ARGS
    "-DEnableLibcwdLocation:BOOL=ON"
)
</pre>

There is a tutorial video on youtube that shows how to set up a project with
cwds and libcwd [here](https://www.youtube.com/watch?v=9tQnhOGK0zo).
