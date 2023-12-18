#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit

f_path="include/yacl/math/mpint/mp_int.h"
row_num=$(awk -v pattern="mp_int n_;" '$0 ~ pattern {print NR-1}' "$f_path")
sed -i '' "${row_num}s/protected:/public:/" "$f_path"
