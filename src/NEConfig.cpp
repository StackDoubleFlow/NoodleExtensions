#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "NEConfig.h"
#include "NEHooks.h"

using namespace UnityEngine;
using namespace QuestUI;

DEFINE_CONFIG(NEConfig);

void UIDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        AddConfigValueToggle(container->get_transform(), getNEConfig().enableNoteDissolve);
        AddConfigValueToggle(container->get_transform(), getNEConfig().enableObstacleDissolve);
    }
}

void InstallNEConfigHooks(Logger& logger) {
    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(NELogger::modInfo, UIDidActivate);
}
NEInstallHooks(InstallNEConfigHooks);
