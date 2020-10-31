#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

static ModInfo modInfo;

const Logger &getLogger() {
    static const Logger &logger(modInfo);
    return logger;
}

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = "0.1.0";
    modInfo = info;
}

extern "C" void load() {
    getLogger().info("Installing NoodleExtensions Hooks!");


    getLogger().info("Installed NoodleExtensions Hooks!");
}