#include "Animation/Events.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "NELogger.h"
#include "Zenject/DiContainer.hpp"
#include "songcore/shared/SongCore.hpp"

float NECaches::noteJumpMovementSpeed;
float NECaches::noteJumpStartBeatOffset;
float NECaches::numberOfLines;
float NECaches::beatsPerMinute;
float NECaches::noteJumpValue;
GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType NECaches::noteJumpValueType;
bool NECaches::LeftHandedMode;
SafePtr<Zenject::DiContainer> NECaches::GameplayCoreContainer;

void InstallAndRegisterAll() {
  Hooks::InstallHooks();
  NEEvents::AddEventCallbacks();
  SongCore::API::Capabilities::RegisterCapability(NoodleExtensions::U8_REQUIREMENTNAME);

  // Modloader::requireMod("MappingExtensions");
  //     if (!Modloader::getMods().contains("MappingExtensions")) return;

  SongCore::API::LevelSelect::GetLevelWasSelectedEvent() +=
      [](SongCore::API::LevelSelect::LevelWasSelectedEventArgs const& event) {
        if (!event.isCustom) return;

        if (!event.customLevelDetails) return;
        auto const& requirements = event.customLevelDetails->difficultyDetails.requirements;

        bool meRequirement = std::any_of(requirements.begin(), requirements.end(),
                                         [](auto const& s) { return s == NoodleExtensions::U8_ME_REQUIREMENTNAME; });
        bool neRequirement = std::any_of(requirements.begin(), requirements.end(),
                                         [](auto const& s) { return s == NoodleExtensions::U8_REQUIREMENTNAME; });

        for (auto const& r : requirements) {
          NELogger::Logger.debug("Installed on map {}", r.c_str());
        }

        if (meRequirement && neRequirement) {
          NELogger::Logger.debug("Remove install");
          SongCore::API::PlayButton::DisablePlayButton(NoodleExtensions::U8_REQUIREMENTNAME,
                                                       "Map uses both NE and ME. This is not supported!");

        } else {
          NELogger::Logger.debug("Register install");
          SongCore::API::PlayButton::EnablePlayButton(NoodleExtensions::U8_REQUIREMENTNAME);
        }
      };
}
