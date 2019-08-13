# cwds depends on utils if --disable-global-debug is not specified and utils exists.
m4_sinclude([utils/configure_cwds.m4])

m4_if(cwm4_submodule_dirname, [], [m4_append_uniq([CW_SUBMODULE_SUBDIRS], cwm4_submodule_basename, [ ])])
m4_append_uniq([CW_SUBMODULE_CONFIG_FILES], cwm4_quote(cwm4_submodule_path[/Makefile]), [ ])

AC_SUBST(CW_EXTRA_CWDS_LIBS)
AC_SUBST(CW_EXTRA_CWDS_R_LIBS)

# Require at least version 1.1.1.
if ! pkg-config --print-errors --atleast-version="1.1.1" libcwd_r; then
  AC_MSG_ERROR([Please upgrade libcwd.])
fi

dnl vim: filetype=config
