#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
echo "Current Directory: $CD"

# build yacl
# [ -d yacl ] || https://github.com/dtera/yacl.git
pkg=yacl
rm -rf "$pkg" && tar xvf "$pkg".tar.gz && "$pkg"/third_party/build.sh
[ -d "$CD"/lib ] || mkdir -p "$CD"/lib
[ -d "$CD"/include ] || mkdir -p "$CD"/include
if [ "$(uname)" == "Darwin" ]; then
  cp -R "$CD"/$pkg/third_party/lib/* "$CD"/lib/
else
  cp -dR "$CD"/$pkg/third_party/lib/* "$CD"/lib/
  cp -dR "$CD"/$pkg/third_party/lib64/* "$CD"/lib/
fi
cp -R "$CD"/$pkg/include/* "$CD"/include/
cp -R "$CD"/$pkg/third_party/include/* "$CD"/include/
rm -rf "$CD"/lib/cmake && rm -rf "$CD"/lib/pkgconfig
cd "$CD"/$pkg && mkdir build && cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX="$CD/" ..
make -j8 yacl

cp "$CD"/$pkg/build/libyacl.* "$CD"/lib/
cd "$CD" || exit
rm -rf "$pkg"
