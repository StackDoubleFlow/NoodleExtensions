#include "NEConfig.h"
#include "NEHooks.h"

#include "UnityEngine/UI/LayoutElement.hpp"
#include "HMUI/ViewController.hpp"

#include <algorithm>

using namespace UnityEngine;

void UIDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
  if (firstActivation) {
    //auto* scroll = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

    //auto container = scroll->GetComponent<UI::HorizontalOrVerticalLayoutGroup*>();
    //        auto horizontal = BeatSaberUI::CreateHorizontalLayoutGroup(scroll->get_transform());
    //        auto container = BeatSaberUI::CreateVerticalLayoutGroup(horizontal->get_transform());

    // Align and size children properly
    /*container->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);

    container->set_childControlHeight(true);
    container->set_childForceExpandHeight(true);
    container->set_childControlWidth(true);
    container->set_childForceExpandWidth(true);

    auto* layoutelem = scroll->AddComponent<UI::LayoutElement*>();
    layoutelem->set_preferredWidth(80);

    //        auto horizontal = BeatSaberUI::
    //
    //        auto container = BeatSaberUI::CreateVerticalLayoutGroup()

    AddConfigValueToggle(container->get_transform(), getNEConfig().enableNoteDissolve);
    AddConfigValueToggle(container->get_transform(), getNEConfig().enableMirrorNoteDissolve);
    AddConfigValueToggle(container->get_transform(), getNEConfig().enableObstacleDissolve);

    auto values = getMaterialBehaviourValues();
    auto defaultVal = std::clamp<int>(getNEConfig().materialBehaviour.GetValue(), 0, values.size() - 1);

    auto dropdown =
        QuestUI::BeatSaberUI::CreateDropdown(container->get_transform(), getNEConfig().materialBehaviour.GetName(),
                                             values[defaultVal], values, [values](StringW val) {
                                               auto index =
                                                   values.begin() - std::find(values.begin(), values.end(), val);

                                               getNEConfig().materialBehaviour.SetValue(std::abs(index));
                                             });

    BeatSaberUI::AddHoverHint(
        dropdown->get_transform(),
        "Smart: Changes obstacle material based on Chroma coloring to simulate distortion opacity."
        "\nSemiBasic: switches material opaque/transparent if dissolve is applied."
        "\nBasic: retains opaque material color once dissolve is applied");

    auto text =
        BeatSaberUI::CreateText(container->get_transform(),
                                "Qosmetics models cause an outstanding amount of lag in the current state. Disable "
                                "this setting if you do not care and will happily jam to UwU notes and rainbow walls");

    text->set_fontSize(text->get_fontSize() * 1.125f);
    text->set_enableWordWrapping(true);

    AddConfigValueToggle(container->get_transform(), getNEConfig().qosmeticsModelDisable);*/
  }
}

void InstallNEConfigHooks(Logger& logger) {
  //QuestUI::Init();
  //QuestUI::Register::RegisterModSettingsViewController(NELogger::modInfo, UIDidActivate);
}
NEInstallHooks(InstallNEConfigHooks);
