##
## LIB3DS cmake find module.
##

FIND_PATH(LIB3DS_INCLUDE_DIR NAMES NAMES atmosphere.h camera.h ease.h float.h light.h matrix.h node.h shadow.h tracks.h vector.h background.h chunk.h file.h io.h material.h mesh.h quat.h tcb.h types.h viewport.h PATHS /usr/include/lib3ds /usr/local/include/lib3ds)

FIND_LIBRARY(LIB3DS_LIBRARY lib3ds.a PATHS /usr/lib /usr/local/lib)

SET(LIB3DS_FOUND FALSE)
IF(LIB3DS_INCLUDE_DIR AND LIB3DS_LIBRARY)
	SET(LIB3DS_FOUND TRUE)
ENDIF(LIB3DS_INCLUDE_DIR AND LIB3DS_LIBRARY)
