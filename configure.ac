AC_INIT([gperf-all-threads], 0.1)
AC_CONFIG_FILES([Makefile])

PKG_CHECK_MODULES([libprofiler], [libprofiler],[],[
  libprofiler_CFLAGS="-pthread"
  libprofiler_LIBS="-lprofiler -pthread"
])

AC_OUTPUT
