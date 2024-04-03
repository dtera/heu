#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit
. "$CD"/versions.sh

pkg=""
download_url=""
targets=""
build_dir="build"
cmake_opts="-DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_CXX_STANDARD=17 -DCMAKE_INSTALL_PREFIX=$CD"
is_delete_pkg=true
out_only_hdr=false

show_usage="args: [-h|--help -p|--pkg -u|--download_url -b|--build_dir -o|--cmake_opts -t|targets -d|--is_delete_pkg -H|--out_only_hdr] \n\
-h|--help         \t\t show help information  \n\
-p|--pkg          \t\t package name \n\
-u|--download_url  \t  download url for package \n\
-b|--build_dir     \t  cmake build dir \n\
-o|--cmake_opts    \t  cmake options for building package \n\
-H|--out_only_hdr  \t  whether only output header files \n\
-d|--is_delete_pkg \t  whether delete package after install package"
ARGS=`getopt -o hp:u:b:o:t:H:d: -l help,pkg:,download_url:,build_dir:,cmake_opts:,targets:,out_only_hdr:,is_delete_pkg: -n 'build_template.sh' -- "$@"`
# shellcheck disable=SC2181
if [[ $? != 0 ]]; then
  echo "Terminating..."
  exit 1
fi
eval set -- "${ARGS}"

while true
do
  case $1 in
    -h|--help) echo -e ${show_usage}; exit 0;;
    -p|--pkg) pkg="$2"; shift 2;;
    -u|--download_url) download_url="$2"; shift 2;;
    -b|--build_dir) build_dir="$2"; shift 2;;
    -o|--cmake_opts)
       if echo "$2" | grep -q "CMAKE_BUILD_TYPE"; then
         cmake_opts="$2"
       else
         cmake_opts="$cmake_opts $2"
       fi
       shift 2;;
    -t|--targets) targets="$2"; shift 2;;
    -d|--is_delete_pkg)
       if [[ "$2" == "false" || $2 == 0 ]]; then
          is_delete_pkg=false
       fi
       shift 2;;
    -H|--out_only_hdr)
       if [[ "$2" == "true" || $2 == 1 ]]; then
          out_only_hdr=true
       fi
       shift 2;;
    --) shift; break;;
    *) echo "unknown args: $1"; exit 1;;
  esac
done

[ -f src/"$pkg".tar.gz ] || curl "$download_url" -L -o src/"$pkg".tar.gz
rm -rf "$pkg" && tar xvf src/"$pkg".tar.gz

if ${out_only_hdr}; then
  cp -R "$pkg"/include/* ./include/
else
  cd "$pkg" && mkdir "$build_dir" && cd "$build_dir" || exit
  cmake $cmake_opts ..
  make -j8 $targets && make install
fi

cd "$CD" || exit
if ${is_delete_pkg}; then
  rm -rf "$pkg"
fi
