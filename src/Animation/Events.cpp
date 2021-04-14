#include "GlobalNamespace/BeatmapObjectCallbackController.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Vector3.hpp"

#include "custom-json-data/shared/CustomEventData.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-types/shared/register.hpp"

#include "Animation/Easings.h"
#include "Animation/Events.h"
#include "Animation/Easings.h"
#include "Animation/Track.h"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "Animation/PlayerTrack.h"
#include "AssociatedData.h"
#include "NELogger.h"

using namespace Events;
using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace UnityEngine;

// BeatmapObjectCallbackController.cpp
extern BeatmapObjectCallbackController *callbackController;

BeatmapObjectSpawnController *spawnController;

std::vector<AnimateTrackContext> coroutines;
std::vector<AssignPathAnimationContext> pathCoroutines;

MAKE_HOOK_OFFSETLESS(BeatmapObjectSpawnController_Start, void, BeatmapObjectSpawnController *self) {
    spawnController = self;
    coroutines.clear();
    BeatmapObjectSpawnController_Start(self);
}

bool UpdateCoroutine(AnimateTrackContext& context) {
    float elapsedTime = callbackController->audioTimeSource->get_songTime() - context.startTime;
    float time = Easings::Interpolate(std::min(elapsedTime / context.duration, 1.0f), context.easing);
    if (!context.property->value.has_value()) {
        context.property->value = { 0 };
    }
    switch (context.property->type) {
    case PropertyType::linear:
        context.property->value->linear = context.points->InterpolateLinear(time);
        break;
    case PropertyType::vector3:
        context.property->value->vector3 = context.points->Interpolate(time);
        break;
    case PropertyType::vector4:
        context.property->value->vector4 = context.points->InterpolateVector4(time);
        break;
    case PropertyType::quaternion:
        context.property->value->quaternion = context.points->InterpolateQuaternion(time);
        break;
    }

    return elapsedTime < context.duration;
}

bool UpdatePathCoroutine(AssignPathAnimationContext& context) {
    float elapsedTime = callbackController->audioTimeSource->get_songTime() - context.startTime;
    context.property->value->time = Easings::Interpolate(std::min(elapsedTime / context.duration, 1.0f), context.easing);

    return elapsedTime < context.duration;
}

void Events::UpdateCoroutines() {
    for (auto it = coroutines.begin(); it != coroutines.end();) {
        if (UpdateCoroutine(*it)) {
            it++;
        } else {
            coroutines.erase(it);
        }
    }

    for (auto it = pathCoroutines.begin(); it != pathCoroutines.end();) {
        if (UpdatePathCoroutine(*it)) {
            it++;
        } else {
            it->property->value->Finish();
            pathCoroutines.erase(it);
        }
    }
}

void CustomEventCallback(CustomJSONData::CustomEventData *customEventData) {
    auto *customBeatmapData = (CustomJSONData::CustomBeatmapData*) callbackController->beatmapData;
    BeatmapAssociatedData *ad = getBeatmapAD(customBeatmapData->customData);
    rapidjson::Value& eventData = *customEventData->data;

    EventType type;
    if (customEventData->type == "AnimateTrack") {
        type = EventType::animateTrack;
    } else if (customEventData->type == "AssignPathAnimation") {
        type = EventType::assignPathAnimation;
    } else if (customEventData->type == "AssignTrackParent") {
        Track *track = &ad->tracks[eventData["_parentTrack"].GetString()];

        rapidjson::Value& rawChildrenTracks = eventData["_childrenTracks"];
        std::vector<Track*> childrenTracks;
        for (rapidjson::Value::ConstValueIterator itr = rawChildrenTracks.Begin(); itr != rawChildrenTracks.End(); itr++) {
            childrenTracks.push_back(&ad->tracks[itr->GetString()]);
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

        ParentObject::AssignTrack(childrenTracks, track, startPos, startRot, startLocalRot, startScale);
        return;
    } else if (customEventData->type == "AssignPlayerToTrack") {
        Track *track = &ad->tracks[eventData["_track"].GetString()];
        PlayerTrack::AssignTrack(track);
    } else {
        return;
    }

    Track *track = &ad->tracks[eventData["_track"].GetString()];
    float duration = customEventData->data->HasMember("_duration") ? eventData["_duration"].GetFloat() : 0;
    Functions easing = customEventData->data->HasMember("_easing") ? FunctionFromStr(eventData["_easing"].GetString()) : Functions::easeLinear;

    duration = 60 * duration / spawnController->get_currentBpm();

    auto& properties = track->properties;
    auto& pathProperties = track->pathProperties;

    rapidjson::Value::ConstMemberIterator itr;
    for (itr = eventData.MemberBegin(); itr < eventData.MemberEnd(); itr++) {
        const char *name = (*itr).name.GetString();
        if (strcmp(name, "_track") && strcmp(name, "_duration") && strcmp(name, "_easing")) {
            switch (type) {
            case EventType::animateTrack: {
                Property *property = properties.FindProperty(name);
                if (property) {
                    for (auto it = coroutines.begin(); it != coroutines.end();) {
                        if (it->property == property) {
                            coroutines.erase(it);
                        } else {
                            it++;
                        }
                    }

                    auto *pointData = AnimationHelper::TryGetPointData(ad, eventData, name);
                    if (pointData) {
                        coroutines.push_back(AnimateTrackContext { pointData, property, duration, customEventData->time, easing });
                    }
                } else {
                    NELogger::GetLogger().warning("Could not find track property with name %s", name);
                }
                break;
            }
            case EventType::assignPathAnimation:
                PathProperty *property = pathProperties.FindProperty(name);
                if (property) {
                    for (auto it = pathCoroutines.begin(); it != pathCoroutines.end();) {
                        if (it->property == property) {
                            pathCoroutines.erase(it);
                        } else {
                            it++;
                        }
                    }

                    auto *pointData = AnimationHelper::TryGetPointData(ad, eventData, name);
                    if (pointData) {
                        if (!property->value.has_value()) property->value = PointDefinitionInterpolation();
                        property->value->Init(pointData);
                        pathCoroutines.push_back(AssignPathAnimationContext { property, duration, customEventData->time, easing });
                    }
                } else {
                    NELogger::GetLogger().warning("Could not find track property with name %s", name);
                }
            }
            
            
        }
    }
}

void Events::AddEventCallbacks(Logger& logger) {
    CustomJSONData::CustomEventCallbacks::AddCustomEventCallback(&CustomEventCallback);
    custom_types::Register::RegisterTypes<ParentObject, PlayerTrack>();

    INSTALL_HOOK_OFFSETLESS(logger, BeatmapObjectSpawnController_Start, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectSpawnController", "Start", 0));
}
