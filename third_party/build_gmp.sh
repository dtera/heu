#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
. "$CD"/versions.sh
GMP_PREFIX=$([[ "$1" == "" ]] && echo "$CD" || echo "$1")

# build gmp
# shellcheck disable=SC2154
pkg=gmp-"$gmp_ver"
[ -f src/"$pkg".tar.gz ] || curl https://gmplib.org/download/gmp/"$pkg".tar.xz -L -o src/"$pkg".tar.gz
rm -rf "$pkg" && tar xvf src/"$pkg".tar.gz && cd "$pkg" || exit

./configure --enable-fat --prefix="$GMP_PREFIX"
make -j8 && make install
if [[ "$GMP_PREFIX" != "$CD" ]]; then
  [ -f "$CD/include/gmp.h" ] || mv "$GMP_PREFIX/include/gmp.h" "$CD/include/"
  mv "$GMP_PREFIX/lib/libgmp."* "$CD/lib/"
  rm -rf "$GMP_PREFIX"/lib/{cmake,pkgconfig}
fi

cd "$CD" || exit
rm -rf "$pkg"
