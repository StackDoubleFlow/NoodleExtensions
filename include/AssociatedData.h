#pragma once

#include "custom-json-data/shared/JSONWrapper.h"
#include "custom-json-data/shared/CustomEventData.h"

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
    std::optional<bool> disableNoteGravity;
    std::optional<bool> disableNoteLook;

    // obstacles
    std::optional<std::array<std::optional<float>, 3>> scale;

    ObjectCustomData() = default;
    ObjectCustomData(const rapidjson::Value& customData, std::optional<NEVector::Vector2>& flip);
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
    NEVector::Vector3 boundsSize; // obstacles
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

    // flip for notes
    std::optional<NEVector::Vector2> flip;

    // hide for obstacles
    bool doUnhide;

    bool parsed = false;

    void ResetState();
};

struct PlayerTrackEventData {
    explicit PlayerTrackEventData(Track* track) : track(track) {}

    Track* track;
};

struct ParentTrackEventData {
    explicit ParentTrackEventData(const rapidjson::Value& customData, std::vector<Track*>  childrenTracks, std::string_view parentTrackName, Track* parentTrack);

    const std::string parentTrackName;
    Track* parentTrack;
    std::optional<NEVector::Vector3> pos;
    std::optional<NEVector::Quaternion> rot;
    std::optional<NEVector::Quaternion> localRot;
    std::optional<NEVector::Vector3> scale;
    std::vector<Track *> childrenTracks;
};

struct BeatmapEventAssociatedData {
    // union?
    std::optional<PlayerTrackEventData> playerTrackEventData;
    std::optional<ParentTrackEventData> parentTrackEventData;

    bool parsed = false;
};

BeatmapEventAssociatedData& getEventAD(CustomJSONData::CustomEventData const* customData);
void clearEventADs();

BeatmapObjectAssociatedData& getAD(CustomJSONData::JSONWrapper *customData);