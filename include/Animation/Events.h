#pragma once
#include "NELogger.h"
#include "Animation/PointDefinition.h"
#include "Animation/Track.h"

namespace Events {

void AddEventCallbacks(Logger& logger);
void UpdateCoroutines();

enum class EventType {
    animateTrack,
    assignPathAnimation
};

struct AnimateTrackContext {
    PointDefinition *points;
    Property *property;
    float duration;
    float startTime;
    Functions easing;
};

struct AssignPathAnimationContext {
    PathProperty *property;
    float duration;
    float startTime;
    Functions easing;
    PointDefinitionInterpolation pointDataInterpolation;
};

} // end namespace Events