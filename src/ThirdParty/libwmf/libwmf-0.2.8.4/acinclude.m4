dnl AC_CHECK_VERSION(version_number_of_package,reference_minimum_version_number)
dnl sets ac_check_version_okay=yes if successful
dnl 
AC_DEFUN([AC_CHECK_VERSION],[
	current_version=$1

	current_major_version=`echo $current_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	current_minor_version=`echo $current_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	current_micro_version=`echo $current_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

	minimum_version=$2

	minimum_major_version=`echo $minimum_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	minimum_minor_version=`echo $minimum_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	minimum_micro_version=`echo $minimum_version | sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

	ac_check_version_okay=no

	if [ test $current_major_version -gt $minimum_major_version ]; then
		ac_check_version_okay=yes
	elif [ test $current_major_version -eq $minimum_major_version ]; then
		if [ test $current_minor_version -gt $minimum_minor_version ]; then
			ac_check_version_okay=yes
		elif [ test $current_minor_version -eq $minimum_minor_version ]; then
			if [ test $current_micro_version -ge $minimum_micro_version ]; then
				ac_check_version_okay=yes
			fi
		fi
	fi
])
AC_DEFUN([AC_LIBPLOT_LIBS],[
	ac_can_link_libplot=no

	LIBPLOT_LIBS="-lplot"

	ac_libplot_ldflags=$LDFLAGS

	LDFLAGS="$LDFLAGS $LIBPLOT_LIBS"

	AC_MSG_CHECKING(whether libplot requires X)
	AC_TRY_LINK([
#include <stdio.h>
#include <plot.h>
	],[
	plPlotter* plotter;
	plPlotterParams* params;

	params = pl_newplparams ();
	plotter = pl_newpl_r ("X",stdin,stdout,stderr,params);

	pl_deletepl_r (plotter);
	pl_deleteplparams (params);
	],[	ac_can_link_libplot=yes
		AC_MSG_RESULT(no)
	],[	AC_MSG_RESULT(yes)
	])

	LDFLAGS=$ac_libplot_ldflags

	if [ test "x$no_x" != "xyes" ]; then

		dnl Athena:

		if [ test $ac_can_link_libplot != yes ]; then
			LIBPLOT_LIBS="-lplot $X_LIBS -lXaw -lXmu -lXt $X_PRE_LIBS -lXext -lX11 $X_EXTRA_LIBS -lm"

			ac_libplot_ldflags=$LDFLAGS

			LDFLAGS="$LDFLAGS $LIBPLOT_LIBS"

			AC_MSG_CHECKING(whether libplot links against Athena)
			AC_TRY_LINK([
#include <stdio.h>
#include <plot.h>
			],[
	plPlotter* plotter;
	plPlotterParams* params;

	params = pl_newplparams ();
	plotter = pl_newpl_r ("X",stdin,stdout,stderr,params);

	pl_deletepl_r (plotter);
	pl_deleteplparams (params);
			],[	ac_can_link_libplot=yes
				AC_MSG_RESULT(yes)
			],[	AC_MSG_RESULT(no)
			])

			LDFLAGS=$ac_libplot_ldflags
		fi

		dnl Motif:

		if [ test $ac_can_link_libplot != yes ]; then
			ac_libplot_ldflags=$LDFLAGS

			LDFLAGS="$LDFLAGS $X_LIBS"

			AC_CHECK_LIB(Xp,main,[libXp="-lXp"],[libXp=""],-lXext -lX11)
		
			LIBPLOT_LIBS="-lplot $X_LIBS -lXm $libXp -lXt $X_PRE_LIBS -lXext -lX11 $X_EXTRA_LIBS -lm"

			AC_CHECK_LIB(gen,main,[LIBPLOT_LIBS="$LIBPLOT_LIBS -lgen"])
			AC_CHECK_LIB(PW,main,[LIBPLOT_LIBS="$LIBPLOT_LIBS -lPW"])

			LDFLAGS=$ac_libplot_ldflags

			LDFLAGS="$LDFLAGS $LIBPLOT_LIBS"

			AC_MSG_CHECKING(whether libplot links against Motif)
			AC_TRY_LINK([
#include <stdio.h>
#include <plot.h>
			],[
	plPlotter* plotter;
	plPlotterParams* params;

	params = pl_newplparams ();
	plotter = pl_newpl_r ("X",stdin,stdout,stderr,params);

	pl_deletepl_r (plotter);
	pl_deleteplparams (params);
			],[	ac_can_link_libplot=yes
				AC_MSG_RESULT(yes)
			],[	AC_MSG_RESULT(no)
			])

			LDFLAGS=$ac_libplot_ldflags
		fi
	fi
])
