##
## OGGZ1 cmake find module.
##

SET(OGGZ_NAME "oggz")

FIND_PATH(OGGZ_INCLUDE_DIR NAMES oggz.h PATHS /usr/include/${OGGZ_NAME} /usr/include /usr/include/glib /usr/local/include /usr/local/include/glib /usr/local/include/${OGGZ_NAME})

FIND_LIBRARY(OGGZ_LIBRARY ${OGGZ_NAME} PATHS /usr/lib /usr/local/lib)

IF (OGGZ_INCLUDE_DIR AND OGGZ_LIBRARY)
	SET(OGGZ_FOUND TRUE)
ENDIF (OGGZ_INCLUDE_DIR AND OGGZ_LIBRARY)

IF (NOT OGGZ_FOUND)
	IF (OGGZ_FIND_REQUIRED)
		MESSAGE (FATAL_ERROR "${OGGZ_NAME} development package is mandatory ! Please install from your distro media or download from ftp://ftp.gnu.org.")
	ENDIF (OGGZ_FIND_REQUIRED)
ENDIF (NOT OGGZ_FOUND)
