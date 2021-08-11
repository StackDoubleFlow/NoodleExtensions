#!/bin/bash

cat ./test.log | $ANDROID_NDK_HOME/ndk-stack -sym ./obj/local/arm64-v8a/ | sed '/??/d' > test_unstripped.log