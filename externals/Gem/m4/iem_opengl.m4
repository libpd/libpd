# SYNOPSIS
#
#   IEM_OPENGL
#
# DESCRIPTION
#
#  some checks for OpenGL-related stuff
#  basically just a wrapper around Braden McDaniel's ax_check_gl* family (as 
#  found in Gnu's autoconf-archive) with additionally setting am-conditionals HAVE_GL* 
#
# LICENSE
#
#   Copyright (c) 2011 IOhannes m zmölnig <zmoelnig@iem.at>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.


AC_DEFUN([IEM_CHECK_GL],
[AC_REQUIRE([AX_CHECK_GL])
AM_CONDITIONAL(HAVE_GL, [test "x$no_gl" != "xyes"])
])

AC_DEFUN([IEM_CHECK_GLU],
[AC_REQUIRE([AX_CHECK_GLU])
AM_CONDITIONAL(HAVE_GLU, [test "x$no_glu" != "xyes"])
])


AC_DEFUN([IEM_CHECK_GLUT],
[AC_REQUIRE([AX_CHECK_GLUT])
AM_CONDITIONAL(HAVE_GLUT, [test "x$no_glut" != "xyes"])
])


AC_DEFUN([IEM_CHECK_GLX],
[
AC_CHECK_HEADERS([GL/glx.h],,no_glx=yes)
AM_CONDITIONAL(HAVE_GLX, [test "x$no_glx" != "xyes"])
AC_SUBST([GLX_CFLAGS])
AC_SUBST([GLX_LIBS])
])



AC_DEFUN([IEM_CHECK_AGL],
[
GEM_CHECK_FRAMEWORK(AGL)
if test "x$gem_check_ldflags_success" != "xyes"; then
   no_agl="yes"
fi
AGL_CFLAGS=""
AGL_LIBS="${GEM_FRAMEWORK_AGL}"
AC_SUBST([AGL_CFLAGS])
AC_SUBST([AGL_LIBS])
AM_CONDITIONAL(HAVE_AGL, [test "x$no_agl" != "xyes"])
])



AC_DEFUN([IEM_CHECK_GLEW],
[
GEM_CHECK_LIB(glew, GLEW, glewInit,,no_glew=yes,,[OpenGL Extension Wrangler library], [no])
AM_CONDITIONAL(HAVE_GLEW, [test "x$no_glew" != "xyes"])
])



dnl # original code in Gem/configure.ac
dnl have_gl="no"
dnl have_glu="no"
dnl 
dnl GEM_CHECK_FRAMEWORK(OpenGL,
dnl         [have_opengl_framework="yes" have_gl="yes"],
dnl         [have_opengl_framework="no" have_gl="no"])
dnl 
dnl # don't need libGL if we have the OpenGL framework
dnl if test "x$have_opengl_framework" != "xyes"; then
dnl  AC_CHECK_LIB(GL,glInitNames)
dnl  if test "x$ac_cv_lib_GL_glInitNames" = "xyes"; then
dnl    have_gl="yes"
dnl  fi
dnl 
dnl  AC_CHECK_LIB(GLU,gluLookAt)
dnl  if test "x$ac_cv_lib_GLU_gluLookAt" = "xyes"; then
dnl    have_glu="yes"
dnl  fi
dnl fi
dnl GEM_CHECK_FRAMEWORK(AGL)
dnl if test "x$gem_check_ldflags_success" = "xyes"; then
dnl   have_glu="yes"
dnl fi


dnl have_gl_headers="no"
dnl AC_CHECK_HEADER(GL/gl.h,have_gl_headers="yes")
dnl AC_CHECK_HEADER(OpenGL/gl.h,have_gl_headers="yes")
dnl if test "x$have_gl_headers" != "xyes"; then
dnl   have_gl="no"
dnl fi
dnl 
dnl have_glu_headers="no"
dnl AC_CHECK_HEADER(GL/glu.h,have_glu_headers="yes")
dnl AC_CHECK_HEADER(OpenGL/glu.h,have_glu_headers="yes")
dnl if test "x$have_glu_headers" != "xyes"; then
dnl   have_glu="no"
dnl fi
dnl 
dnl AC_CHECK_HEADERS(GL/glx.h)
