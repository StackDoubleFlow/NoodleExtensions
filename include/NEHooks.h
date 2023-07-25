#pragma once
#include "NELogger.h"

class Hooks {
private:
  static inline std::vector<void (*)(Logger& logger)> installFuncs;

  static inline bool NoodleHookEnabled;

public:
  static void AddInstallFunc(void (*installFunc)(Logger& logger)) {
    installFuncs.push_back(installFunc);
  }

  static void InstallHooks(Logger& logger) {
    for (auto installFunc : installFuncs) {
      installFunc(logger);
    }
  }

  static bool isNoodleHookEnabled() {
    return NoodleHookEnabled;
  }

  static constexpr void setNoodleHookEnabled(bool noodleHookEnabled) {
    NoodleHookEnabled = noodleHookEnabled;
  }
};

#define NEInstallHooks(func)                                                                                           \
  struct __NERegister##func {                                                                                          \
    __NERegister##func() {                                                                                             \
      Hooks::AddInstallFunc(func);                                                                                     \
      __android_log_print(ANDROID_LOG_DEBUG, "NEInstallHooks", "Registered install func: " #func);                     \
    }                                                                                                                  \
  };                                                                                                                   \
  static __NERegister##func __NERegisterInstance##func;

void InstallAndRegisterAll();