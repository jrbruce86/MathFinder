#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([MathFinder], [1.0], [jrbruce86@gmail.com])
AM_INIT_AUTOMAKE([tar-ustar subdir-objects])
#AM_INIT_AUTOMAKE([1.9 tar-ustar])
LT_INIT #libtool

AC_CONFIG_SUBDIRS([src/THIRDPARTY/Tesseract src/THIRDPARTY/dlib-18.4 src/FINDER/COMMON])

AX_CXX_COMPILE_STDCXX_11

# Make files and scripts
AC_CONFIG_FILES([Makefile
                src/FINDER/APP/Makefile
                src/FINDER/COMMON/Makefile
                src/THIRDPARTY/dlib-18.4/Makefile
                src/THIRDPARTY/Tesseract/Makefile
                src/TOOLS/GROUNDTRUTH_GENERATION/select_random/Makefile 
                src/TOOLS/GROUNDTRUTH_GENERATION/select_training/Makefile
                src/TOOLS/GROUNDTRUTH_GENERATION/Makefile])

AC_CONFIG_FILES([src/TOOLS/GROUNDTRUTH_GENERATION/scripts/removealpha:src/TOOLS/GROUNDTRUTH_GENERATION/scripts/removealpha], \
[chmod +x src/TOOLS/GROUNDTRUTH_GENERATION/scripts/removealpha])

AC_CONFIG_FILES([src/TOOLS/GROUNDTRUTH_GENERATION/scripts/convert_pdfpages:src/TOOLS/GROUNDTRUTH_GENERATION/scripts/convert_pdfpages], \
[chmod +x src/TOOLS/GROUNDTRUTH_GENERATION/scripts/convert_pdfpages])

AC_CONFIG_FILES([src/TOOLS/GROUNDTRUTH_GENERATION/scripts/writefilenames:src/TOOLS/GROUNDTRUTH_GENERATION/scripts/writefilenames], \
[chmod +x src/TOOLS/GROUNDTRUTH_GENERATION/scripts/writefilenames])

AC_CONFIG_FILES([src/TOOLS/GROUNDTRUTH_GENERATION/scripts/printPngInfo:src/TOOLS/GROUNDTRUTH_GENERATION/scripts/printPngInfo], \
[chmod +x src/TOOLS/GROUNDTRUTH_GENERATION/scripts/printPngInfo])

AC_CONFIG_FILES([src/TOOLS/GROUNDTRUTH_GENERATION/scripts/convertPngsToBinary:src/TOOLS/GROUNDTRUTH_GENERATION/scripts/convertPngsToBinary], \
[chmod +x src/TOOLS/GROUNDTRUTH_GENERATION/scripts/convertPngsToBinary])

# Go ahead and attempt to build the qt dependency
AC_CONFIG_FILES([src/TOOLS/GROUNDTRUTH_GENERATION/MathFinderGtGen/qbuild:src/TOOLS/GROUNDTRUTH_GENERATION/MathFinderGtGen/qbuild], \
[chmod +x src/TOOLS/GROUNDTRUTH_GENERATION/MathFinderGtGen/qbuild; cd src/TOOLS/GROUNDTRUTH_GENERATION/MathFinderGtGen; ./qbuild; cd -;])

AC_CONFIG_SRCDIR([src/FINDER/APP/FIND/MathExpressionFinderMain.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Check for libpng
AC_MSG_CHECKING([for libpng-dev])
if test -d /usr/include/libpng; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_ERROR([libpng-dev not found])
fi

#Check for qt4
AC_MSG_CHECKING([for Qt])
if test -d /usr/include/qt*; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_ERROR([Qt not found])
fi

# Check for libjpeg
AC_MSG_CHECKING([for libjpeg-dev])
if test -f /usr/include/jpeglib.h; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_ERROR([libjpeg-dev not found])
fi

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

#AC_CONFIG_COMMANDS_PRE([./src/TOOLS/GROUNDTRUTH_GENERATION/MathFinderGtGen/qbuild])

# Create the user's local directories
mkdir -p ~/.mathfinder/training
cp training/* ~/.mathfinder/training -R
mkdir -p ~/.mathfinder/groundtruth/advcalc1
cp data/Groundtruth/WithoutLabels/advcalc1/* ~/.mathfinder/groundtruth/advcalc1 -R

AC_OUTPUT
