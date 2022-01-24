#include "Animation/Events.h"
#include "pinkcore/shared/RequirementAPI.hpp"
#include "NEHooks.h"
#include "NECaches.h"

float NECaches::noteJumpMovementSpeed;
float NECaches::noteJumpStartBeatOffset;
bool NECaches::LeftHandedMode;

void InstallAndRegisterAll() {
    Logger& logger = NELogger::GetLogger();
    Hooks::InstallHooks(logger);
    NEEvents::AddEventCallbacks(logger);
    PinkCore::RequirementAPI::RegisterInstalled("Noodle Extensions");
}