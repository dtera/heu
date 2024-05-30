#!/usr/bin/env bash

CD=$(cd "$(dirname "$0")" || exit && pwd)
cd "$CD" || exit

# shellcheck disable=SC2034
gmp_ver=6.3.0
# shellcheck disable=SC2034
seal_ver=4.1.2
# shellcheck disable=SC2034
cgbn_ver=e8b9d265c7b84077d02340b0986f3c91b2eb02fb
