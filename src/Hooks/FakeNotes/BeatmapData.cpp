// TODO: Fix with SongCore changes

#include "GlobalNamespace/BeatmapData.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "GlobalNamespace/BeatmapDataBasicInfo.hpp"

#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"

#include "NEHooks.h"
#include "NEJSON.h"
#include "SceneTransitionHelper.hpp"
#include "AssociatedData.h"

#include "BeatmapDataLoaderVersion3/BeatmapDataLoader.hpp"

#include "BeatmapDataLoaderVersion2_6_0AndEarlier/BeatmapDataLoader.hpp"
#include "BeatmapSaveDataVersion2_6_0AndEarlier/BeatmapSaveData.hpp"
#include "BeatmapSaveDataVersion2_6_0AndEarlier/NoteData.hpp"

#include "custom-json-data/shared/CustomBeatmapSaveDatav2.h"
#include "UnityEngine/JsonUtility.hpp"

#include "Constants.hpp"
#include "sombrero/shared/linq_functional.hpp"
using namespace System;
using namespace System::Collections::Generic;
using namespace GlobalNamespace;
using namespace CustomJSONData;
using namespace UnityEngine;

// return true if fake
// subtracts from object count if fake
template <typename T> static bool IsFake(T* o, bool v2) {
  auto const optData = o->customData;

  if (!optData || !optData) return false;

  rapidjson::Value const& customData = *optData;

  auto fake = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_FAKE_NOTE
                                                      : NoodleExtensions::Constants::INTERNAL_FAKE_NOTE);
  return fake.value_or(false);
}

template <typename U, typename T> auto FakeCount(ArrayW<T> list, bool v2) {
  int i = list.size();
  for (auto o : list) {
    auto note = il2cpp_utils::try_cast<U>(o);
    if (!note) continue;

    if (IsFake(*note, v2)) i--;
  }

  return i;
}

MAKE_HOOK_MATCH(V2_BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveDataJson,
                &BeatmapDataLoaderVersion2_6_0AndEarlier::BeatmapDataLoader::GetBeatmapDataBasicInfoFromSaveDataJson, GlobalNamespace::BeatmapDataBasicInfo*,
                StringW beatmapSaveDataJson) {
  if(!beatmapSaveDataJson) return nullptr;

  auto beatmapSaveData = JsonUtility::FromJson<BeatmapSaveDataVersion2_6_0AndEarlier::BeatmapSaveData*>(beatmapSaveDataJson);
  if (beatmapSaveData == nullptr)
  {
    return nullptr;
  }
  ListW<BeatmapSaveDataVersion2_6_0AndEarlier::NoteData*> notes = beatmapSaveData->notes;

  auto notBombs = notes | Sombrero::Linq::Functional::Where([](BeatmapSaveDataVersion2_6_0AndEarlier::NoteData* x){ return x->type != BeatmapSaveDataVersion2_6_0AndEarlier::NoteType::Bomb; }) | Sombrero::Linq::Functional::ToArray();
  auto bombs = notes | Sombrero::Linq::Functional::Where([](BeatmapSaveDataVersion2_6_0AndEarlier::NoteData* x){ return x->type == BeatmapSaveDataVersion2_6_0AndEarlier::NoteType::Bomb; }) | Sombrero::Linq::Functional::ToArray();
  
  int noteCount = FakeCount<v2::CustomBeatmapSaveData_NoteData>(notBombs, true);
  int bombCount = FakeCount<v2::CustomBeatmapSaveData_NoteData>(bombs, true);
  int obstacleCount = FakeCount<v2::CustomBeatmapSaveData_ObstacleData>(beatmapSaveData->obstacles->ToArray(), true);
  return BeatmapDataBasicInfo::New_ctor(4, noteCount, bombCount, obstacleCount);
}

static JSONWrapper* JSONWrapperOrNull(v3::CustomDataOpt const& val) {
  auto* wrapper = JSONWrapper::New_ctor();

  if (!val || !val->get().IsObject()) {
    return wrapper;
  }

  wrapper->value = val;

  return wrapper;
}

MAKE_HOOK_MATCH(V3_BeatmapDataLoader_GetBeatmapDataFromSaveData, 
                &BeatmapDataLoaderVersion3::BeatmapDataLoader::GetBeatmapDataFromSaveData, GlobalNamespace::BeatmapData*, 
                ::BeatmapSaveDataVersion3::BeatmapSaveData *beatmapSaveData, ::BeatmapSaveDataVersion4::LightshowSaveData *defaultLightshowSaveData, 
                ::GlobalNamespace::BeatmapDifficulty beatmapDifficulty, float_t startBpm, bool loadingForDesignatedEnvironment, ::GlobalNamespace::EnvironmentKeywords *environmentKeywords, 
                ::GlobalNamespace::IEnvironmentLightGroups *environmentLightGroups, ::GlobalNamespace::PlayerSpecificSettings *playerSpecificSettings, ::System::Diagnostics::Stopwatch *stopwatch) {
  using namespace CustomJSONData::v3;
  auto beatmap = il2cpp_utils::cast<CustomBeatmapSaveData>(beatmapSaveData);
  if (!beatmap->customData) return V3_BeatmapDataLoader_GetBeatmapDataFromSaveData(beatmapSaveData, defaultLightshowSaveData, beatmapDifficulty, startBpm, loadingForDesignatedEnvironment, environmentKeywords, environmentLightGroups, playerSpecificSettings, stopwatch);;

  rapidjson::Value const& customData = *beatmap->customData;

#define PARSE_ARRAY(key, array, parse)                                                                                 \
  auto key##it = customData.FindMember(#key);                                                                          \
  if (key##it != customData.MemberEnd()) {                                                                             \
    for (auto const& it : key##it->value.GetArray()) {                                                                 \
      auto item = parse(it);                                                                                           \
      auto& ad = getAD(JSONWrapperOrNull(item->customData));                                                                              \
      ad.objectData.fake = true;                                                                                       \
      array->Add(item);                                                                                                \
    }                                                                                                                  \
  }

  PARSE_ARRAY(fakeColorNotes, beatmap->colorNotes, Parser::DeserializeColorNote);
  //PARSE_ARRAY(fakeBombNotes, beatmap->bombNotes, Parser::DeserializeBombNote);
  //PARSE_ARRAY(fakeObstacles, beatmap->obstacles, Parser::DeserializeObstacle);
  //PARSE_ARRAY(fakeBurstSliders, beatmap->burstSliders, Parser::DeserializeBurstSlider);
  //PARSE_ARRAY(fakeSliders, beatmap->sliders, Parser::DeserializeSlider);
  return V3_BeatmapDataLoader_GetBeatmapDataFromSaveData(beatmapSaveData, defaultLightshowSaveData, beatmapDifficulty, startBpm, loadingForDesignatedEnvironment, environmentKeywords, environmentLightGroups, playerSpecificSettings, stopwatch);
}

void HandleFakeV3Objects(v3::CustomBeatmapSaveData*) {

}
void InstallBeatmapDataHooks() {
  // force CJD to be first, is this needed?
  //Modloader::requireMod("CustomJSONData");
  INSTALL_HOOK(NELogger::Logger, V2_BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveDataJson)
  INSTALL_HOOK(NELogger::Logger, V3_BeatmapDataLoader_GetBeatmapDataFromSaveData)

  v3::Parser::ParsedEvent.addCallback(HandleFakeV3Objects);
}

NEInstallHooks(InstallBeatmapDataHooks);