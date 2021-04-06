#include "GlobalNamespace/BeatmapObjectCallbackController.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/IAudioTimeSource.hpp"

#include "custom-json-data/shared/CustomEventData.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include "Animation/Easings.h"
#include "Animation/Events.h"
#include "Animation/Easings.h"
#include "Animation/Track.h"
#include "Animation/AnimationHelper.h"
#include "AssociatedData.h"
#include "NELogger.h"

using namespace Events;
using namespace GlobalNamespace;

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
    EventType type;
    if (customEventData->type == "AnimateTrack") {
        type = EventType::animateTrack;
    } else if (customEventData->type == "AssignPathAnimation") {
        type = EventType::assignPathAnimation;
    } else {
        return;
    }

    auto *customBeatmapData = (CustomJSONData::CustomBeatmapData*) callbackController->beatmapData;
    BeatmapAssociatedData *ad = getBeatmapAD(customBeatmapData->customData);

    rapidjson::Value& eventData = *customEventData->data;

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

                    auto *pointData = AnimationHelper::TryGetPointData(eventData, name);
                    if (pointData) {
                        coroutines.push_back(AnimateTrackContext { pointData, property, duration, customEventData->time, easing });
                    }
                } else {
                    NELogger::GetLogger().warning("Could not find track property with name %s", name);
                }
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

                    auto *pointData = AnimationHelper::TryGetPointData(eventData, name);
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

    INSTALL_HOOK_OFFSETLESS(logger, BeatmapObjectSpawnController_Start, il2cpp_utils::FindMethodUnsafe("", "BeatmapObjectSpawnController", "Start", 0));
}

