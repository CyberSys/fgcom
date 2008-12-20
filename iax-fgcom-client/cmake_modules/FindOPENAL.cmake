##
## OPENAL cmake find module.
##
SET(OPENAL_NAME "openal")

FIND_PATH(OPENAL_INCLUDE_DIR NAMES alc.h alext.h al.h alut.h PATHS /usr/include /usr/include/AL /usr/local/include /usr/local/include/AL)
FIND_LIBRARY(OPENAL_LIBRARY ${OPENAL_NAME} PATHS /usr/lib /usr/local/lib)
INCLUDE_DIRECTORIES(${OPENAL_INCLUDE_DIR})


IF (OPENAL_INCLUDE_DIR AND OPENAL_LIBRARY)
	SET(OPENAL_FOUND TRUE)
ENDIF (OPENAL_INCLUDE_DIR AND OPENAL_LIBRARY)

IF (NOT OPENAL_FOUND)
	IF (OPENAL_FIND_REQUIRED)
		MESSAGE (FATAL_ERROR "${OPENAL_NAME} development package is mandatory ! Please install from your distro media or download from ftp://ftp.gnu.org.")
	ENDIF (OPENAL_FIND_REQUIRED)
ENDIF (NOT OPENAL_FOUND)

