#pragma once

#include "custom-json-data/shared/JSONWrapper.h"

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

#include "Animation/Track.h"

namespace GlobalNamespace {
    class CutoutAnimateEffect;
}

struct AnimationObjectData {
    PointDefinition *position;
    PointDefinition *rotation;
    PointDefinition *scale;
    PointDefinition *localRotation;
    PointDefinition *dissolve;
    PointDefinition *definitePosition;
};

struct BeatmapObjectAssociatedData {
    float aheadTime;
    Track *track;
    UnityEngine::Quaternion worldRotation;
    UnityEngine::Quaternion localRotation;
    UnityEngine::Vector3 moveStartPos;
    UnityEngine::Vector3 moveEndPos;
    UnityEngine::Vector3 jumpEndPos;
    UnityEngine::Vector3 noteOffset;
    float xOffset;
    GlobalNamespace::CutoutAnimateEffect *cutoutAnimationEffect;
    AnimationObjectData animationData;
};

class BeatmapAssociatedData {
public:
    std::unordered_map<std::string, Track> tracks;
    std::unordered_map<std::string, PointDefinition> pointDefinitions;
    std::vector<PointDefinition*> anonPointDefinitions;

    ~BeatmapAssociatedData();
};

// struct ParentTrackAssociatedData {
//     Track *parentTrack;
//     std::vector<Track*> childrenTracks;
// };

// struct PlayerTrackAssociatedData {
//     Track *track;
// };

BeatmapObjectAssociatedData *getAD(CustomJSONData::JSONWrapper *customData);
BeatmapAssociatedData *getBeatmapAD(CustomJSONData::JSONWrapper *customData);