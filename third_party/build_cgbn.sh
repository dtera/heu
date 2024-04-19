#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
. "$CD"/versions.sh

# build cgbn
# [ -d cgbn ] || git clone https://github.com/NVlabs/CGBN.git
# shellcheck disable=SC2154
pkg=CGBN-"$cgbn_ver"
download_url=https://github.com/NVlabs/CGBN/archive/"$cgbn_ver".tar.gz
[ -f src/"$pkg".tar.gz ] || curl "$download_url" -L -o src/"$pkg".tar.gz
rm -rf "$pkg" && tar xvf src/"$pkg".tar.gz && cd "$pkg" || exit
cp "$CD/bazel_cpp/patches/cgbn.patch" ./
patch -p1 -i cgbn.patch
cp -R ../src/include/cgbn/arith/arith.h include/cgbn/arith/
cp -R ../src/include/cgbn/core/padded_resolver.cuh include/cgbn/core/

cd "$CD/$pkg/include" || exit
# shellcheck disable=SC2044
for path in $(find . -name "*.h" -o -name "*.cuh"); do
  #head_file=${path##*/}
  head_path=${path%/*}
  head_save_path="$CD"/include/$head_path
  mkdir -p "$head_save_path" && cp "$path" "$head_save_path"
done

cd "$CD" || exit
rm -rf "$pkg"