#include "Animation/Events.h"
#include "pinkcore/shared/RequirementAPI.hpp"
#include "NEHooks.h"

void InstallAndRegisterAll() {
    Logger& logger = NELogger::GetLogger();
    Hooks::InstallHooks(logger);
    NEEvents::AddEventCallbacks(logger);
    PinkCore::RequirementAPI::RegisterInstalled("Noodle Extensions");
    PinkCore::RequirementAPI::RegisterInstalled("Chroma");
}