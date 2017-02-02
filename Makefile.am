AM_CPPFLAGS = -iquote $(top_srcdir)

noinst_LTLIBRARIES =
if CW_NON_THREADED
noinst_LTLIBRARIES += libcwd.la
endif
if CW_THREADED
noinst_LTLIBRARIES += libcwd_r.la
endif

SOURCES = \
	debug.cxx \
	debug.h \
	debug_ostream_operators.cxx \
	debug_ostream_operators.h

libcwd_la_SOURCES = ${SOURCES}
libcwd_la_CXXFLAGS = @LIBCWD_FLAGS@
libcwd_la_LIBADD = @LIBCWD_LIBS@

libcwd_r_la_SOURCES = ${SOURCES}
libcwd_r_la_CXXFLAGS = @LIBCWD_R_FLAGS@
libcwd_r_la_LIBADD = @LIBCWD_R_LIBS@

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
