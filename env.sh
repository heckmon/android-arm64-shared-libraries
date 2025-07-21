#/bin/bash

SELF=$(cd `dirname $0`; pwd)

export NDK="/home/athul/Android/ndk/28.1.13356709"
export ENVSRCTARBALL="/home/athul/Downloads/dna2oslab/android/buildv2/sources"
export HOST_GCC_DIR="/usr/bin"
export ENVDISTBIN="dist"
export ENVHOST=linux-x86_64
export ENVTARGET=aarch64-linux-android
export ENVANDROIDVER=35
export CROSS_COMPILE=" --build=x86_64-linux --host=arm64-v8a --target=arm64-v8a "

if [ -f $SELF/local/env.sh ]; then
   source $SELF/local/env.sh
fi

if [ "$NDK" == "" ]; then
   echo 'check:    ENVHOST, ENVTARGET, ENVANDROIDVER'
   echo
   echo 'run cmd:  mkdir local; touch local/env.sh'
   echo 'fill:     NDK, ENVSRCTARBALL'
   echo 'optional: HOST_GCC_DIR'
   exit 0
fi

export COMPILERDIR="$NDK/toolchains/llvm/prebuilt/$ENVHOST/bin"
export CC="$COMPILERDIR/${ENVTARGET}${ENVANDROIDVER}-clang"
export CXX="$COMPILERDIR/${ENVTARGET}${ENVANDROIDVER}-clang++"
export LD="$COMPILERDIR/$ENVTARGET-ld"
export AS="$COMPILERDIR/$ENVTARGET-as"
export AR="$COMPILERDIR/llvm-ar"
export STRIP="$COMPILERDIR/llvm-strip"
export OBJCOPY="$COMPILERDIR/llvm-objcopy"
export OBJDUMP="$COMPILERDIR/llvm-objdump"
export RANLIB="$COMPILERDIR/llvm-ranlib"
export NM="$COMPILERDIR/llvm-nm"
export STRINGS="$COMPILERDIR/llvm-strings"
export READELF="$COMPILERDIR/llvm-readelf"

export ANDROID="$COMPILERDIR/../sysroot"

function make_install() {
# $1: package name
  make install
  rm -rf ../$ENVDISTBIN/$1
  mkdir -p ../$ENVDISTBIN/$1
  mv dist/* ../$ENVDISTBIN/$1/
  cd ..
  rm -rf $1
}

function fetch_source() {
# $1: package file name, e.g. vim-7.4.0001.tar.gz
# $2: source url
  test -f "$ENVSRCTARBALL/$1" || curl -k -L -o "$ENVSRCTARBALL/$1" "$2"
  test -f "$ENVSRCTARBALL/$1" || exit 1
}
