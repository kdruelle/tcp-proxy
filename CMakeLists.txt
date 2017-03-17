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

CMAKE_MINIMUM_REQUIRED (VERSION 2.6.3)
if("${CMAKE_VERSION}" VERSION_GREATER 3.0.0)
cmake_policy(SET CMP0037 OLD)
endif()
set(CMAKE_LEGACY_CYGWIN_WIN32 0)


################################################################################
# Project Properties Section
# You must edit this section to match your needs
################################################################################


SET (PROJECT_TYPE EXECUTABLE)                                       # Project type (EXECUTABLE, LIBRARY)

PROJECT (tcp-proxy)                                                 # Name of the executable or library object
SET (APPLICATION_NAME "TCP Proxy")                                  # Application pretty name
SET (APPLICATION_CODENAME "tcp-proxy")                              # Application intern codename for log porpose for exemple
SET (APPLICATION_SHORTNAME "tcp-proxy")                           # Application intern short name for log porpose for exemple
SET (APPLICATION_COPYRIGHT_YEARS "2015")
SET (APPLICATION_VENDOR_NAME "Kevin Druelle <kevin@druelle.info>")


LIST (APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}")
LIST (APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/tools")
SET (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/tools/cmake")


################################################################################
# Required CMake Scripts
################################################################################
INCLUDE (GitVersion)
INCLUDE (DocumentationTargets)
INCLUDE (DistcleanTarget)
INCLUDE (package)
INCLUDE (ExternalProject)

################################################################################
# Debugging Options
################################################################################

if(NOT CMAKE_BUILD_TYPE)
  SET (CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)


################################################################################
# Project Output Paths
################################################################################
SET (EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
SET (LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/lib")


INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")

################################################################################
# Compiler Option
################################################################################
if(DEVELOPPER_MODE)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEVELOPPER_MODE")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEVELOPPER_MODE")
endif(DEVELOPPER_MODE)

IF (${PROJECT_TYPE} STREQUAL "EXECUTABLE")
  set(PROJECT_TYPE_EXECUTABLE 1)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DPROJECT_TYPE_EXECUTABLE")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DPROJECT_TYPE_EXECUTABLE")
ENDIF (${PROJECT_TYPE} STREQUAL "EXECUTABLE")

IF (${PROJECT_TYPE} STREQUAL "LIBRARY")
  set(PROJECT_TYPE_LIBRARY 1)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DPROJECT_TYPE_LIBRARY")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DPROJECT_TYPE_LIBRARY")
ENDIF (${PROJECT_TYPE} STREQUAL "LIBRARY")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")


################################################################################
# INSTALLATION TARGETS
################################################################################
IF (EXISTS "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}" AND IS_DIRECTORY "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}")
    INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}" DESTINATION "include")
ENDIF (EXISTS "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}" AND IS_DIRECTORY "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}")

IF (EXISTS "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.conf")
    INSTALL(FILES "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.conf" DESTINATION conf)
ENDIF (EXISTS "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.conf")

################################################################################
# Configure Files
################################################################################
FILE (GLOB_RECURSE CONFIGINPUTS1 include/*.h.in)
FILE (GLOB_RECURSE CONFIGINPUTS2 src/*.h.in)
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS1}")
LIST (APPEND CONFIGINPUTS "${CONFIGINPUTS2}")
FOREACH (CONFIGINPUT ${CONFIGINPUTS})
  STRING (REPLACE ".in.h.cmake" ".h" CONFIGOUTPUT1 "${CONFIGINPUT}")
  STRING (REPLACE ".h.in" ".h" CONFIGOUTPUT2 "${CONFIGOUTPUT1}")
  CONFIGURE_FILE ("${CONFIGINPUT}" "${CONFIGOUTPUT2}")
ENDFOREACH (CONFIGINPUT ${CONFIGINPUTS})


################################################################################
# Dependencies
################################################################################

SET (Boost_ADDITIONAL_VERSIONS "1.50" "1.50.0")
FIND_PACKAGE (Boost 1.50 COMPONENTS "program_options" "date_time" "system" "filesystem" "thread" REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
IF(Boost_SYSTEM_FOUND)
  SET (HAVE_BOOST_SYSTEM 1)
  SET (project_LIBS ${project_LIBS} ${Boost_LIBRARIES})
ENDIF(Boost_SYSTEM_FOUND)


# Static OpenSSL
ExternalProject_Add(
    openssl
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/openssl
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/external/openssl/config no-shared no-psk no-srp no-weak-ssl-ciphers
    BINARY_DIR ${CMAKE_SOURCE_DIR}/external/openssl
    BUILD_COMMAND make depend COMMAND make VERBOSE=1
    INSTALL_COMMAND ""
)


ADD_SUBDIRECTORY(src)

