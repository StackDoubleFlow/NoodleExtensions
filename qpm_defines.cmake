# YOU SHOULD NOT MANUALLY EDIT THIS FILE, QPM WILL VOID ALL CHANGES
# Version defines, pretty useful
set(MOD_VERSION "1.0.0")
# take the mod name and just remove spaces, that will be MOD_ID, if you don't like it change it after the include of this file
set(MOD_ID "NoodleExtensions")

# derived from override .so name or just id_version
set(COMPILE_ID "noodleextensions")
# derived from whichever codegen package is installed, will default to just codegen
set(CODEGEN_ID "codegen")

# given from qpm, automatically updated from qpm.json
set(EXTERN_DIR_NAME "extern")
set(SHARED_DIR_NAME "shared")

# if no target given, use Debug
if (NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

# defines used in ninja / cmake ndk builds
if (NOT DEFINED CMAKE_ANDROID_NDK)
	if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/ndkpath.txt")
		file (STRINGS "ndkpath.txt" CMAKE_ANDROID_NDK)
	else()
		if(EXISTS $ENV{ANDROID_NDK_HOME})
			set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_HOME})
		elseif(EXISTS $ENV{ANDROID_NDK_LATEST_HOME})
			set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_LATEST_HOME})
		endif()
	endif()
endif()
if (NOT DEFINED CMAKE_ANDROID_NDK)
	message(Big time error buddy, no NDK)
endif()
message(Using NDK ${CMAKE_ANDROID_NDK})
string(REPLACE "\\" "/" CMAKE_ANDROID_NDK ${CMAKE_ANDROID_NDK})

set(ANDROID_PLATFORM 24)
set(ANDROID_ABI arm64-v8a)
set(ANDROID_STL c++_static)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_ANDROID_NDK}/build/cmake/android.toolchain.cmake)
# define used for external data, mostly just the qpm dependencies
set(EXTERN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${EXTERN_DIR_NAME})
set(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${SHARED_DIR_NAME})
# get files by filter recursively
MACRO(RECURSE_FILES return_list filter)
	FILE(GLOB_RECURSE new_list ${filter})
	SET(file_list "")
	FOREACH(file_path ${new_list})
		SET(file_list ${file_list} ${file_path})
	ENDFOREACH()
	LIST(REMOVE_DUPLICATES file_list)
	SET(${return_list} ${file_list})
ENDMACRO()