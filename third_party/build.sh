#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
echo "Current Directory: $CD"

rm -rf include/* lib/*
# build yacl
"$CD"/build_yacl.sh
