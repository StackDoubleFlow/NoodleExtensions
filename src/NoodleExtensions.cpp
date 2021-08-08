#include "Animation/Events.h"
#include "NEHooks.h"

void InstallAndRegisterAll() {
    Logger& logger = NELogger::GetLogger();
    Hooks::InstallHooks(logger);
    NEEvents::AddEventCallbacks(logger);
}