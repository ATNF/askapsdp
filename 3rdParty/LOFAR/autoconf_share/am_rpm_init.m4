dnl
dnl Available from the SourceForge Autoconf Macro Archive at:
dnl http://ac-archive.sourceforge.net/Installed_Packages/am_rpm_init.html
dnl
dnl AM_RPM_INIT
dnl
dnl Figure out how to create rpms for this system and setup for an
dnl automake target
dnl

AC_DEFUN([AM_RPM_INIT],
[dnl
AC_REQUIRE([AC_CANONICAL_HOST])
dnl Find the RPM program
AC_ARG_ENABLE(rpm-rules,
	      [  --disable-rpm-rules       Do not add rpm make rules],
              disable_rpm_rules=yes, disable_rpm_rules=no)
AC_ARG_WITH(rpm-prog,
            [  --with-rpm-prog=PROG       Which rpm program to use (optional)],
            rpm_prog="$withval", rpm_prog="")

AC_ARG_WITH(rpm-extra-args,
	    [  --with-rpm-extra-args=ARGS Run rpm with extra arguments (defaults to none)],
            rpm_extra_args="$withval", rpm_extra_args="")

dnl echo disable_rpm_rules is $disable_rpm_rules
dnl echo rpm_prog is $rpm_prog

  RPM_TARGET=""

  if test x$disable_rpm_rules = xyes ; then
     AC_MSG_CHECKING([whether rpm rules should be enabled])
     AC_MSG_RESULT([no])
     no_rpm=yes
  else
    if test x$rpm_prog != x ; then
       if test x${RPM_PROG+set} != xset ; then
          RPM_PROG=$rpm_prog
       fi
    fi

    AC_PATH_PROG(RPMBUILD_PROG, rpmbuild, no)
    if test "$RPMBUILD_PROG" = "no"; then
	AC_PATH_PROG(RPM_PROG, rpm, no)
    else
        RPM_PROG=$RPMBUILD_PROG;
    fi
    no_rpm=no
    if test "$RPM_PROG" = "no" ; then
echo *** RPM Configuration Failed
echo *** Failed to find the rpm program.  If you want to build rpm packages
echo *** indicate the path to the rpm program using  --with-rpm-prog=PROG
      no_rpm=yes
      RPM_MAKE_RULES=""
    else
      AC_MSG_CHECKING(how rpm sets %{_rpmdir})
      rpmdir=`rpm --eval %{_rpmdir}`
      if test x$rpmdir = x"%{_rpmdir}" ; then
        AC_MSG_RESULT([not set (cannot build rpms?)])
        echo *** Could not determine the value of %{_rpmdir}
        echo *** This could be because it is not set, or your version of rpm does not set it
        echo *** It must be set in order to generate the correct rpm generation commands
        echo ***
        echo *** You might still be able to create rpms, but I could not automate it for you
        echo *** BTW, if you know this is wrong, please help to improve the rpm.m4 module
        echo *** Send corrections, updates and fixes to dhawkins@cdrgts.com.  Thanks.
      else
        AC_MSG_RESULT([$rpmdir])
      fi
      AC_MSG_CHECKING(how rpm sets %{_rpmfilename})
      rpmfilename=$rpmdir/`rpm --eval %{_rpmfilename} | sed "s/%{ARCH}/${host_cpu}/g" | sed "s/%{NAME}/$PACKAGE/g" | sed "s/%{VERSION}/${VERSION}/g" | sed "s/%{RELEASE}/${RPM_RELEASE}/g"`
      AC_MSG_RESULT([$rpmfilename])

      RPM_DIR=${rpmdir}
      RPM_TARGET=$rpmfilename
      RPM_ARGS="-ts $rpm_extra_args"
      RPM_TARBALL=${PACKAGE}-${VERSION}.tar.gz
    fi
  fi

  case "${no_rpm}" in
    yes) make_rpms=false;;
    no) make_rpms=true;;
    *) AC_MSG_WARN([bad value ${no_rpm} for no_rpm (not making rpms)])
       make_rpms=false;;
  esac
  AC_SUBST(RPM_DIR)
  AC_SUBST(RPM_TARGET)
  AC_SUBST(RPM_ARGS)
  AC_SUBST(RPM_TARBALL)

  RPM_CONFIGURE_ARGS=${ac_configure_args}
  AC_SUBST(RPM_CONFIGURE_ARGS)
])

