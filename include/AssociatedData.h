#pragma once

#include "custom-json-data/shared/JSONWrapper.h"

#include "tracks/shared/Vector.h"

#include "tracks/shared/Animation/Track.h"
#include "tracks/shared/AssociatedData.h"

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
    AnimationObjectData(TracksAD::BeatmapAssociatedData& beatmapAD, const rapidjson::Value& customData);
};

struct BeatmapObjectAssociatedData {
    float aheadTime;
    NEVector::Quaternion worldRotation;
    NEVector::Quaternion localRotation;
    NEVector::Vector3 moveStartPos;
    NEVector::Vector3 moveEndPos;
    NEVector::Vector3 jumpEndPos;
    NEVector::Vector3 noteOffset;
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

    void ResetState();
};

BeatmapObjectAssociatedData& getAD(CustomJSONData::JSONWrapper *customData);