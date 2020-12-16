$NDKPath = Get-Content ./ndkpath.txt

$buildScript = "$NDKPath/build/ndk-build"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $buildScript += ".cmd"
}

& $buildScript NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk
& adb push libs/arm64-v8a/libnoodleextensions.so /sdcard/Android/data/com.beatgames.beatsaber/files/mods/libnoodleextensions.so
& adb shell am force-stop com.beatgames.beatsaber
& adb shell am start com.beatgames.beatsaber/com.unity3d.player.UnityPlayerActivity
adb logcat -c; adb logcat > test.log