#!/bin/sh
# Clone libpng from github, if ../libpng does not exits, git worktree for branches.

# $0 is not reliable, but ...
cd "$(dirname $0)"
SRC_DIR=$PWD
echo "SRC_DIR=$SRC_DIR"
cd ..
pwd

BRANCHES=${BRANCHES:="libpng12 libpng15 libpng16"}
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
  [ ! -d $b ] && (cd $LIBPNG_GIT_DIR; git worktree add -f ../$b origin/$b; )
  [ ! -d $BUILD_ROOT_SUBDIR/$b ] && mkdir -p $BUILD_ROOT_SUBDIR/$b
done
