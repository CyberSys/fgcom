#
# libiax2/CMakeLists.txt, Charles INGELS (zedraken@maisonblv.net), December 2008
# 

# 
# We fill a variable with all header files needed.
#
SET(IAX2_HEADER_FILES answer.h dialtone.h iax2.h iax-client.h jitterbuf.h miniphone.h ring10.h winpoop.h busy.h frame.h iax2-parser.h iax.h md5.h options.h ringtone.h)
#
# We fill a variable with all source files requested to build 'libiax2.a'.
#
SET(IAX2_SRC_FILES ${IAX2_HEADER_FILES} iax2-parser.c iax.c jitterbuf.c md5.c options.c)
#
# We indicate the target which is the library 'libiax2.a'.
#
ADD_LIBRARY(iax2 STATIC ${IAX2_SRC_FILES})
ADD_LIBRARY(iax2-dynamic SHARED ${IAX2_SRC_FILES})
SET_TARGET_PROPERTIES(iax2 PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(iax2-dynamic PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(iax2-dynamic PROPERTIES OUTPUT_NAME "iax2")
#
# We indicate requested include directories.
#
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/include ${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/libiax2 ${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/gsm/inc)
#
# To build the 'miniphone' application, we need to indicate the directory
# where we can find the 'libiax2.a' library.
#
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/libiax2)
#
# We add a new target to build the miniphone application.
#
# ADD_EXECUTABLE(miniphone miniphone.c)
#
# We can display some informational messages.
#
MESSAGE(STATUS "------------------------------------------------------------")
MESSAGE(STATUS "IAX2 library source dir	: ${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/libiax2")


