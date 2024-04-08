#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit

os_release=$(awk -F= '/^ID=/{print $2}' /etc/os-release)
if [[ "$os_release" == "ubuntu" ]]; then
  apt-get update && apt-get install -y glibc-static libstdc++-static
else
  yum install -y glibc-static libstdc++-static
fi
