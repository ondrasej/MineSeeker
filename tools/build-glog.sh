#!/bin/bash

tar xfz glog-0.3.1-1.tar.gz
cd glog-0.3.1

TARGET_PATH=`dirname "$0"`/../..
ABS_TARGET_PATH=`cd "$TARGET_PATH"; pwd`

./configure --prefix="$ABS_TARGET_PATH" "$@"
make install

cd ..
rm -rf glog-0.3.1
