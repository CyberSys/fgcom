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
SET(LIBVER "\"${IAX_FGCOM_CLIENT_MAJOR_VERSION}.${IAX_FGCOM_CLIENT_MINOR_VERSION}.${IAX_FGCOM_CLIENT_PATCH_VERSION}\"")
#
# Set IAX Fgcom subdirectory variable which contains all used libraries.
#
SET(IAX_CLIENT_DIR "iax-fgcom-client")
#
# Set path to some helper.
#
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/cmake_modules/" "${PROJECT_SOURCE_DIR}/src/cmake_modules/")
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
# By precedence, the audio library to use is one of the following:
#
# 1 - OpenAL
# 2 - PortAudio
# 3 - Alsa
#
MESSAGE(STATUS)
IF(USE_ALSA)
	MESSAGE(STATUS "Audio library to use		:    Alsa")
ELSEIF(USE_PORTAUDIO)
	MESSAGE(STATUS "Audio library to use		: PortAudio")
ELSE(USE_ALSA)
	MESSAGE(STATUS "Audio library to use		: OpenAL")
	SET(USE_OPENAL 1)
ENDIF(USE_ALSA)

MESSAGE(STATUS "To change audio library, type: ")
MESSAGE(STATUS "'cmake -DUSE_PORTAUDIO=1 .'")
MESSAGE(STATUS "'cmake -DUSE_ALSA=1 .'")
MESSAGE(STATUS "'cmake -DUSE_OPENAL=1 .'")
MESSAGE(STATUS "Default is: OpenAL")
#
# Here, we set other sub-directories to compile.
#
ADD_SUBDIRECTORY(${IAX_CLIENT_DIR}/libiax2)
ADD_SUBDIRECTORY(${IAX_CLIENT_DIR}/gsm/src)
ADD_SUBDIRECTORY(${IAX_CLIENT_DIR}/libspeex)
ADD_SUBDIRECTORY(${IAX_CLIENT_DIR}/spandsp)
ADD_SUBDIRECTORY(${IAX_CLIENT_DIR}/portmixer)
ADD_SUBDIRECTORY(${IAX_CLIENT_DIR}/lib)
ADD_SUBDIRECTORY(src)
MESSAGE(STATUS "------------------------------------------------------------")
#
# End of file.
#
