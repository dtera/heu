#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
WD=$(cd "$(dirname "$CD")" || exit && pwd)
echo "Current Directory: $CD"
echo "Work Directory: $WD"

[ "$1" == "build" ] && cd "$WD" && bazel build heu/library/... -c opt && (cd - || exit)

BAZEL_DIR="$WD/bazel-bin"
LIB_DIR="$BAZEL_DIR/heu/library"
EXT_LIB_DIR="$BAZEL_DIR/external"
LIB_OUT_DIR="$WD/libs"
LIB_SUFFIX=a     # dylib so a
OUT_LIB_SUFFIX=a # dylib so a
LINK_T=static    # dynamic static

for module in algorithms phe numpy; do
  out_path="$LIB_OUT_DIR"/libheu-"$module"."${OUT_LIB_SUFFIX}"
  # shellcheck disable=SC2038
  [ -e "$out_path" ] || find "$LIB_DIR/$module" -name "*.${LIB_SUFFIX}" | xargs libtool -"$LINK_T" -o "$out_path"
done

#com_github_gflags_gflags com_github_google_benchmark
for m_path in com_github_fmtlib_fmt/copy_fmtlib/fmtlib/lib \
  com_github_libtom_libtommath/copy_libtommath/libtommath/lib yacl; do
  module=${m_path%%/*}
  out_path="$LIB_OUT_DIR"/libext-"${module##*_}"."${OUT_LIB_SUFFIX}"
  # shellcheck disable=SC2038
  [ -e "$out_path" ] || find "$EXT_LIB_DIR"/"$m_path" -name "*.${LIB_SUFFIX}" | xargs libtool -"$LINK_T" -o "$out_path"
done

out_path="$LIB_OUT_DIR"/libheu."${OUT_LIB_SUFFIX}"
# shellcheck disable=SC2038
[ -e "$out_path" ] || find "$LIB_OUT_DIR" -name "*.${OUT_LIB_SUFFIX}" | xargs libtool -"$LINK_T" -o "$out_path"
