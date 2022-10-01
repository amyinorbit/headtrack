#!/usr/bin/env zsh

ROOT_DIR=$(pwd)
OUT_DIR=$ROOT_DIR
PLUGIN_DIR=$OUT_DIR/htrack
GIT_REV=$(git describe --tags --always)
# GIT_REV=$(git rev-parse --short --verify HEAD)
BUILD_ID="$(date +"%m%d")-$GIT_REV"

BUILD_ZIP="htrack-$BUILD_ID.zip"

echo "Packaging..."
cd $OUT_DIR
zip -rq $BUILD_ZIP htrack
rm -f $INFO_FILE
