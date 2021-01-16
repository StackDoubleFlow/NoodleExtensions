#!/bin/bash

NDKPath=`cat ndkpath.txt`

buildScript="$NDKPath/build/ndk-build"

./$buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk
mkdir BMBFBuild
cp "./bmbfmod.json" "./libs/arm64-v8a/libnoodleextensions.so" "./libs/arm64-v8a/libbeatsaber-hook_0_8_4.so" "./libs/arm64-v8a/libcustom-types.so" ./BMBFBuild
cd BMBFBuild
zip "../NoodleExtensions.zip" *
cd ..
rm -r BMBFBuild