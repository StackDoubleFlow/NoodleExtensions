#pragma once
#include "NELogger.h"

class Hooks {
private:
  static inline std::vector<void (*)()> installFuncs;

  static inline bool NoodleHookEnabled;

public:
  static void AddInstallFunc(void (*installFunc)()) {
    installFuncs.push_back(installFunc);
  }

  static void InstallHooks() {
    for (auto installFunc : installFuncs) {
      installFunc();
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
      NELogger::Logger.info("NEHooks Registered install func: " #func);                                                \
    }                                                                                                                  \
  };                                                                                                                   \
  static __NERegister##func __NERegisterInstance##func;

void InstallAndRegisterAll();
