##
## GLIB-2.0 cmake find module.
##

SET(GLIB2_NAME "glib-2.0")


FIND_PATH(GLIB2_INCLUDE_DIR NAMES glib.h PATHS /usr/include/${GLIB2_NAME} /usr/include /usr/include/glib /usr/local/include /usr/local/include/glib /usr/local/include/${GLIB2_NAME})

FIND_LIBRARY(GLIB2_LIBRARY ${GLIB2_NAME} PATHS /usr/lib /usr/local/lib)

FIND_PATH(GLIB2_CONFIG_INCLUDE glibconfig.h PATHS /usr/lib/${GLIB2_NAME} /usr/local/lib/${GLIB2_NAME} /usr/lib/${GLIB2_NAME}/include /usr/local/lib/${GLIB2_NAME}/include)

IF (GLIB2_INCLUDE_DIR AND GLIB2_LIBRARY AND GLIB2_CONFIG_INCLUDE)
	SET(GLIB2_FOUND TRUE)
ENDIF (GLIB2_INCLUDE_DIR AND GLIB2_LIBRARY AND GLIB2_CONFIG_INCLUDE)

IF (NOT GLIB2_FOUND)
	IF (GLIB2_FIND_REQUIRED)
		MESSAGE (FATAL_ERROR "${GLIB2_NAME} development package is mandatory ! Please install from your distro media or download from ftp://ftp.gnu.org.")
	ENDIF (GLIB2_FIND_REQUIRED)
ENDIF (NOT GLIB2_FOUND)
