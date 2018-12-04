#!/bin/sh
# Clone libpng from github, if ../libpng does not exits, git worktree for branches.
# Build with cmake in ../_build/libpng<version> and run resulting binaries.
# Set environment EXTRA_CHECKS=ON to have extra filters checks.

# $0 is not reliable, but ...
cd "$(dirname $0)"
SRC_DIR=$PWD
echo "SRC_DIR=$SRC_DIR"
cd ..
pwd

BRANCHES=${BRANCHES:="libpng12 libpng15 libpng16 libpng17"}
# Where to build in subdirectory with branch name.
BUILD_ROOT_SUBDIR=${BUILD_ROOT_SUBDIR:="_build"}
LIBPNG_GIT_DIR=${LIBPNG_GIT_DIR:="libpng"}
LIBPNG_GITHUB="https://github.com/glennrp/libpng.git"

# Clone to libpng subdir, if it does not already exists.
if [ ! -d $LIBPNG_GIT_DIR ]; then
  git clone $LIBPNG_GITHUB $LIBPNG_GIT_DIR \
  || { echo "Failed git clone to $LIBPNG_GIT_DIR from $LIBPNG_GITHUB"; exit 1; }
fi

# git worktree for BRANCHES
for b in $BRANCHES; do
  [ ! -d $b ] && (
    cd $LIBPNG_GIT_DIR
    # Take first remote branch, if any.
    ref=$(git branch --list -r */${b} | awk '{if(NR==1) {$1=$1;print}}';)
    # If not found use as <commit-ish>
    [ "$ref" = "" ] && ref="${b}"
    echo "Make git worktree in ../$b from '$ref'"
    git worktree add -f ../$b $ref;
  )
  [ ! -d $BUILD_ROOT_SUBDIR/$b ] && mkdir -p $BUILD_ROOT_SUBDIR/$b
  ( cd $BUILD_ROOT_SUBDIR/$b; pwd;
    rm CMakeCache.txt
    CMD="cmake -DCMAKE_BUILD_TYPE=Release -DLIBPNG_SRC_DIR=$SRC_DIR/../$b";
    [ "ON" = "$EXTRA_CHECKS" ] && CMD="$CMD -DEXTRA_PNG_FILTERS_CHECKS=ON"
    echo "$CMD $SRC_DIR"; eval "$CMD $SRC_DIR";
    cmake --build . --target all
  )
done

for b in $BRANCHES; do
  ls -gG $BUILD_ROOT_SUBDIR/$b/set_filter_check
done

for b in $BRANCHES; do
  $BUILD_ROOT_SUBDIR/$b/set_filter_check
done
