##
## GNET-2.0 cmake find module.
##

SET(GNET2_NAME "gnet-2.0")


FIND_PATH(GNET2_INCLUDE_DIR NAMES gnet.h PATHS /usr/include/${GNET2_NAME} /usr/include /usr/include/gnet /usr/local/include /usr/local/include/gnet /usr/local/include/${GNET2_NAME})

FIND_PATH(GNET2_CONFIG_DIR NAMES gnetconfig.h PATHS /usr/include/${GNET2_NAME} /usr/include /usr/include/gnet /usr/local/include /usr/local/include/gnet /usr/local/include/${GNET2_NAME} /usr/lib/${GNET2_NAME}/include)

FIND_LIBRARY(GNET2_LIBRARY ${GNET2_NAME} PATHS /usr/lib /usr/local/lib)

FIND_PATH(GNET2_CONFIG_INCLUDE gnetconfig.h PATHS /usr/lib/${GNET2_NAME} /usr/local/lib/${GNET2_NAME} /usr/lib/${GNET2_NAME}/include /usr/local/lib/${GNET2_NAME}/include)

IF (GNET2_INCLUDE_DIR AND GNET2_LIBRARY AND GNET2_CONFIG_INCLUDE)
	SET(GNET2_FOUND TRUE)
ENDIF (GNET2_INCLUDE_DIR AND GNET2_LIBRARY AND GNET2_CONFIG_INCLUDE)

IF (NOT GNET2_FOUND)
	IF (GNET2_FIND_REQUIRED)
		MESSAGE (FATAL_ERROR "${GNET2_NAME} development package is mandatory ! Please install from your distro media or download from ftp://ftp.gnu.org.")
	ENDIF (GNET2_FIND_REQUIRED)
ENDIF (NOT GNET2_FOUND)
