dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.53)

AC_INIT(SynCE Core, 0.17)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR(lib/utils/synce.h.in)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE

CFLAGS="-Wall -Wsign-compare -Wno-long-long $CFLAGS"

case $target in

	powerpc-apple-*)
		dnl Prevent "Undefined Macro argument list" error.
		CFLAGS="-no-cpp-precomp $CFLAGS"
		;;

	*)
		;;

esac

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

dnl Find the path to network tools
AC_PATH_PROG([DHCLIENTPATH], [dhclient], [no])
if test "x$DHCLIENTPATH" = "xno"; then
   AC_MSG_ERROR([Failed to find dhclient in PATH, is it installed ?])
fi

AC_PATH_PROG([IFCONFIGPATH], [ifconfig], [no])
if test "x$IFCONFIGpath" = "xno"; then
   AC_MSG_ERROR([Failed to find ifconfig in PATH, is it installed ?])
fi

AC_PATH_PROG([PPPDPATH], [pppd], [/usr/sbin/pppd])

dnl AM_PATH_CHECK()

dnl Check for dmalloc first
AC_ARG_WITH(dmalloc,
		AC_HELP_STRING([--with-dmalloc=PREFIX],[use dmalloc (default is NO)]),
[case "${withval}" in
  yes) dmalloc=true ;;
  no)  dmalloc=false ;;
  *) dmalloc=true; dmalloc_prefix="-L${withval}" ;;
esac],[dmalloc=false])

if ${dmalloc}; then
	AC_CHECK_LIB(dmalloc,main,,[
			AC_MSG_ERROR([Can't find dmalloc library])
		],${dmalloc_prefix})
	AC_CHECK_HEADERS(dmalloc.h,,[
			AC_MSG_ERROR([Can't find dmalloc.h])
		])
fi

dnl Checks for libraries.
AC_LIB_RPATH

dnl Check for GLib
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.7, gobject-2.0 >= 2.4, gio-2.0 >= 2.22, gio-unix-2.0 >= 2.22])

dnl Check for D-Bus, we need dbus-1 for config file locations
dnl irrespective of which dbus library we use
PKG_CHECK_MODULES(DBUS, [dbus-1 >= 0.60], dbus_found=yes, dbus_found=no)
PKG_CHECK_MODULES(DBUS, [dbus-glib-1 >= 0.74], dbusglib_found=yes, dbusglib_found=no)

dnl Check if GLib version is high enough for GDBus
AC_MSG_CHECKING([whether to use GDBus])
PKG_CHECK_EXISTS([gio-2.0 >= 2.26], have_gdbus=yes, have_gdbus=no)
if test "$have_gdbus" = "yes"; then
   use_gdbus=1;
   AC_MSG_RESULT([yes])
else
   use_gdbus=0;
   AC_MSG_RESULT([no])
fi
AC_DEFINE_UNQUOTED(USE_GDBUS, $use_gdbus, [Define to 1 if you want to use GDBus support.])
AM_CONDITIONAL(USE_GDBUS, test "x$use_gdbus" = "x1")

dnl
dnl whether to build documentation
dnl
AC_CHECK_PROGS([DOXYGEN], [doxygen])

AC_ARG_ENABLE(generate-docs,
              [  --enable-generate-docs  build documentation (requires doxygen)],
              enable_generate_docs=$enableval, enable_generate_docs=no)

if ! test "$enable_generate_docs" = "no"; then
  if test -z "$DOXYGEN"; then
    AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
    enable_generate_docs="no"
  else
    AC_MSG_NOTICE([Building doxygen documentation])
    enable_generate_docs="yes"
  fi
else
  AC_MSG_NOTICE([Documentation not requested])
fi
AM_CONDITIONAL([BUILD_DOCS], [test "$enable_generate_docs" = "yes"])


dnl
dnl whether (v)dccm connection file support is required
dnl
AC_ARG_ENABLE(dccm-file-support,
              [  --enable-dccm-file-support  enable dccm file support],
              enable_dccm_file_support=$enableval, enable_dccm_file_support=no)

if ! test "$enable_dccm_file_support" = "no"; then
  AC_MSG_NOTICE([Building with dccm file support])
  AC_DEFINE(ENABLE_DCCM_FILE_SUPPORT, 1, [Define if building with dccm file support])
  enable_dccm_file_support=yes
else
  AC_MSG_NOTICE([Building without dccm file support])
fi

if test "$enable_dccm_file_support" = "yes"; then dccm_file=1; else dccm_file=0; fi
AC_DEFINE_UNQUOTED(ENABLE_DCCM_FILE_SUPPORT, $dccm_file, [Define to 1 if you want dccm file support.])

dnl
dnl whether odccm dbus support is required
dnl
AC_ARG_ENABLE(odccm-support,
              [  --enable-odccm-support  enable odccm support (requires D-Bus)],
              enable_odccm_support=$enableval, enable_odccm_support=no)

if ! test "$enable_odccm_support" = "no"; then
  dnl do we have dbus ?
  if test "$use_gdbus" = "no" -a "$dbusglib_found" = "no"; then
    if test "$enable_odccm_support" = "auto"; then
      AC_MSG_NOTICE([D-Bus Glib not found, building without odccm support])
      enable_odccm_support=no
    else
      AC_MSG_ERROR([odccm support requested but D-Bus Glib could not be found])
    fi
  else
    AC_MSG_NOTICE([Building with odccm support])
    AC_DEFINE(ENABLE_ODCCM_SUPPORT, 1, [Define if building with odccm support])
    enable_odccm_support=yes
  fi
else
  AC_MSG_NOTICE([Building without odccm support])
fi

if test "$enable_odccm_support" = "yes"; then odccm=1; else odccm=0; fi
AC_DEFINE_UNQUOTED(ENABLE_ODCCM_SUPPORT, $odccm, [Define to 1 if you want odccm support.])


dnl
dnl whether dbus support for udev is required
dnl
AC_ARG_ENABLE(udev-support,
              [  --disable-udev-support  enable udev support (requires D-Bus)],
              enable_udev_support=$enableval, enable_udev_support=yes)

if ! test "$enable_udev_support" = "no"; then
  dnl do we have dbus library support ?
  if test "$use_gdbus" = "no" -a "$dbusglib_found" = "no"; then
    if test "$enable_udev_support" = "auto"; then
      AC_MSG_NOTICE([D-Bus Glib not found, building without udev support])
      enable_udev_support=no
    else
      AC_MSG_ERROR([udev support requested but D-Bus Glib could not be found])
    fi
  dnl do we have dbus ?
  elif test "$dbus_found" = "no"; then
    if test "$enable_udev_support" = "auto"; then
      AC_MSG_NOTICE([D-Bus not found, building without udev support])
      enable_udev_support=no
    else
      AC_MSG_ERROR([udev support requested but D-Bus could not be found])
    fi
  else
    AC_MSG_NOTICE([Building with udev support])
    AC_DEFINE(ENABLE_UDEV_SUPPORT, 1, [Define if building with udev support])
    enable_udev_support=yes
  fi
else
  AC_MSG_NOTICE([Building without udev support])
fi

if test "$enable_udev_support" = "yes"; then udev=1; else udev=0; fi
AC_DEFINE_UNQUOTED(ENABLE_UDEV_SUPPORT, $udev, [Define to 1 if you want udev support.])
AM_CONDITIONAL(ENABLE_UDEV, test "x$udev" = "x1")


# check for libgudev setup
PKG_CHECK_MODULES(GUDEV, gudev-1.0, have_gudev=1, have_gudev=0)
AC_DEFINE_UNQUOTED(HAVE_GUDEV, $have_gudev, [Define to 1 for libgudev support.])

PKG_CHECK_MODULES(UDEV, udev)

dnl Find the path to udevadm
AC_PATH_PROG([UDEVADMPATH], [udevadm], [no])
if test "x$UDEVADMPATH" = "xno"; then
    AC_MSG_ERROR([Failed to find udevadm in PATH, is it installed ?])
fi

dnl Check for override of default udev dir
AC_MSG_CHECKING([udev dir])
AC_ARG_WITH(udev-dir, [  --with-udev-dir=PATH     Location of the udev config.],
       with_udev_dir="$withval", with_udev_dir="no")
if test "x${with_udev_dir}" = "xno"; then
    udev_dir="`pkg-config --variable=udevdir udev`"
    AC_MSG_RESULT([$udev_dir])
else
    udev_dir="$with_udev_dir"
    AC_MSG_RESULT([$udev_dir])
fi
AC_SUBST(udev_dir)


AC_ARG_ENABLE(bluetooth-support, AC_HELP_STRING([--enable-bluetooth-support], [Build in bluetooth support]),enable_bluetooth_support="$enableval",enable_bluetooth_support=no)
AM_CONDITIONAL(ENABLE_BLUETOOTH_SUPPORT, test "x$enable_bluetooth_support" = "xyes")
if test "x$enable_bluetooth_support" = "xyes"; then
       AC_DEFINE(ENABLE_BLUETOOTH_SUPPORT, 1, [Building in bluetooth support])
fi

dnl Checks for header files.
AC_STDC_HEADERS

dnl #
dnl # Now we must check for Python/Pyrex
dnl #
dnl # (need python, python headers, and pyrex)

AC_ARG_ENABLE(python-bindings, AS_HELP_STRING([--disable-python-bindings], [do not build python bindings]),[enable_python=$enableval], [enable_python=auto])

if test x$enable_python = xno; then
    have_python=no
else
    AC_MSG_NOTICE([Checking to see if we can build Python bindings])
    have_python=no
    if test x$enable_python != xyes -a x$enable_python != xauto; then
        minimum_version=$enable_python
	enable_python=yes
    else
        minimum_version=2.3
    fi
	AM_PATH_PYTHON($minimum_version, [have_python_path=yes], [have_python_path=no] )
	AC_CHECK_PROGS(PYREX, pyrexc)
	AM_CHECK_PYTHON_HEADERS([have_python_header=yes], [have_python_header=no])
	
	if test x$have_python_path = xno -o x$have_python_header = xno -o x$PYREX != xpyrexc ; then
            if test x$enable_python = xyes ; then
                AC_MSG_ERROR([Building python bindings requested, but can't build them])
            else
                AC_MSG_NOTICE([Not building Python bindings])
            fi
	else
	    if test -z "$PYTHON" ; then
	        AC_MSG_WARN([Python not found])
	    else
		have_python=yes
	    fi
	
	    if test x$have_python = xno ; then
	        if test x$enable_python = xyes ; then
	            AC_MSG_ERROR([Building python explicitly requested, but can't build python bindings])
		else
		    AC_MSG_NOTICE([Not building Python bindings])
	        fi
	    else

dnl # require at least pyrex 0.9.6

		AC_MSG_CHECKING([pyrex version])

		pyrex_ver=`$PYREX --version 2>&1 | sed -e 's/Pyrex version //'`
	 	`echo "$pyrex_ver" | awk 'BEGIN { FS = "."; } { printf "export pyrex_maj_ver=%s export pyrex_min_ver=%s export pyrex_sub_ver=%s", $1, $2, $3;}'`

		if test "$pyrex_min_ver" -lt 9 || test "$pyrex_sub_ver" -lt 6 ; then
		    AC_MSG_RESULT([$pyrex_ver])
		    if test x$enable_python = xyes ; then
			AC_MSG_ERROR([Building python explicitly requested, but can't build python bindings, need at least pyrex 0.9.6])
		    else
			AC_MSG_NOTICE([Not building Python bindings, need at least pyrex 0.9.6])
			have_python=no
	            fi
    		else
		    AC_MSG_RESULT([ok])
    		fi

	    fi
	fi
fi
AM_CONDITIONAL(HAVE_PYTHON, test x$have_python = xyes)

# checks for pthreads

saved_CFLAGS="$CFLAGS"
[CFLAGS=`echo $CFLAGS | sed 's|-Werror[^[:space:]]*||'`]

ACX_PTHREAD([enable_threads="pthread"],[enable_threads="no"])
CFLAGS="$saved_CFLAGS"
if test $enable_threads != "pthread"; then
	AC_MSG_ERROR([unable to find pthreads, currently this is required])
else
	AC_DEFINE(HAVE_PTHREAD,1,[Define if you have POSIX threads libraries and header files.])
	LIBS="$PTHREAD_LIBS $LIBS"
	CFLAGS="$PTHREAD_CFLAGS $CFLAGS"
	CXXFLAGS="$PTHREAD_CFLAGS $CXXFLAGS"
fi


dnl Endian stuff
AC_SUBST(WORDS_BIGENDIAN)
AC_SUBST(BYTESWAP_HEADER,dummy)
AC_SUBST(HAVE_BYTESWAP_HEADER,0)

AC_C_BIGENDIAN()

if test "$ac_cv_c_bigendian" = yes; then
  WORDS_BIGENDIAN=1
else
  WORDS_BIGENDIAN=0
fi

AC_CHECK_HEADERS(sys/byteswap.h,[BYTESWAP_HEADER=sys/byteswap.h; HAVE_BYTESWAP_HEADER=1])
AC_CHECK_HEADERS(byteswap.h,    [BYTESWAP_HEADER=byteswap.h;     HAVE_BYTESWAP_HEADER=1])

AM_CONDITIONAL(IMPLEMENT_BSWAP_XX, test "x$HAVE_BYTESWAP_HEADER" = "x0")

dnl Check where uint16_t and uint32_t may be found
AC_SUBST(INT_HEADER)

dnl Old compilers
AC_CHECK_HEADERS(inttypes.h,[INT_HEADER=inttypes.h])

dnl C99 compliant compilers
AC_CHECK_HEADERS(stdint.h,[INT_HEADER=stdint.h])

if test -z "$INT_HEADER"; then
	AC_MSG_ERROR([Don't know how to get uint16_t and uint32_t on your system])
fi

AC_CHECK_HEADERS(stdbool.h,,[
  AC_MSG_ERROR([Don't know how to get bool values on your system])
])

dnl For setlocale
AC_CHECK_HEADERS(locale.h)
dnl For nl_langinfo
AC_CHECK_HEADERS(langinfo.h)

dnl Checks for typedefs, structures, and compiler characteristics.
saved_CFLAGS="$CFLAGS"
CFLAGS=""
AC_C_CONST
CFLAGS="$saved_CFLAGS"

dnl Checks for library functions.
AC_CHECK_FUNCS(nl_langinfo)
AC_CHECK_FUNCS(setlocale)
AC_CHECK_FUNCS(poll)
AC_REPLACE_FUNCS(inet_pton)

# http://www.gnu.org/manual/autoconf-2.53/html_node/AC_LIBOBJ-vs.-LIBOBJS.html
# This is necessary so that .o files in LIBOBJS are also built via
# the ANSI2KNR-filtering rules.
LIB@&t@OBJS=`echo "$LIB@&t@OBJS" | sed 's,\.[[^.]]* ,$U&,g;s,\.[[^.]]*$,$U&,'`
LTLIBOBJS=`echo "$LIB@&t@OBJS" | sed 's,\.[[^.]]* ,.lo ,g;s,\.[[^.]]*$,.lo,'`
AC_SUBST(LTLIBOBJS)


AM_ICONV()
if test "$am_cv_func_iconv" '!=' yes; then
	AC_MSG_ERROR([iconv not found, try using --with-libiconv-prefix])
fi
AC_CHECK_HEADERS(iconv.h,,[
		AC_MSG_WARN([iconv.h not found (INCICONV=$INCICONV)])
		])

saved_CFLAGS="$CFLAGS"
CFLAGS=`echo $CFLAGS | sed 's/-Werror//'`
AC_CHECK_LIB(socket, socket, AC_CHECK_LIB(nsl, getpwnam), [])
AC_CHECK_LIB(m, floor)
CFLAGS="$saved_CFLAGS"

AS_AC_EXPAND(LIBEXECDIR, "$libexecdir")
AS_AC_EXPAND(SYSCONFDIR, "$sysconfdir")
AS_AC_EXPAND(LOCALSTATEDIR, "$localstatedir")
AS_AC_EXPAND(SYNCEDATADIR, "$datadir/${PACKAGE}")
AS_AC_EXPAND(UDEVDIR, "$udev_dir")

AC_CONFIG_FILES( [lib/utils/synce.h] )

AC_CONFIG_FILES(
	[tools/synce-install-cab],
	[chmod +x tools/synce-install-cab]
)

AC_CONFIG_FILES(
	[tools/synce-remove-program],
	[chmod +x tools/synce-remove-program]
)

AC_CONFIG_FILES([
        scripts/synceconnector.py \
        scripts/udev-synce-rndis \
        scripts/udev-synce-serial \
        scripts/synce-udev-rndis \
        scripts/synce-udev-serial \
        scripts/synce-serial \
        scripts/85-synce.rules \
        bluetooth/synce-udev-bt-ipup \
        etc/org.synce.dccm.service \
        etc/synce-connector.conf \
        docs/Doxyfile] )

AC_OUTPUT([libsynce.pc
           Makefile
           lib/Makefile
           lib/utils/Makefile
           lib/utils/config/Makefile
           lib/rapi/Makefile
           lib/rapi/support/Makefile
           lib/rapi/backend_ops_1/Makefile
           lib/rapi/backend_ops_2/Makefile
           tests/Makefile
           tests/CeRapiInvoke/Makefile
           tests/CeRapiInvoke/dll/Makefile
           tests/rapi/Makefile
           tools/Makefile
           python/Makefile
           python/tests/Makefile
           man/Makefile
           man/man1/Makefile
           man/man3/Makefile
           man/man7/Makefile
           scripts/Makefile
           dccm/Makefile
           etc/Makefile
           bluetooth/Makefile
           docs/Makefile
])

