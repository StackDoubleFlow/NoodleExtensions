#pragma once

#include "custom-json-data/shared/JSONWrapper.h"

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"

#include "Animation/Track.h"

namespace GlobalNamespace {
    class CutoutAnimateEffect;
    class CutoutEffect;
    template<typename T>
    class DisappearingArrowControllerBase_1;
    class GameNoteController;
    class MirroredCubeNoteController;
    class ConditionalMaterialSwitcher;
}

namespace UnityEngine {
    class Renderer;
}

class BeatmapAssociatedData {
public:
    std::unordered_map<std::string, Track> tracks;
    std::unordered_map<std::string, PointDefinition> pointDefinitions;
    std::vector<PointDefinition*> anonPointDefinitions;

    ~BeatmapAssociatedData();
};

struct AnimationObjectData {
    PointDefinition *position;
    PointDefinition *rotation;
    PointDefinition *scale;
    PointDefinition *localRotation;
    PointDefinition *dissolve;
    PointDefinition *dissolveArrow;
    PointDefinition *cuttable;
    PointDefinition *definitePosition;

    AnimationObjectData() {}
    AnimationObjectData(BeatmapAssociatedData& beatmapAD, const rapidjson::Value& customData);
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
    // cutout for obstacles
    GlobalNamespace::CutoutAnimateEffect *cutoutAnimationEffect;
    GlobalNamespace::CutoutAnimateEffect *mirroredCutoutAnimationEffect;
    // cutout for notes
    GlobalNamespace::CutoutEffect *cutoutEffect;
    GlobalNamespace::CutoutEffect *mirroredCutoutEffect;
    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController *> *disappearingArrowController;
    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredCubeNoteController *> *mirroredDisappearingArrowController;
    // conditional material switch for dissolve
    GlobalNamespace::ConditionalMaterialSwitcher *materialSwitcher;
    UnityEngine::Renderer *mirroredRenderer;
    AnimationObjectData animationData;
};

// struct ParentTrackAssociatedData {
//     Track *parentTrack;
//     std::vector<Track*> childrenTracks;
// };

// struct PlayerTrackAssociatedData {
//     Track *track;
// };

BeatmapObjectAssociatedData& getAD(CustomJSONData::JSONWrapper *customData);
BeatmapAssociatedData& getBeatmapAD(CustomJSONData::JSONWrapper *customData);