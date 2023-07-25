#pragma once

#include "custom-json-data/shared/JSONWrapper.h"
#include "NELogger.h"
#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"

namespace GlobalNamespace {
class IDifficultyBeatmap;
class PlayerSpecificSettings;
} // namespace GlobalNamespace

namespace CustomJSONData {
class CustomBeatmapData;
}

namespace NoodleExtensions {
class SceneTransitionHelper {
public:
  static bool CheckIfInArray(CustomJSONData::ValueUTF16 const& val, std::u16string_view stringToCheck) {
    if (val.IsArray()) {
      for (auto const& element : val.GetArray()) {
        if (element.IsString() && element.GetString() == stringToCheck) return true;
      }
    }

    if (val.IsObject()) {
      for (auto const& element : val.GetObject()) {
        if (element.value.IsString() && element.value.GetString() == stringToCheck) return true;
      }
    }

    return false;
  }

  static void Patch(GlobalNamespace::IDifficultyBeatmap* difficultyBeatmap,
                    CustomJSONData::v3::CustomBeatmapSaveData* customBeatmapDataCustom,
                    GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings);

  static bool CheckIfNoodle(CustomJSONData::ValueUTF16 const& rapidjsonData) {
    auto requirements = rapidjsonData.FindMember(u"_requirements");

    if (requirements != rapidjsonData.MemberEnd()) {
      return CheckIfInArray(requirements->value, REQUIREMENTNAME);
    }

    return false;
  }

  static bool CheckIfME(CustomJSONData::ValueUTF16 const& rapidjsonData) {
    auto requirements = rapidjsonData.FindMember(u"_requirements");

    if (requirements != rapidjsonData.MemberEnd()) {
      return CheckIfInArray(requirements->value, ME_REQUIREMENTNAME);
    }

    return false;
  }
};
} // namespace NoodleExtensions
