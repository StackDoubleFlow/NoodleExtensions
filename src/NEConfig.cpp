#include "NEConfig.h"
#include "NEHooks.h"

#include "UnityEngine/UI/LayoutElement.hpp"
#include "HMUI/ViewController.hpp"

#include "bsml/shared/BSML.hpp"
#include "bsml/shared/BSML-Lite.hpp"

#include <algorithm>

using namespace UnityEngine;

void UIDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
  if (firstActivation) {
    auto* scroll = BSML::Lite::CreateScrollableSettingsContainer(self->get_transform());

    auto container = scroll->GetComponent<UI::HorizontalOrVerticalLayoutGroup*>();
    auto horizontal = BSML::Lite::CreateHorizontalLayoutGroup(scroll->get_transform());

    // Align and size children properly
    container->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);

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

    std::vector<std::string_view> values_view;
    values_view.reserve(values.size());

    for (const auto& str : values) {
        values_view.push_back(str);
    }

    // Create a span from the vector of string_views
    std::span<std::string_view> span_view(values_view);

    auto defaultVal = std::clamp<int>(getNEConfig().materialBehaviour.GetValue(), 0, values.size() - 1);

    auto dropdown =
        BSML::Lite::CreateDropdown(container->get_transform(), getNEConfig().materialBehaviour.GetName(),
                                             values[defaultVal], span_view, [values](StringW val) {
                                               auto index =
                                                   values.begin() - std::find(values.begin(), values.end(), val);

                                               getNEConfig().materialBehaviour.SetValue(std::abs(index));
                                             });

    BSML::Lite::AddHoverHint(
        dropdown->get_transform(),
        "Smart: Changes obstacle material based on Chroma coloring to simulate distortion opacity."
        "\nSemiBasic: switches material opaque/transparent if dissolve is applied."
        "\nBasic: retains opaque material color once dissolve is applied");

    auto text =
        BSML::Lite::CreateText(container->get_transform(),
                                "Qosmetics models cause an outstanding amount of lag in the current state. Disable "
                                "this setting if you do not care and will happily jam to UwU notes and rainbow walls");

    text->set_fontSize(text->get_fontSize() * 1.125f);
    text->set_enableWordWrapping(true);

    AddConfigValueToggle(container->get_transform(), getNEConfig().qosmeticsModelDisable);
  }
}

void InstallNEConfigHooks() {
  BSML::Register::RegisterSettingsMenu("Noodle Extensions", UIDidActivate, false);
}
NEInstallHooks(InstallNEConfigHooks);
