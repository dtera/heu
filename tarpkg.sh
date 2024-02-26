#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit

pkg=heu
mkdir -p tmp/$pkg
cp -R include third_party $pkg CMakeLists.txt tmp/$pkg/
# shellcheck disable=SC2164
pushd tmp
rm -rf $pkg/third_party/{include,lib,bin,share,yacl,bazel_cpp,bazel_rust}
if [[ "$OSTYPE" == "darwin"* ]]; then
  gtar cvf $pkg.tar.gz $pkg
else
  tar cvf $pkg.tar.gz $pkg
fi
rm -f ../$pkg.tar.gz && mv $pkg.tar.gz ..
# shellcheck disable=SC2164
popd
rm -rf tmp