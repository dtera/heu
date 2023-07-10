#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit

# build libtommath
# [ -d libtommath ] || git clone https://github.com/libtom/libtommath.git
rm -rf libtommath && tar xvf libtommath.tar.gz && cd libtommath && mkdir build && cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX="$CD/" ..
make -j8 && make install
cd "$CD" || exit
rm -rf libtommath
