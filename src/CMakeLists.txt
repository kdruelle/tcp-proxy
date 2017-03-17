################################################################################
## 
## (C) 2011 Kevin Druelle <kevin@druelle.info>
##
## this software is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
## 
## This software is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this software.  If not, see <http://www.gnu.org/licenses/>.
## 
###############################################################################


FILE (GLOB_RECURSE project_SRCS *.cpp *.cxx *.cc *.C *.c *.h *.hpp)

SET (project_BIN ${PROJECT_NAME})

IF (${PROJECT_TYPE} STREQUAL "EXECUTABLE")
  ADD_EXECUTABLE(${project_BIN} ${project_SRCS})
  SET_TARGET_PROPERTIES (
    ${project_BIN}
    PROPERTIES 
        PREFIX ""
        SUFFIX "-${PROJECT_VERSION_STRING}"
  )
ENDIF (${PROJECT_TYPE} STREQUAL "EXECUTABLE")

IF (${PROJECT_TYPE} STREQUAL "LIBRARY")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -e lib_main_function")
  ADD_LIBRARY(${project_BIN} SHARED ${project_SRCS})
  ADD_LIBRARY(${project_BIN}_s STATIC ${project_SRCS})
  SET_TARGET_PROPERTIES (
    ${project_BIN}
    PROPERTIES
        SOVERSION ${APPLICATION_VERSION_MAJOR}
        VERSION ${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}
  )
ENDIF (${PROJECT_TYPE} STREQUAL "LIBRARY")

INSTALL(
    TARGETS ${project_BIN}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

TARGET_LINK_LIBRARIES(${project_BIN} ${project_LIBS} ${CMAKE_SOURCE_DIR}/external/openssl/libssl.a ${CMAKE_SOURCE_DIR}/external/openssl/libcrypto.a dl)
