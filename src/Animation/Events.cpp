#include "custom-json-data/shared/CustomEventData.h"

#include "Animation/Easings.h"
#include "Animation/Events.h"
#include "Animation/Easings.h"
#include "Animation/Track.h"
#include "NELogger.h"

using namespace NoodleExtensions::Animation;

struct AnimateTrackContext {
    // PointDefinition points;
    Property property;
    float duration;
    float startTime;
    Functions easing;
};

void CustomEventCallback(CustomJSONData::CustomEventData *customEventData) {
    if (customEventData->type == "AnimateTrack") {

    }
}

void NoodleExtensions::Animation::AddEventCallbacks() {
    CustomJSONData::CustomEventCallbacks::AddCustomEventCallback(&CustomEventCallback);
}

