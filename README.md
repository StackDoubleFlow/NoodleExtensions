# Noodle Extensions

A port of the Noodle Extensions mod made originally for the PC version of Beat Saber to the Oculus Quest platform.

## Building from Source

You'll need [qpm_rust](https://github.com/RedBrumbler/QuestPackageManager-Rust), powershell, and cmake.

You'll also need the latest Android NDK. Create a file called `ndkpath.txt` containing the path to the directly where you extracted the ndk to.

```sh
qpm-rust restore
qpm-rust cache legacy-fix
pwsh ./build.ps1
pwsh ./createqmod.ps1 noodle_extensions
```
