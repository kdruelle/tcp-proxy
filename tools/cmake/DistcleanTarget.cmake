################################################################################
## 
## Author : Kevin Druelle <kdruelle@opsise.com>  
##  
## 
## (c) Copyright 2014 OpSiSe 
## All Rights Reserved.
##
## This program is an unpublished copyrighted work which is proprietary
## to OpSiSe. This computer program includes Confidential,
## Proprietary Information and is a Trade Secret of OpSiSe.
## Any use, disclosure, modification and/or reproduction is prohibited
## unless authorized in writing by an officer of OpSiSe. All Rights Reserved
##
## 
################################################################################

#
# This CMake package creates a distclean target.
#


function(ADD_PREFIX set_variable prefix)
    foreach(curr ${ARGN})
        set(${set_variable}_TMP ${${set_variable}_TMP} "${prefix}/${curr}")
    endforeach(curr)
    set(${set_variable} ${${set_variable}_TMP} PARENT_SCOPE)
endfunction(ADD_PREFIX)

IF (UNIX)
  ADD_CUSTOM_TARGET (distclean @echo cleaning for source distribution)
  SET(DISTCLEANED
   CMakeTmp CMakeFiles _CPack_Packages
   bin
   lib
   ARGS/
   appinfo.h
   config.h
   cmake.depends
   cmake.check_depends
   CMakeCache.txt
   cmake.check_cache
   *.cmake
   Makefile
   core core.*
   gmon.out
   compile_commands.json
   *.pb.*
   *.deb
   install_manifest.txt
   *~
  )
  
  ADD_PREFIX(SRC "src/" ${DISTCLEANED})
  ADD_PREFIX(TEST "test/" ${DISTCLEANED})
  

  ADD_CUSTOM_COMMAND(
    DEPENDS clean
    COMMENT "distribution clean"
    COMMAND rm
    ARGS    -Rf ${DISTCLEANED} ${TEST} ${SRC} ${SRC_INTERNAL}
    TARGET  distclean
  )
ENDIF(UNIX)