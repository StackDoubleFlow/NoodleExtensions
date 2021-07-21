// It doesn't like it if i include this after
#include "NEConfig.h"

#include "modloader/shared/modloader.hpp"

#include "NEHooks.h"
#include "NELogger.h"

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = VERSION;
    NELogger::modInfo = info;
    getNEConfig().Init(info);
}

void InstallNEConfigHooks(Logger &logger);

extern "C" void load() {
    NELogger::GetLogger().info("Installing NoodleExtensions Hooks!");
    // This prevents any and all Utils logging
    // Logger::get().options.silent = false;

    // Install hooks

    InstallAndRegisterAll();

    // what the fuck
    InstallNEConfigHooks(NELogger::GetLogger());

    NELogger::GetLogger().info("Installed NoodleExtensions Hooks!");
}
