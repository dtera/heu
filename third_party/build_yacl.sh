#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
echo "Current Directory: $CD"
PREFIX=$([[ "$1" == "" ]] && echo "$CD" || echo "$1")

# build yacl
# [ -d yacl ] || https://github.com/dtera/yacl.git
pkg=yacl
[ -f src/"$pkg".tar.gz ] || curl https://github.com/dtera/yacl/releases/download/v1.0.3/"$pkg".tar.gz -L -o src/"$pkg".tar.gz
rm -rf "$pkg" && tar xvf src/"$pkg".tar.gz && "$pkg"/third_party/build.sh "$PREFIX"
#"$pkg"/third_party/build_openssl.sh "$PREFIX"
#mv "$CD/$pkg/third_party/lib/libssl"* "$CD"/lib/ && mv "$CD/$pkg/third_party/lib/libcrypto"* "$CD"/lib/
cp -R "$CD"/$pkg/third_party/lib/* "$CD"/lib/ && cp -R "$CD"/$pkg/third_party/include/* "$CD"/include/

cd "$CD"/"$pkg" || exit 0
# shellcheck disable=SC2044
for path in $(find $pkg -name "*.h"); do
  #head_file=${path##*/}
  head_path=${path%/*}
  head_save_path="$CD"/include/$head_path
  mkdir -p "$head_save_path" && cp "$path" "$head_save_path"
done

if echo "$OSTYPE" | grep -q "linux" || [[ "$OSTYPE" == "" ]]; then
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"$CD"/lib
  ldconfig
fi

cd "$CD"/$pkg && rm -rf build && mkdir build && cd build || exit
if [[ "$OSTYPE" == "darwin"* ]]; then
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX="$CD" ..
  cmake --build . -j 8 --target "$pkg"
else
  cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX="$CD" ..
  make -j8 "$pkg"
fi

cp "$CD"/$pkg/build/lib"$pkg".* "$CD"/lib/
cd "$CD" || exit
rm -rf "$pkg"
