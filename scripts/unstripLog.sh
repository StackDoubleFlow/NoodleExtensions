#!/bin/bash

NDKPath=`cat ndkpath.txt`

cat ./test.log | $NDKPath/ndk-stack -sym ./obj/local/arm64-v8a/ > test_unstripped.log