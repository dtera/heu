#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
. "$CD"/versions.sh

# build seal
# [ -d benchmark ] || git clone https://github.com/microsoft/SEAL.git
# shellcheck disable=SC2154
pkg=SEAL-"$seal_ver"
download_url=https://github.com/microsoft/SEAL/archive/refs/tags/v"$seal_ver".tar.gz
sh "$CD"/build_template.sh --pkg "$pkg" -u "$download_url" -o \
"-DSEAL_USE_MSGSL=OFF -DSEAL_BUILD_DEPS=OFF -DSEAL_USE_ZSTD=OFF -DSEAL_USE_ZLIB=OFF -DSEAL_THROW_ON_TRANSPARENT_CIPHERTEXT=OFF"

# shellcheck disable=SC2115
mv "$CD"/lib64/libseal* "$CD"/lib/ && rm -rf "$CD"/lib64
m_ver=${seal_ver%.*}
rm -rf "$CD"/include/seal && mv "$CD"/include/SEAL-"$m_ver"/seal "$CD"/include/ && rm -rf "$CD"/include/SEAL-"$m_ver"
rm -rf "$CD"/lib/{cmake,pkgconfig}
