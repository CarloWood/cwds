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

## Checking out a project that uses the cwds submodule.

To clone a project example-project that uses cwds simply run:

<pre>
<b>git clone --recursive</b> &lt;<i>URL-to-project</i>&gt;<b>/example-project.git</b>
<b>cd example-project</b>
<b>./autogen.sh</b>
</pre>

The <tt>--recursive</tt> is optional because <tt>./autogen.sh</tt> will fix
it when you forgot it.

Afterwards you probably want to use <tt>--enable-mainainer-mode</tt>
as option to the generated <tt>configure</tt> script. Note that <tt>--enable-mainainer-mode</tt>
enables debugging by default (<tt>--enable-debug</tt>), and will try to use libcwd when
available; <tt>configure</tt> will check for the existence of libcwd and use it when
found. If you want to use maintainer-mode without debugging then configure
using <tt>--enable-mainainer-mode --disable-debug</tt>, or if you want to enforce
using libcwd, with or without maintainer-mode, then use <tt>--enable-libcwd</tt>.

## Adding the cwds submodule to a project

To add this submodule to a project, that project should already
be set up to use [cwm4](https://github.com/CarloWood/cwm4).

Simply execute the following in the root of that project:

<pre>
git submodule add https://github.com/CarloWood/cwds.git
</pre>

This should clone cwds into the subdirectory <tt>cwds</tt>, or
if you already cloned it there, it should add it.

Changes to <tt>configure.ac</tt> and <tt>Makefile.am</tt>
are taken care of by <tt>cwm4</tt>, except for linking
which works as usual.

For example a module that defines a

<pre>
bin_PROGRAMS = singlethreaded_foobar multithreaded_foobar
</pre>

would also define

<pre>
singlethreaded_foobar_SOURCES = singlethreaded_foobar.cpp
singlethreaded_foobar_CXXFLAGS = @LIBCWD_FLAGS@
singlethreaded_foobar_LDADD = $(top_builddir)/cwds/libcwds.la

multithreaded_foobar_SOURCES = multithreaded_foobar.cpp
multithreaded_foobar_CXXFLAGS = @LIBCWD_R_FLAGS@
multithreaded_foobar_LDADD = $(top_builddir)/cwds/libcwds_r.la
</pre>

or whatever the path to `cwds` etc. is, to link with cwds and libcwd.

The availability of libcwds.la and/or libcwds_r.la is determined by the second
parameter of the <tt>CW_OPG_CXXFLAGS</tt> macro in <tt>configure.ac</tt>;
which can be <tt>[no]</tt> (single-threaded) in which case only <tt>libcwds.la</tt>
is available, <tt>[yes]</tt> (multi-threaded) in which case only <tt>libcwds_r.la</tt>
is available, or <tt>[both]</tt> in which case both are available.
See the [cwm4](https://github.com/CarloWood/cwm4) submodule for more details.

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
in their <tt>Makefile.am</tt>:

<pre>
AM_CPPFLAGS = -iquote $(top_builddir) -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds
</pre>

So that library projects (or applications) can put a <tt>debug.h</tt>
in <tt>$(top_srcdir)</tt> that contains something like

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
directly. The <tt>-iquote $(top_builddir)</tt> is needed to find
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
