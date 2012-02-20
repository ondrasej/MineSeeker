#!/bin/bash

tar xfz gflags-2.0.tar.gz
cd gflags-2.0

TARGET_PATH=`dirname "$0"`/../..
ABS_TARGET_PATH=`cd "$TARGET_PATH"; pwd`

./configure --prefix="$ABS_TARGET_PATH" "$@"
make install

cd ..
rm -rf gflags-2.0
