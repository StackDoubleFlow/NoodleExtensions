#include "Animation/Events.h"
#include "NEHooks.h"

std::vector<void (*)(Logger& logger)> Hooks::installFuncs;

void InstallAndRegisterAll() {
    Logger& logger = NELogger::GetLogger();
    Hooks::InstallHooks(logger);
    NEEvents::AddEventCallbacks(logger);
}