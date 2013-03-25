#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([Project Isagoge], [1.0], [jrbruce86@gmail.com])
AM_INIT_AUTOMAKE

# Make files and scripts
AC_CONFIG_FILES([Makefile
                src/Makefile
                src/Tesseract/Makefile
                src/Data_Generation_Tools/select_random/Makefile 
                src/Data_Generation_Tools/select_training/Makefile
                src/Data_Generation_Tools/Makefile],
                [src/Data_Generation_Tools/removealpha],
                [chmod +x src/Data_Generation_Tools/removealpha],
                [src/Data_Generation_Tools/rename_pngs],
                [chmod +x src/Data_Generation_Tools/rename_pngs],
                [src/Data_Generation_Tools/convert_pdfpages],
                [chmod +x src/Data_Generation_Tools/convert_pdfpages],
                [chmod +x sr/Data_Generation_Tools/Equation_Labeler/qbuild])

AC_CONFIG_SRCDIR([src/EquationDetectorSVM.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
have_lept=no
if test "$LIBLEPT_HEADERSDIR" = "" ; then
  LIBLEPT_HEADERSDIR="/usr/local/include /usr/include"
fi
for incd in $LIBLEPT_HEADERSDIR
do
  for lept in . leptonica liblept
  do
    if test -r "$incd/$lept/allheaders.h" ; then
      CPPFLAGS="$CPPFLAGS -I$incd/$lept"
      have_lept=yes
    fi
  done
done
if test "$have_lept" = yes ; then
  AC_MSG_RESULT(yes)
  AC_CHECK_LIB(lept,pixCreate,[], AC_MSG_ERROR([leptonica library missing]))
else
  AC_MSG_ERROR([leptonica not found])
fi

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h] [allheaders.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([floor])

# Check location of leptonica/liblept headers.
AC_MSG_CHECKING(for leptonica)
AC_ARG_VAR(LIBLEPT_HEADERSDIR,[Leptonica headers directory])

AC_CONFIG_SUBDIRS([src/Tesseract])
AC_OUTPUT
