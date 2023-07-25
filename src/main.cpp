#include "modloader/shared/modloader.hpp"

#include "NEConfig.h"
#include "NEHooks.h"
#include "NELogger.h"

extern "C" void setup(ModInfo& info) {
  info.id = "NoodleExtensions";
  info.version = VERSION;
  NELogger::modInfo = info;
  getNEConfig().Init(info);
}

extern "C" void load() {
  NELogger::GetLogger().info("Installing NoodleExtensions Hooks!");

  // Install hooks
  InstallAndRegisterAll();

  NELogger::GetLogger().info("Installed NoodleExtensions Hooks!");
}
