#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
. "$CD"/versions.sh

# build gmp
pkg=gmp-"$gmp_ver"
[ -f src/"$pkg".tar.gz ] || curl https://gmplib.org/download/gmp/"$pkg".tar.xz -L -o src/"$pkg".tar.gz
rm -rf "$pkg" && tar xvf src/"$pkg".tar.gz && cd "$pkg" || exit

./configure --enable-fat --prefix="$CD/"
make -j8 && make install
cd "$CD" || exit
rm -rf "$pkg"
