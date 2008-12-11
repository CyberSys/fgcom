##
## SPEEX cmake find module.
##
SET(SPEEX_NAME "speex")

FIND_PATH(SPEEX_INCLUDE_DIR NAMES speex.h PATHS /usr/include /usr/local/include /usr/include/speex /usr/local/include/speex)
FIND_LIBRARY(SPEEX_LIBRARY ${SPEEX_NAME} PATHS /usr/lib /usr/local/lib)
INCLUDE_DIRECTORIES(${SPEEX_INCLUDE_DIR} ${SPEEX_INCLUDE_DIR}/glib)


IF (SPEEX_INCLUDE_DIR AND SPEEX_LIBRARY)
	SET(SPEEX_FOUND TRUE)
ENDIF (SPEEX_INCLUDE_DIR AND SPEEX_LIBRARY)

IF (NOT SPEEX_FOUND)
	IF (SPEEX_FIND_REQUIRED)
		MESSAGE (FATAL_ERROR "${SPEEX_NAME} development package is mandatory ! Please install from your distro media or download from ftp://ftp.gnu.org.")
	ENDIF (SPEEX_FIND_REQUIRED)
ENDIF (NOT SPEEX_FOUND)

