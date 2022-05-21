#include "Animation/Events.h"
#include "pinkcore/shared/RequirementAPI.hpp"
#include "pinkcore/shared/API.hpp"
#include "NEHooks.h"
#include "NECaches.h"


float NECaches::noteJumpMovementSpeed;
float NECaches::noteJumpStartBeatOffset;
float NECaches::numberOfLines;
float NECaches::beatsPerMinute;
float NECaches::noteJumpValue;
GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType NECaches::noteJumpValueType;
bool NECaches::LeftHandedMode;

void InstallAndRegisterAll() {
    Logger& logger = NELogger::GetLogger();
    Hooks::InstallHooks(logger);
    NEEvents::AddEventCallbacks(logger);
    PinkCore::RequirementAPI::RegisterInstalled(NoodleExtensions::U8_REQUIREMENTNAME);

    Modloader::requireMod("MappingExtensions");
//    if (!Modloader::getMods().contains("MappingExtensions")) return;

    PinkCore::API::GetFoundRequirementCallbackSafe() += [](const std::vector<std::string>& requirements){
        bool meRequirement = std::any_of(requirements.begin(), requirements.end(), [](auto const& s) {return s == NoodleExtensions::U8_ME_REQUIREMENTNAME; });
        bool neRequirement = std::any_of(requirements.begin(), requirements.end(), [](auto const& s) {return s == NoodleExtensions::U8_ME_REQUIREMENTNAME; });

        for (auto const& r : requirements) {
            NELogger::GetLogger().debug("Installed on map %s", r.c_str());
        }

        if (meRequirement && neRequirement) {
            NELogger::GetLogger().debug("Remove install");
            PinkCore::RequirementAPI::RemoveInstalled(NoodleExtensions::U8_REQUIREMENTNAME);
        } else {
            NELogger::GetLogger().debug("Register install");
            PinkCore::RequirementAPI::RegisterInstalled(NoodleExtensions::U8_REQUIREMENTNAME);
        }
    };
}