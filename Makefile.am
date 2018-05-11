# This submodule needs a special trick to include sys.h and debug.h from $(top_builddir) and/or
# $(top_srcdir) if they exist there, instead of from $(srcdir).
# Using double quotes always first looks in the current directory, so we need to include <sys.h>,
# and <debug.h> respectively in this source tree and then use -I instead of -iquote to find them.
#
# Also, we need to overwrite DEFAULT_INCLUDES to obtain the correct order (instead of using AM_CPPFLAGS).
DEFAULT_INCLUDES = -I $(top_builddir) -I $(top_srcdir) -I $(srcdir)

noinst_LTLIBRARIES =
if CW_NON_THREADED
noinst_LTLIBRARIES += libcwds.la
endif
if CW_THREADED
noinst_LTLIBRARIES += libcwds_r.la
endif

SOURCES = \
	debug.cxx \
	debug.h \
	debug_ostream_operators.cxx \
	debug_ostream_operators.h \
	signal_safe_printf.cxx \
	signal_safe_printf.h \
	benchmark.cxx \
	benchmark.h \
	tracked.h

libcwds_la_SOURCES = ${SOURCES}
libcwds_la_CXXFLAGS = -std=c++11 @LIBCWD_FLAGS@
libcwds_la_LIBADD = @LIBCWD_LIBS@

libcwds_r_la_SOURCES = ${SOURCES}
libcwds_r_la_CXXFLAGS = -std=c++11 @LIBCWD_R_FLAGS@
libcwds_r_la_LIBADD = @LIBCWD_R_LIBS@

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
