#!/bin/sh

export SRC_PATH=`pwd`
export INSTALL_PATH="/usr/local/fgcom"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$INSTALL_PATH/lib/pkgconfig"

# Building speex
cd $SRC_PATH/speex
./configure --prefix=$INSTALL_PATH --enable-static --enable-shared
make
make install

# Building iaxclient
cd $SRC_PATH/iaxclient
autogen.sh
./configure --prefix=$INSTALL_PATH --enable-static --enable-shared --enable-local-gsm --disable-video --disable-clients --without-theora --without-vidcap --with-speex=$INSTALL_PATH
make
make install

# Building fgcom
cd $SRC_PATH/fgcom
make
make install
