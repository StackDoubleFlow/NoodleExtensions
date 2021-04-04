#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/SpawnRotationProcessor.hpp"
#include "GlobalNamespace/BeatmapEventTypeExtensions.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NEHooks.h"

using namespace GlobalNamespace;

MAKE_HOOK_OFFSETLESS(ProcessBeatmapEventData, bool, SpawnRotationProcessor *self, CustomJSONData::CustomBeatmapEventData *beatmapEventData) {
    if (BeatmapEventTypeExtensions::IsRotationEvent(beatmapEventData->type)) {
        if (!beatmapEventData->customData->value) {
            return ProcessBeatmapEventData(self, beatmapEventData);
        }

        rapidjson::Value& customData = *beatmapEventData->customData->value;
        std::optional<float> rotation = customData.HasMember("_rotation") ? std::optional{customData["_rotation"].GetFloat()} : std::nullopt;

        if (rotation.has_value()) {
            self->rotation = rotation.value();
            return true;
        }
    }
    return ProcessBeatmapEventData(self, beatmapEventData);
}

void NoodleExtensions::InstallSpawnRotationProcessorHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, ProcessBeatmapEventData, il2cpp_utils::FindMethodUnsafe("", "SpawnRotationProcessor", "ProcessBeatmapEventData", 1));
}