
# SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/cmake_modules")
SET(GSM_SRC_FILES add.c debug.c gsm_create.c gsm_destroy.c gsm_explode.c gsm_option.c long_term.c preprocess.c short_term.c code.c decode.c gsm_decode.c gsm_encode.c gsm_implode.c gsm_print.c lpc.c rpe.c table.c)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/gsm/inc)
ADD_LIBRARY(gsm STATIC ${GSM_SRC_FILES})
ADD_LIBRARY(gsm-dynamic SHARED ${GSM_SRC_FILES})
SET_TARGET_PROPERTIES(gsm PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(gsm-dynamic PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(gsm-dynamic PROPERTIES OUTPUT_NAME "gsm")


MESSAGE(STATUS)
MESSAGE(STATUS "------------------------------------------------------------")
MESSAGE(STATUS "libgsm source dir		: ${PROJECT_SOURCE_DIR}/${IAX_CLIENT_DIR}/gsm/src")
# IF(CMAKE_BUILD_TYPE STREQUAL "debug")
# 	MESSAGE(STATUS "Src files			: ${GSM_SRC_FILES}")
# ENDIF(CMAKE_BUILD_TYPE STREQUAL "debug")

