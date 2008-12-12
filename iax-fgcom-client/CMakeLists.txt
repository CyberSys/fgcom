#
# IAX FGCOM client - Overall CMakeLists.txt file.
# Charles INGELS <charles@maisonblv.net>
# December 2008
#

#
# Minimum requested version of 'cmake'.
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
#
# We set here the project name.
#
PROJECT (IAX_FGCOM_CLIENT)
#
# We set some informations on the project.
#
SET(${PROJECT_NAME}_AUTHOR "Holger WIRTZ & Charles INGELS")
SET(${PROJECT_NAME}_EMAIL "dcoredump@googlemail.com & charles@maisonblv.net")
SET(${PROJECT_NAME}_TITLE "iax-fgcom-client")
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 0)
SET(${PROJECT_NAME}_PATCH_VERSION 1)
#
# By default, we build the release version.
#
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE release)
ENDIF(NOT CMAKE_BUILD_TYPE)
#
# We define compiler options depending on the build type.
#
IF(CMAKE_BUILD_TYPE STREQUAL "debug")
	ADD_DEFINITIONS("-Wall -ggdb -O0 -D__DEBUG__ -D_GNU_SOURCE -DREENTRANT")
ELSE(CMAKE_BUILD_TYPE STREQUAL "debug")
	ADD_DEFINITIONS("-O2 -D_GNU_SOURCE -DREENTRANT")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "debug")
#
# We display some useful informations on the project.
#
MESSAGE(STATUS)
MESSAGE(STATUS "------------------------------------------------------------")
MESSAGE(STATUS "Project name			: ${PROJECT_NAME}")
MESSAGE(STATUS "Project version		: ${IAX_FGCOM_CLIENT_MAJOR_VERSION}.${IAX_FGCOM_CLIENT_MINOR_VERSION}.${IAX_FGCOM_CLIENT_PATCH_VERSION}")
MESSAGE(STATUS "Project authors		: ${IAX_FGCOM_CLIENT_AUTHOR}")
MESSAGE(STATUS "Project source dir		: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Operating system 		: ${CMAKE_SYSTEM_NAME}")
MESSAGE(STATUS "Target system		: ${CMAKE_SYSTEM_PROCESSOR}")
MESSAGE(STATUS "Build type			: ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS)
MESSAGE(STATUS "To change build, type 'cmake -DCMAKE_BUILD_TYPE=[debug|release] .'")
#
# Here, we set other sub-directories to compile.
#
ADD_SUBDIRECTORY(libiax2)
ADD_SUBDIRECTORY(gsm/src)
ADD_SUBDIRECTORY(libspeex)
ADD_SUBDIRECTORY(spandsp)
ADD_SUBDIRECTORY(portmixer)
ADD_SUBDIRECTORY(lib)
MESSAGE(STATUS "------------------------------------------------------------")
#
# End of file.
#
