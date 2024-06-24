#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
echo "Current Directory: $CD"
[ -d src ] || mkdir src
PREFIX=$([[ "$1" == "" ]] && echo "$CD" || echo "$1")

rm -rf include lib && mkdir include lib
# build gmp
"$CD"/build_gmp.sh "$PREFIX"
# build yacl
"$CD"/build_yacl.sh "$PREFIX"
# build seal
"$CD"/build_seal.sh
# patches
"$CD"/patches.sh

rm -rf "$CD"/lib/{cmake,pkgconfig,engines-1.1} "$CD"/{share,lib54}