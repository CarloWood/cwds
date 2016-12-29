AM_CPPFLAGS = -iquote $(top_srcdir)

noinst_LTLIBRARIES = libdebug.la

libdebug_la_SOURCES = \
	debug.cxx \
	debug.h \
	debug_ostream_operators.cxx \
	debug_ostream_operators.h

libdebug_la_CXXFLAGS = @LIBCWD_FLAGS@
libdebug_la_LIBADD = @LIBCWD_LIBS@

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
