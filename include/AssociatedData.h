#pragma once

#include "custom-json-data/shared/JSONWrapper.h"
#include "custom-json-data/shared/CustomEventData.h"

#include "tracks/shared/Vector.h"

#include "tracks/shared/Animation/Track.h"
#include "tracks/shared/AssociatedData.h"

#include "questui/shared/CustomTypes/Components/WeakPtrGO.hpp"

#include "Constants.hpp"

#include "GlobalNamespace/CutoutEffect.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"

namespace GlobalNamespace {
    class CutoutAnimateEffect;
    class GameNoteController;
    class ConditionalMaterialSwitcher;
}

namespace UnityEngine {
    class Renderer;
}

struct AnimationObjectData {
    PointDefinition *position = nullptr;
    PointDefinition *rotation = nullptr;
    PointDefinition *scale = nullptr;
    PointDefinition *localRotation = nullptr;
    PointDefinition *dissolve = nullptr;
    PointDefinition *dissolveArrow = nullptr;
    PointDefinition *cuttable = nullptr;
    PointDefinition *definitePosition = nullptr;
    bool parsed = false;

    AnimationObjectData() = default;
    AnimationObjectData(TracksAD::BeatmapAssociatedData &beatmapAD, const rapidjson::Value &animation,
                        bool v2);
};

struct ObjectCustomData {
    std::optional<float> startX;
    std::optional<float> startY;

    std::optional<NEVector::Quaternion> rotation;
    std::optional<NEVector::Quaternion> localRotation;
    std::optional<float> noteJumpMovementSpeed;
    std::optional<float> noteJumpStartBeatOffset;
    std::optional<bool> fake;
    std::optional<bool> uninteractable;

    // notes
    std::optional<bool> disableNoteGravity;
    bool disableNoteLook;

    // obstacles
    std::optional<std::array<std::optional<float>, 3>> scale;

    ObjectCustomData() = default;
    ObjectCustomData(const rapidjson::Value &customData,
                     CustomJSONData::CustomNoteData *noteData,
                     CustomJSONData::CustomObstacleData *obstacleData, bool v2);
};

struct BeatmapObjectAssociatedData {
    BeatmapObjectAssociatedData() {}
    BeatmapObjectAssociatedData(BeatmapObjectAssociatedData&&) = default;
    BeatmapObjectAssociatedData(BeatmapObjectAssociatedData const&) = default;

    // Set in NotesInTimeRowProcessor.ProcessAllNotesInTimeRow
    std::optional<float> startNoteLineLayer;

    float* aheadTime;
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
    explicit ParentTrackEventData(const rapidjson::Value &eventData, TracksAD::BeatmapAssociatedData &beatmapAD,
                                  bool v2);

    Track* parentTrack;
    std::optional<NEVector::Vector3> pos;
    std::optional<NEVector::Quaternion> rot;
    std::optional<NEVector::Quaternion> localRot;
    std::optional<NEVector::Vector3> scale;
    bool worldPositionStays;
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

constexpr BeatmapObjectAssociatedData& getAD(CustomJSONData::JSONWrapper *customData) {
    std::any &ad = customData->associatedData['N'];
    if (!ad.has_value())
        ad = std::make_any<::BeatmapObjectAssociatedData>();
    return std::any_cast<::BeatmapObjectAssociatedData &>(ad);
}