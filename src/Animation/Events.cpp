#include "Animation/Events.h"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapObjectCallbackController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Vector3.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/CustomEventData.h"
#include "custom-types/shared/register.hpp"

#include "Animation/ParentObject.h"
#include "Animation/PlayerTrack.h"
#include "AssociatedData.h"
#include "NELogger.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "tracks/shared/Vector.h"

using namespace Events;
using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace NEVector;

BeatmapObjectSpawnController *spawnController;

MAKE_HOOK_MATCH(BeatmapObjectSpawnController_Start, &BeatmapObjectSpawnController::Start, void,
                BeatmapObjectSpawnController *self) {
    spawnController = self;
    BeatmapObjectSpawnController_Start(self);
}

void LoadNoodleEvent(TracksAD::BeatmapAssociatedData &beatmapAD, CustomJSONData::CustomEventData const* customEventData);

void CustomEventCallback(BeatmapObjectCallbackController *callbackController,
                         CustomJSONData::CustomEventData *customEventData) {
    bool isType = false;

    static std::hash<std::string_view> stringViewHash;
    auto typeHash = stringViewHash(customEventData->type);

#define TYPE_GET(jsonName, varName)                                \
    static auto jsonNameHash_##varName = stringViewHash(jsonName); \
    if (!isType && typeHash == (jsonNameHash_##varName))                      \
        isType = true;

    TYPE_GET("AssignTrackParent", AssignTrackParent)
    TYPE_GET("AssignPlayerToTrack", AssignPlayerToTrack)

    if (!isType) {
        return;
    }

    auto const &ad = getEventAD(customEventData);

    // fail safe, idek why this needs to be done smh
    // CJD you bugger
    if (!ad.parsed) {
        auto *customBeatmapData = (CustomJSONData::CustomBeatmapData *)callbackController->beatmapData;
        TracksAD::BeatmapAssociatedData &beatmapAD = TracksAD::getBeatmapAD(customBeatmapData->customData);
        LoadNoodleEvent(beatmapAD, customEventData);
    }

    if (ad.parentTrackEventData) {
        auto const& parentTrackData = *ad.parentTrackEventData;
        ParentObject::AssignTrack(parentTrackData.childrenTracks, parentTrackData.parentTrack, parentTrackData.pos,
                                  parentTrackData.rot, parentTrackData.localRot,
                                  parentTrackData.scale, parentTrackData.worldPositionStays);
    } else if (ad.playerTrackEventData) {
        PlayerTrack::AssignTrack(ad.playerTrackEventData->track);
    }
}

void NEEvents::AddEventCallbacks(Logger &logger) {
    CustomJSONData::CustomEventCallbacks::AddCustomEventCallback(&CustomEventCallback);
    custom_types::Register::AutoRegister();

    INSTALL_HOOK(logger, BeatmapObjectSpawnController_Start);
}
