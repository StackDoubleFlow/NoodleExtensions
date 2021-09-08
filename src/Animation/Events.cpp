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

void CustomEventCallback(BeatmapObjectCallbackController *callbackController,
                         CustomJSONData::CustomEventData *customEventData) {
    auto *customBeatmapData = (CustomJSONData::CustomBeatmapData *)callbackController->beatmapData;
    TracksAD::BeatmapAssociatedData &ad = TracksAD::getBeatmapAD(customBeatmapData->customData);
    rapidjson::Value &eventData = *customEventData->data;

    if (customEventData->type == "AssignTrackParent") {
        Track *track = &ad.tracks[eventData["_parentTrack"].GetString()];

        rapidjson::Value &rawChildrenTracks = eventData["_childrenTracks"];
        std::vector<Track *> childrenTracks;
        for (rapidjson::Value::ConstValueIterator itr = rawChildrenTracks.Begin();
             itr != rawChildrenTracks.End(); itr++) {
            Track *child = &ad.tracks[itr->GetString()];
            // NELogger::GetLogger().debug("Assigning track %s(%p) to parent track %s(%p)", itr->GetString(), child, eventData["_parentTrack"].GetString(), track);
            childrenTracks.push_back(child);
        }

        std::optional<Vector3> startPos;
        std::optional<Quaternion> startRot;
        std::optional<Quaternion> startLocalRot;
        std::optional<Vector3> startScale;

        if (eventData.HasMember("_position")) {
            float x = eventData["_position"][0].GetFloat();
            float y = eventData["_position"][1].GetFloat();
            float z = eventData["_position"][2].GetFloat();
            startPos = Vector3(x, y, z);
        }

        if (eventData.HasMember("_rotation")) {
            float x = eventData["_rotation"][0].GetFloat();
            float y = eventData["_rotation"][1].GetFloat();
            float z = eventData["_rotation"][2].GetFloat();
            startRot = Quaternion::Euler(x, y, z);
        }

        if (eventData.HasMember("_localRotation")) {
            float x = eventData["_localRotation"][0].GetFloat();
            float y = eventData["_localRotation"][1].GetFloat();
            float z = eventData["_localRotation"][2].GetFloat();
            startLocalRot = Quaternion::Euler(x, y, z);
        }

        if (eventData.HasMember("_scale")) {
            float x = eventData["_scale"][0].GetFloat();
            float y = eventData["_scale"][1].GetFloat();
            float z = eventData["_scale"][2].GetFloat();
            startScale = Vector3(x, y, z);
        }

        ParentObject::AssignTrack(childrenTracks, track, startPos, startRot, startLocalRot,
                                  startScale);
    } else if (customEventData->type == "AssignPlayerToTrack") {
        Track *track = &ad.tracks[eventData["_track"].GetString()];
        NELogger::GetLogger().debug("Assigning player to track %s at %p",
                                    eventData["_track"].GetString(), track);
        PlayerTrack::AssignTrack(track);
    }
}

void NEEvents::AddEventCallbacks(Logger &logger) {
    CustomJSONData::CustomEventCallbacks::AddCustomEventCallback(&CustomEventCallback);
    custom_types::Register::AutoRegister();

    INSTALL_HOOK(logger, BeatmapObjectSpawnController_Start);
}
