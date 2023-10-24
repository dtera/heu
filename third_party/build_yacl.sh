#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
echo "Current Directory: $CD"

# build yacl
# [ -d yacl ] || https://github.com/dtera/yacl.git
pkg=yacl
[ -f src/"$pkg".tar.gz ] || curl https://github.com/dtera/yacl/releases/download/v1.0.0/"$pkg".tar.gz -L -o src/"$pkg".tar.gz
rm -rf "$pkg" && tar xvf src/"$pkg".tar.gz && "$pkg"/third_party/build.sh
lib=".so"
# [[ "$OSTYPE" == "darwin"* ]] && lib=".dylib"
rm -f "$CD"/$pkg/third_party/lib/libssl"$lib" && \
cp "$CD"/$pkg/third_party/lib/libssl.1.1"$lib" "$CD"/$pkg/third_party/lib/libssl"$lib" && \
rm -f "$CD"/$pkg/third_party/lib/libssl.1.1"$lib"
[ -d "$CD"/lib ] || mkdir -p "$CD"/lib
[ -d "$CD"/include ] || mkdir -p "$CD"/include
cp -R "$CD"/$pkg/third_party/lib/* "$CD"/lib/ && cp -R "$CD"/$pkg/third_party/include/* "$CD"/include/

cd "$CD"/"$pkg" || exit 0
# shellcheck disable=SC2044
for path in $(find $pkg -name "*.h"); do
  #head_file=${path##*/}
  head_path=${path%/*}
  head_save_path="$CD"/include/$head_path
  mkdir -p "$head_save_path" && cp "$path" "$head_save_path"
done

cd "$CD"/$pkg && rm -rf build && mkdir build && cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX="$CD/" ..
make -j8 "$pkg"

rm -rf "$CD"/lib/cmake && rm -rf "$CD"/lib/pkgconfig
cp "$CD"/$pkg/build/lib"$pkg".* "$CD"/lib/
cd "$CD" || exit
#rm -rf "$pkg"
