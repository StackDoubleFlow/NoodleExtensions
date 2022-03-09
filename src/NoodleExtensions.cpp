#include "Animation/Events.h"
#include "pinkcore/shared/RequirementAPI.hpp"
#include "pinkcore/shared/API.hpp"
#include "NEHooks.h"
#include "NECaches.h"

float NECaches::noteJumpMovementSpeed;
float NECaches::noteJumpStartBeatOffset;
bool NECaches::LeftHandedMode;

void InstallAndRegisterAll() {
    Logger& logger = NELogger::GetLogger();
    Hooks::InstallHooks(logger);
    NEEvents::AddEventCallbacks(logger);
    PinkCore::RequirementAPI::RegisterInstalled(NoodleExtensions::U8_REQUIREMENTNAME);

    Modloader::requireMod("MappingExtensions");
    if (!Modloader::getMods().contains("MappingExtensions")) return;

    PinkCore::API::GetFoundRequirementCallbackSafe() += [](const std::vector<std::string>& requirements){
        bool meRequirement = std::any_of(requirements.begin(), requirements.end(), [](auto const& s) {return s == NoodleExtensions::U8_ME_REQUIREMENTNAME; });
        bool neRequirement = std::any_of(requirements.begin(), requirements.end(), [](auto const& s) {return s == NoodleExtensions::U8_ME_REQUIREMENTNAME; });

        if (meRequirement && neRequirement) {
            PinkCore::RequirementAPI::RemoveInstalled(NoodleExtensions::U8_REQUIREMENTNAME);
        } else {
            PinkCore::RequirementAPI::RegisterInstalled(NoodleExtensions::U8_REQUIREMENTNAME);
        }
    };
}