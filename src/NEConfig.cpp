#include "questui/shared/QuestUI.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "NEConfig.h"
#include "NEHooks.h"

#include "UnityEngine/UI/LayoutElement.hpp"

using namespace UnityEngine;
using namespace QuestUI;

DEFINE_CONFIG(NEConfig);

void UIDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        auto* scroll = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        auto container = scroll->GetComponent<UI::HorizontalOrVerticalLayoutGroup*>();
//        auto horizontal = BeatSaberUI::CreateHorizontalLayoutGroup(scroll->get_transform());
//        auto container = BeatSaberUI::CreateVerticalLayoutGroup(horizontal->get_transform());

        // Align and size children properly
        container->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);

        container->set_childControlHeight(true);
        container->set_childForceExpandHeight(true);
        container->set_childControlWidth(true);
        container->set_childForceExpandWidth(true);

        auto *layoutelem = scroll->AddComponent<UI::LayoutElement *>();
        layoutelem->set_preferredWidth(80);

//        auto horizontal = BeatSaberUI::
//
//        auto container = BeatSaberUI::CreateVerticalLayoutGroup()

        AddConfigValueToggle(container->get_transform(), getNEConfig().enableNoteDissolve);
        AddConfigValueToggle(container->get_transform(), getNEConfig().enableObstacleDissolve);

        auto text = BeatSaberUI::CreateText(container->get_transform(), "Qosmetics models cause an outstanding amount of lag in the current state. Disable this setting if you do not care and will happily jam to UwU notes and rainbow walls");

        text->set_fontSize(text->get_fontSize() * 1.125f);
        text->set_enableWordWrapping(true);

        AddConfigValueToggle(container->get_transform(), getNEConfig().qosmeticsModelDisable);
    }
}

void InstallNEConfigHooks(Logger& logger) {
    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(NELogger::modInfo, UIDidActivate);
}
NEInstallHooks(InstallNEConfigHooks);
