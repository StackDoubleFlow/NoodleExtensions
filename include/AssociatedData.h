#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

struct BeatmapObjectAssociatedData {
    float aheadTime;
    UnityEngine::Vector3 noteOffset;
    UnityEngine::Vector3 moveStartPos;
    UnityEngine::Vector3 moveEndPos;
    UnityEngine::Vector3 jumpEndPos;
    UnityEngine::Quaternion worldRotation;
    UnityEngine::Quaternion localRotation;
};

BeatmapObjectAssociatedData *getAD(CustomJSONData::JSONWrapper *customData);