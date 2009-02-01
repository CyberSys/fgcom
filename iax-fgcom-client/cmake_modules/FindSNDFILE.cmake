##
## SndFile cmake find module.
##

# FIND_PATH(SNDFILE_INCLUDE_DIR NAMES NAMES sndfile.h PATHS /usr/include /usr/local/include)
FIND_PATH(SNDFILE_INCLUDE_DIR NAMES sndfile.h PATHS /usr/include /usr/local/include)

FIND_LIBRARY(SNDFILE_LIBRARY libsndfile.so PATHS /usr/lib /usr/local/lib)

SET(SNDFILE_FOUND FALSE)
IF(SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)
	SET(SNDFILE_FOUND TRUE)
ENDIF(SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)

IF(NOT SNDFILE_FOUND)
	IF(SNDFILE_REQUIRED)
		MESSAGE(FATAL_ERROR "** SndFile library has not been found. Please, check it from your distro media or download from http://www.mega-nerd.com/libsndfile/ and install.")
	ENDIF(SNDFILE_REQUIRED)
ENDIF(NOT SNDFILE_FOUND)
