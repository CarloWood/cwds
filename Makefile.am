AM_CPPFLAGS = -iquote $(top_builddir) -iquote $(top_srcdir)

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
	debug_ostream_operators.h

libcwds_la_SOURCES = ${SOURCES}
libcwds_la_CXXFLAGS = @LIBCWD_FLAGS@
libcwds_la_LIBADD = @LIBCWD_LIBS@

libcwds_r_la_SOURCES = ${SOURCES}
libcwds_r_la_CXXFLAGS = @LIBCWD_R_FLAGS@
libcwds_r_la_LIBADD = @LIBCWD_R_LIBS@

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
