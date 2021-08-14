#!/bin/bash

NDKPath=`cat ndkpath.txt`

buildScript="$NDKPath/build/ndk-build"

./$buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk
mkdir BMBFBuild
cp "./mod.json" "./libs/arm64-v8a/libnoodleextensions.so" ./BMBFBuild
cp "./libs/arm64-v8a/libcustom-json-data.so" ./BMBFBuild
cp "./libs/arm64-v8a/libtracks.so" ./BMBFBuild
cd BMBFBuild
zip "../NoodleExtensions.qmod" *
cd ..
rm -r BMBFBuild