#pragma once
#include "NELogger.h"

class Hooks {
private:
    static std::vector<void (*)(Logger& logger)> installFuncs;
public:
    static void AddInstallFunc(void (*installFunc)(Logger& logger)) {
        installFuncs.push_back(installFunc);
    }

    static void InstallHooks(Logger& logger) {
        for (auto installFunc : installFuncs) {
            installFunc(logger);
        }
    }
};

#define NEInstallHooks(func) \
struct __NERegister##func { \
    __NERegister##func() { \
        Hooks::AddInstallFunc(func); \
    } \
}; \
static __NERegister##func __NERegisterInstance##func;

void InstallAndRegisterAll();