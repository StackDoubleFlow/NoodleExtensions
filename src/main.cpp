#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "NoodleExtensions/NEHooks.h"
#include "CustomJSONData/CustomJSONDataHooks.h"
#include "NELogger.h"

#include <string>
#include <iostream>

extern "C" void setup(ModInfo &info) {
    info.id = "NoodleExtensions";
    info.version = "0.1.0";
    NELogger::modInfo = info;
}

extern "C" void load() {
    NELogger::GetLogger().info("Installing NoodleExtensions Hooks!");

    // This prevents any and all Utils logging
    Logger::get().options.silent = true;

    // Install hooks

    CustomJSONData::InstallHooks();
    NoodleExtensions::InstallHooks();

    NELogger::GetLogger().info("Installed NoodleExtensions Hooks!");
}