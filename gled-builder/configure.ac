dnl Process this file with autoconf to produce a configure script.

#
# AC_ARG_CONF_VAR(<varname>, <desc>, <default-value>)
#
AC_DEFUN([AC_ARG_CONF_VAR],
[
  AC_ARG_VAR([$1], [$2 [$3]])
  if test "x$$1" = "x"; then
    $1=$3
  fi
])

#
# AC_EXEC_AND_EVAL(<command>, <run-intro>)
#
AC_DEFUN([AC_EXEC_AND_EVAL],
[
  { echo "$as_me:$LINENO: checking $2" >&5
    echo $ECHO_N "$2... $ECHO_C" >&6; }
  ene_result=`$1`
  if test $? != 0; then
    echo "${ECHO_T}error"
    echo "$ene_result"
    exit 1
  else
    eval $ene_result
    { echo "$as_me:$LINENO: result: ok ${ECHO_N}${ene_result}" >&5
      echo "${ECHO_T}ok" >&6; }
  fi
])


AC_INIT(gled-builder, 0)


AC_PROG_INSTALL

AC_PREFIX_DEFAULT("`pwd`/install")

AC_CANONICAL_BUILD


AC_ARG_CONF_VAR([RELEASE],
                [Gled release version],
		["1.3.x-r<svnrev>"])

AC_ARG_CONF_VAR([CACHE_DIR],
                [local source package cache],
		["\${PREFIX}/cache"])

AC_ARG_CONF_VAR([CACHE_TAG],
                [Tag for external software],
		["HEAD"])

AC_ARG_CONF_VAR([CACHE_URL],
                [remote source package cache],
		["rsync://rsync.gled.org/cache/\${CACHE_TAG}"])

AC_ARG_CONF_VAR([SVN_BASE],
                [SVN base URL],
		["https://svn.gled.org/gled"])

AC_ARG_CONF_VAR([SVN_DIR],
                [SVN directory],
		["trunk"])

AC_ARG_CONF_VAR([SVN_REV],
                [SVN revison],
		["HEAD"])

AC_ARG_CONF_VAR([SVN_USER],
                [SVN username],
		["$USER"])

AC_ARG_CONF_VAR([MAKE_J_OPT],
		[parallel build option (for packages that support it)],
		["-j4"])

AC_PROG_CPP
AC_PROG_CC
AC_PROG_CXX
AC_LANG_CPLUSPLUS 


AC_EXEC_AND_EVAL([./compiler-version.pl $CC],
                 [determining compiler version])
AC_SUBST(CC_VERSION)
AC_SUBST(CC_MAJOR)
AC_SUBST(CC_MINOR)


AC_EXEC_AND_EVAL([./distro-details.pl $build_os],
                 [determining OS distribution details])

AC_SUBST(distro_vendor)
AC_SUBST(distro_name)
AC_SUBST(distro_version)


AC_CONFIG_FILES([GledBuildConf.pm MakeBuildConf.inc])
AC_OUTPUT
