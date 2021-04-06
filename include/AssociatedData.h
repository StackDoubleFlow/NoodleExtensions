#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

#include "Animation/Track.h"

struct BeatmapObjectAssociatedData {
    float aheadTime;
    Track *track;
    UnityEngine::Quaternion worldRotation;
    UnityEngine::Quaternion localRotation;
    UnityEngine::Vector3 moveStartPos;
    UnityEngine::Vector3 moveEndPos;
    UnityEngine::Vector3 jumpEndPos;
    // Note stuff
    UnityEngine::Vector3 noteOffset;
};

struct BeatmapAssociatedData {
    std::unordered_map<std::string, Track> tracks;
    std::unordered_map<std::string, PointDefinition> pointDefinitions;
};

BeatmapObjectAssociatedData *getAD(CustomJSONData::JSONWrapper *customData);
BeatmapAssociatedData *getBeatmapAD(CustomJSONData::JSONWrapper *customData);