#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataLoader.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"
#include "custom-json-data/shared/VList.h"
#include "pinkcore/shared/API.hpp"

#include "FakeNoteHelper.h"
#include "SceneTransitionHelper.hpp"
#include "NEHooks.h"
#include "NEJSON.h"

#include "songloader/shared/API.hpp"

#include "sombrero/shared/linq_functional.hpp"

#include "Constants.hpp"


typedef rapidjson::GenericDocument<rapidjson::UTF16<char16_t>> DocumentUTF16;

using namespace System;
using namespace System::Collections::Generic;
using namespace GlobalNamespace;
using namespace CustomJSONData;
using namespace Sombrero::Linq;

struct BeatmapRemoveData {
    int toRemoveObstacle = 0;
    int toRemoveNote = 0;
    int toRemoveBomb = 0;
};

static std::unordered_map<BeatmapData*, BeatmapRemoveData> beatmapRemoveDatas;

// return true if fake
// subtracts from object count if fake
template<typename T>
static bool IsFake(T&& o, bool v2) {
    auto const optData = o->customData;

    if (!optData) return false;

    rapidjson::Value const& customData = *optData;

    auto fake = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_FAKE_NOTE
                                                        : NoodleExtensions::Constants::INTERNAL_FAKE_NOTE);
    return fake.value_or(false);
}

template<typename U, typename T>
auto FakeCount(VList<T> list, bool v2) {
    int i = list.size();
    for (auto o : list) {
        auto note = il2cpp_utils::try_cast<U>(o);
        if (!note) continue;

        if(IsFake(*note, v2)) i--;
    }

    return i;
}

MAKE_HOOK_MATCH(BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveData, &BeatmapDataLoader::GetBeatmapDataBasicInfoFromSaveData, GlobalNamespace::BeatmapDataBasicInfo*,
                BeatmapSaveDataVersion3::BeatmapSaveData* beatmapSaveData) {
    auto ret = BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveData(beatmapSaveData);

    auto customBeatmap = il2cpp_utils::try_cast<CustomBeatmapData>(beatmapSaveData);

    if (!customBeatmap) return ret;

    bool v2 = customBeatmap.value()->v2orEarlier;

    ret->cuttableNotesCount = FakeCount<v3::CustomBeatmapSaveData_ColorNoteData>(VList(beatmapSaveData->colorNotes), v2);
    ret->bombsCount = FakeCount<v3::CustomBeatmapSaveData_BombNoteData>(VList(beatmapSaveData->bombNotes), v2);
    ret->obstaclesCount = FakeCount<v3::CustomBeatmapSaveData_ObstacleData>(VList(beatmapSaveData->obstacles), v2);

    return ret;
}

void InstallBeatmapDataHooks(Logger &logger) {
    // force CJD to be first
    Modloader::requireMod("CustomJSONData");
    INSTALL_HOOK(logger, BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveData)

}
NEInstallHooks(InstallBeatmapDataHooks);