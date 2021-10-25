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

    AnimationObjectData() = default;
    AnimationObjectData(TracksAD::BeatmapAssociatedData& beatmapAD, const rapidjson::Value& customData);
};

struct ObjectCustomData {
    std::optional<NEVector::Vector2> position;
    std::optional<NEVector::Quaternion> rotation;
    std::optional<NEVector::Quaternion> localRotation;
    std::optional<float> noteJumpMovementSpeed;
    std::optional<float> noteJumpStartBeatOffset;
    std::optional<bool> fake;
    std::optional<bool> interactable;

    // notes
    std::optional<NEVector::Quaternion> cutDirection;
    std::optional<NEVector::Vector2> flip;
    std::optional<bool> disableNoteGravity;
    std::optional<bool> disableNoteLook;

    // obstacles
    std::optional<std::array<std::optional<float>, 3>> scale;

    ObjectCustomData() {}
    ObjectCustomData(const rapidjson::Value& customData);
};

struct BeatmapObjectAssociatedData {
    // Set in NotesInTimeRowProcessor.ProcessAllNotesInTimeRow
    std::optional<float> startNoteLineLayer;

    float aheadTime;
    NEVector::Quaternion worldRotation;
    NEVector::Quaternion localRotation;
    NEVector::Vector3 moveStartPos;
    NEVector::Vector3 moveEndPos;
    NEVector::Vector3 jumpEndPos;
    NEVector::Vector3 noteOffset;
    float endRotation;

    float xOffset;
    // set to true is the dissolve material is currently in use
    bool dissolveEnabled;
    // cutout for obstacles
    GlobalNamespace::CutoutAnimateEffect *cutoutAnimationEffect;
    GlobalNamespace::CutoutAnimateEffect *mirroredCutoutAnimationEffect;
    // cutout for notes
    GlobalNamespace::CutoutEffect *cutoutEffect;
    GlobalNamespace::CutoutEffect *mirroredCutoutEffect;
    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController *> *disappearingArrowController;
    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredCubeNoteController *> *mirroredDisappearingArrowController;
    // conditional material switch for dissolve
    Array<GlobalNamespace::ConditionalMaterialSwitcher *> *materialSwitchers;
    AnimationObjectData animationData;
    ObjectCustomData objectData;

    // hide for obstacles
    bool doUnhide;

    void ResetState();
};

BeatmapObjectAssociatedData& getAD(CustomJSONData::JSONWrapper *customData);