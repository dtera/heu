#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit

# build fmt
# [ -d fmt ] || git clone https://github.com/fmtlib/fmt.git
rm -rf fmt-8.1.1 && tar xvf fmt-8.1.1.tar.gz && cd fmt-8.1.1 && mkdir build && cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX="$CD/" ..
make -j8 && make install
cd "$CD" || exit
rm -rf fmt-8.1.1
