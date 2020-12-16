
$NDKPath = Get-Content ./ndkpath.txt

$stackScript = "$NDKPath/ndk-stack"
if (-not ($PSVersionTable.PSEdition -eq "Core")) {
    $stackScript += ".cmd"
}

Get-Content ./test.log | & $stackScript -sym ./obj/local/arm64-v8a/ > test_unstripped.log
