#
# IAX FGCOM client - libspeex CMakeLists.txt
# Charles INGELS <charles@maisonblv.net>
# December 2008
#
#
# We set the name of the library that will be produced.
#
SET(SPEEX_NAME "speex")
#
# We set path to cmake modules that we use to find dependencies.
#
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules")
#
# Since Speex uses some fast Fourier transform (FFT), math lib is required.
#
FIND_PACKAGE(LIBMATH REQUIRED)

#
# Default behaviour is to use the Small FT.
#
IF(NOT USE_KISS_FFT)
	SET(USE_SMALLFT 1)
	SET(USE_KISS_FFT 0)
ELSE(NOT USE_KISS_FFT)
	SET(USE_KISS_FFT 1)
	SET(USE_SMALLFT 0)
ENDIF(NOT USE_KISS_FFT)
#
# The file 'config.h.cmake' is processed to produce the file 'config.h' which will be included in various
# source file. Such file defines some #define directives in order to modify behaviour.
#
CONFIGURE_FILE (${PROJECT_SOURCE_DIR}/libspeex/config.h.cmake ${PROJECT_SOURCE_DIR}/libspeex/config.h)
#
# Depending on what kind of FFT is selected, we do not include same source files.
#
IF(USE_SMALLFT)
SET(SPEEX_SRC bits.c exc_5_64_table.c hexc_10_32_table.c lsp.c preprocess.c speex_callbacks.c buffer.c exc_8_128_table.c hexc_table.c lsp_tables_nb.c quant_lsp.c speex_header.c cb_search.c fftwrap.c high_lsp_tables.c ltp.c resample.c stereo.c vbr.c exc_10_16_table.c filterbank.c jitter.c mdf.c sb_celp.c vq.c exc_10_32_table.c filters.c modes.c scal.c window.c exc_20_32_table.c gain_table.c modes_wb.c smallft.c exc_5_256_table.c gain_table_lbr.c lpc.c nb_celp.c speex.c)
ELSE(USE_SMALLFT) 
SET(SPEEX_SRC bits.c exc_5_64_table.c hexc_10_32_table.c lsp.c preprocess.c speex_callbacks.c buffer.c exc_8_128_table.c hexc_table.c lsp_tables_nb.c quant_lsp.c speex_header.c cb_search.c fftwrap.c high_lsp_tables.c ltp.c resample.c stereo.c vbr.c exc_10_16_table.c filterbank.c jitter.c mdf.c sb_celp.c vq.c exc_10_32_table.c filters.c kiss_fft.c modes.c scal.c window.c exc_20_32_table.c gain_table.c kiss_fftr.c modes_wb.c exc_5_256_table.c gain_table_lbr.c lpc.c nb_celp.c speex.c)
ENDIF(USE_SMALLFT)
#
# We set the include directories.
#
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/libspeex ${PROJECT_SOURCE_DIR}/libspeex/speex)
# 
# Here, we specify the libraries to be produced.
#
ADD_LIBRARY(${SPEEX_NAME} STATIC ${SPEEX_SRC})
ADD_LIBRARY(${SPEEX_NAME}-dynamic SHARED ${SPEEX_SRC})
SET_TARGET_PROPERTIES(${SPEEX_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(${SPEEX_NAME}-dynamic PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(${SPEEX_NAME}-dynamic PROPERTIES OUTPUT_NAME "${SPEEX_NAME}")
#
# Here, we add targets for test executable to be built.
#
ADD_EXECUTABLE(testdenoise testdenoise.c)
ADD_EXECUTABLE(testjitter testjitter.c)
ADD_EXECUTABLE(testecho testecho.c)
ADD_EXECUTABLE(testenc testenc.c)
ADD_EXECUTABLE(testenc_uwb testenc_uwb.c)
ADD_EXECUTABLE(testenc_wb testenc_wb.c)
#
# For each executable, we need to indicate where to find requested libraries that must
# be linked against.
#
SET(EXE_LIST testdenoise testjitter testecho testenc testenc_uwb testenc_wb)
FOREACH(EXE ${EXE_LIST})
	TARGET_LINK_LIBRARIES(${EXE} ${PROJECT_SOURCE_DIR}/libspeex/libspeex.a ${LIBMATH_LIBRARY})
ENDFOREACH(EXE)

MESSAGE(STATUS)
MESSAGE(STATUS "------------------------------------------------------------")
MESSAGE(STATUS "Speex source dir		: ${PROJECT_SOURCE_DIR}/libspeex")
IF(USE_SMALLFT)
	MESSAGE(STATUS "Use Small FT			: yes")
ELSE(USE_SMALLFT)
	MESSAGE(STATUS "Use Small FT			: no")
ENDIF(USE_SMALLFT)

IF(USE_KISS_FFT)
	MESSAGE(STATUS "Use Kiss FFT			: yes")
ELSE(USE_KISS_FFT)
	MESSAGE(STATUS "Use Kiss FFT			: no")
ENDIF(USE_KISS_FFT)

MESSAGE(STATUS)
MESSAGE(STATUS "Default FFT in use is Small FT.")
MESSAGE(STATUS "To set a FFT method, type 'cmake -DUSE_SMALLFT=1 .' or")
MESSAGE(STATUS "'cmake -DUSE_KISS_FFT=1 .' to select correct method.")
MESSAGE(STATUS "Note that you must use at least one method, and you *must not*")
MESSAGE(STATUS "use both methods, it will not compile !")
MESSAGE(STATUS "Do not forget the '.' at the end of the 'cmake' directive !")
#
# End of file.
#
