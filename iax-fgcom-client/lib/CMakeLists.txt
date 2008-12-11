##
## iax-fgcom-client/lib/CMakeLists.txt
## Charles INGELS <zedraken@maisonblv.net>
## December 2008
##
#
# We define a variable containing the name of our library.
# This is just a helper variable which ease library name modification
# in case we want to change the output file name.
#
SET(IAXCLIENT_NAME "iaxfgcom")
#
# Set path to utility modules (used for finding other libraries).
#
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules/")
#
# Check for the presence of PortAudio on that system.
#
FIND_PACKAGE(PORTAUDIO REQUIRED)
#
# We indicate here that we want to use library IAX.
#
SET(LIBIAX 1)
#
# Echo method to use (default is Speex echo).
#
SET(SPEEX_EC 1)
#
# Create the 'iax-fgcom-client/include/config.h' file which contains all
# pre-built definitions.
#
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/include/config.h.cmake ${PROJECT_SOURCE_DIR}/include/config.h)
#
# Defines the 'EXTERNAL_INCLUDES' variable that contains all include pathes.
#
SET(EXTERNAL_INCLUDES ${PROJECT_SOURCE_DIR}/include
${PROJECT_SOURCE_DIR}/spandsp ${PROJECT_SOURCE_DIR}/portmixer/px_common
${PROJECT_SOURCE_DIR}/libiax2 ${PROJECT_SOURCE_DIR}/libspeex ${PROJECT_SOURCE_DIR}/gsm/inc)
#
# Set the include pathes.
#
INCLUDE_DIRECTORIES(${EXTERNAL_INCLUDES})
#
# Defines a variable which contains all header files.
#
SET(IAX_FGCOM_HEADER_FILES audio_alsa.h audio_portaudio.h iaxclient.h ringbuffer.h audio_encode.h codec_alaw.h codec_gsm.h codec_speex.h codec_ulaw.h iaxclient_lib.h)
#
# We tell here the pathes to other libraries we need.
#
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/gsm/src ${PROJECT_SOURCE_DIR}/libiax2 ${PROJECT_SOURCE_DIR}/libspeex ${PROJECT_SOURCE_DIR}/gsm/src ${PROJECT_SOURCE_DIR}/portmixer ${PROJECT_SOURCE_DIR}/spandsp)
# Defines a variable that contains all the source files, including header
# files.
#
## SET(IAX_FGCOM_SRC ${IAX_FGCOM_HEADER_FILES} audio_alsa.c audio_portaudio.c iaxclient_lib.c unixfuncs.c winfuncs.c audio_encode.c codec_alaw.c codec_gsm.c codec_speex.c codec_ulaw.c ringbuffer.c)
SET(IAX_FGCOM_SRC ${IAX_FGCOM_HEADER_FILES} audio_alsa.c audio_portaudio.c iaxclient_lib.c unixfuncs.c audio_encode.c codec_alaw.c codec_gsm.c codec_speex.c codec_ulaw.c ringbuffer.c)
#
# Here, we set the target we want to build, that is we want to build the
# 'libiax-fgcom-client.a' static library and the 'libiax-fgcom-client.so'
# dynamic library.
#
ADD_LIBRARY(${IAXCLIENT_NAME} STATIC ${IAX_FGCOM_SRC})
ADD_LIBRARY(${IAXCLIENT_NAME}-dynamic SHARED ${IAX_FGCOM_SRC})
SET_TARGET_PROPERTIES(${IAXCLIENT_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(${IAXCLIENT_NAME}-dynamic PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(${IAXCLIENT_NAME}-dynamic PROPERTIES OUTPUT_NAME "${IAXCLIENT_NAME}")
#
# We display some useful informations on what we have found on the system.
#
MESSAGE(STATUS)
MESSAGE(STATUS "------------------------------------------------------------")
MESSAGE(STATUS "IAX-FGCOM client source dir	: ${PROJECT_NAME}/lib")
IF(SPAN_EC)
	MESSAGE(STATUS "Span echo			: yes")
	MESSAGE(STATUS "Speex echo			: no")
ELSEIF(SPEEX_EC)
	MESSAGE(STATUS "Span echo			: no")
	MESSAGE(STATUS "Speex echo			: yes")
ENDIF(SPAN_EC)
MESSAGE(STATUS)

IF(PORTAUDIO_FOUND)
	MESSAGE(STATUS "Portaudio include dir	: ${PORTAUDIO_INCLUDE_DIR}")
	MESSAGE(STATUS "Portaudio library    	: ${PORTAUDIO_LIBRARY}")
	MESSAGE(STATUS)
ENDIF(PORTAUDIO_FOUND)

IF(CMAKE_BUILD_TYPE STREQUAL "debug")
MESSAGE(STATUS "External include dir		: ${EXTERNAL_INCLUDES}")
ENDIF(CMAKE_BUILD_TYPE STREQUAL "debug")

MESSAGE(STATUS "To select echo, use 'cmake -DSPAN_EC=1 .' or")
MESSAGE(STATUS "'cmake -DSPEEX_EC=1 .'. Default is to use Speex echo.")






