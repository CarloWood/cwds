AM_CPPFLAGS = -iquote $(top_srcdir)

noinst_LTLIBRARIES = libcwd.la

libcwd_la_SOURCES = \
	debug.cxx \
	debug.h \
	debug_ostream_operators.cxx \
	debug_ostream_operators.h

libcwd_la_CXXFLAGS = @LIBCWD_FLAGS@
libcwd_la_LIBADD = @LIBCWD_LIBS@

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
