#!/bin/bash

mkdir BMBFBuild
cp "./mod.json" "./libs/arm64-v8a/libnoodleextensions.so" ./BMBFBuild
cp "./libs/arm64-v8a/libcustom-json-data.so" ./BMBFBuild
cp "./libs/arm64-v8a/libtracks.so" ./BMBFBuild
cd BMBFBuild
zip "../NoodleExtensions.qmod" *
cd ..
rm -r BMBFBuild