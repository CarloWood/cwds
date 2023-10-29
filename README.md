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
be set up to use [cwm4](https://github.com/CarloWood/cwm4).

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
set(GITACHE_PACKAGES libcwd_r)
include(cwm4/cmake/StableGitache)
# End of gitache configuration.

# This project uses aicxx modules.
include(cwm4/cmake/AICxxProject)

include(AICxxSubmodules)
 ...
</pre>

And add <tt>${AICXX_OBJECTS_LIST}</tt> to every target that uses cwds (aka, debugging).
For example,

<pre>
add_executable(sum_first_n_primes sum_first_n_primes.cxx)
target_link_libraries(sum_first_n_primes PRIVATE ${AICXX_OBJECTS_LIST})
</pre>

To use gitache for libcwd you also have to create the file <tt>`<project_root>/`cmake/gitache-configs/libcwd_r.cmake</tt>
with normally the following content (see [gitache](https://github.com/CarloWood/gitache) for more information):

<pre>
gitache_config(
  GIT_REPOSITORY
    "https://github.com/CarloWood/libcwd.git"
  GIT_TAG
    "master"
  CMAKE_ARGS
    "-DEnableLibcwdAlloc:BOOL=OFF -DEnableLibcwdLocation:BOOL=ON"
)
</pre>

As described in the documentation of [libcwd](https://github.com/CarloWood/libcwd),
each (C++) source file must begin with <tt>#include "sys.h"</tt> and
use <tt>#include "debug.h"</tt> when containing any debug code.
This submodule provides those header files, but there is some room for tuning
the namespace used for the debugging specific code of the application.

The following namespaces are relevant:

<pre>
namespace example {
  namespace debug {
    namespace channels {
      namespace dc {
</pre>

Where <tt>namespace example</tt> is optional, in fact, anything
inside namespace <tt>debug</tt> can be put anywhere, we're just using
<tt>example::debug</tt> as the example right now.

Also the name of <tt>namespace channels</tt> can be changed, although
that is only necessary in special cases when writing a library that
wishes to reuse the name of an existing debug channel. You'll know
when you need that, until then I suggest you'll just leave it at
<tt>channels</tt>.

By default (which is ok for any application, but not for libraries)
the debug namespace is just <tt>debug::</tt>, thus no <tt>namespace example</tt>.
This is the case when directly including the <tt>debug.h</tt> provided
by this submodule. Hence, a library should provide yet another debug.h
file and make sure that is first in the include path. Other submodules,
which don't know if such a debug.h is provided should therefore use
the following order of include paths: first the top build directory,
then the top source directory and last the <tt>cwds</tt> directory in
the top source directory.

So that library projects (or applications) can put a <tt>debug.h</tt>
in their top source directory that contains something like

<pre>
#pragma once

// These three defines are only necessary if you want the 'example' namespace.
// The default is just 'debug'.
#define NAMESPACE_DEBUG example::debug
#define NAMESPACE_DEBUG_START namespace example { namespace debug {
#define NAMESPACE_DEBUG_END } }
#include "cwds/debug.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct my_channel;
extern channel_ct ...
NAMESPACE_DEBUG_CHANNELS_END
#endif
</pre>

or if they don't, that then <tt>cwds/debug.h</tt> will be included
directly. The top build directory include is needed to find
any generated header files, most notably <tt>sys.h</tt>.

Debug channels that are only used in a single compilation unit can be
defined by just adding

<pre>
#include "sys.h"
#include "debug.h"

[...]

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
channel_ct my_channel;
NAMESPACE_DEBUG_CHANNELS_END
#endif
</pre>

of the respective source file, of course.

In order to initialize libcwd properly, the following has to be added
to the top of <tt>main</tt>:

<pre>
int main()
{
  Debug(NAMESPACE_DEBUG::init());
</pre>

The first three lines are actually something from [ai-utils](https://github.com/CarloWood/ai-utils),
just shown here to show the typical order.

Threads on the other hand need to begin with the following code:

<pre>
  Debug(NAMESPACE_DEBUG::init_thread());
</pre>

Finally, run

<pre>
./autogen.sh
</pre>

to let cwm4 do its magic, and commit all the changes.

There is a tutorial video on youtube that shows how to set up a project with
cwds and libcwd [here](https://www.youtube.com/watch?v=53wWV0wqOMA&list=PLJzCXkV5Y8Ze6TtQWQSH6w5J-e-yIhgzP&index=2).
