#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "UnityEngine/Transform.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/AnimationHelper.h"
#include "AssociatedData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_OFFSETLESS(NoteController_Init, void, NoteController *self, CustomJSONData::CustomNoteData *noteData, float worldRotation, Vector3 startPos, Vector3 midPos, Vector3 endPos, float move1Duration, float move2Duration, float jumpGravity, float endRotation, float uniformScale) {
    NoteController_Init(self, noteData, worldRotation, startPos, midPos, endPos, move1Duration, move2Duration, jumpGravity, endRotation, uniformScale);
    
    if (!noteData->customData->value) {
        return;
    }
    rapidjson::Value &customData = *noteData->customData->value;
    BeatmapObjectAssociatedData *ad = getAD(noteData->customData);

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;
    
    std::optional<float> curDir = customData.HasMember("_cutDirection") ? std::optional{customData["_cutDirection"].GetFloat()} : std::nullopt;

    if (curDir.has_value()) {
        Quaternion cutQuaternion = Quaternion::Euler(0, 0, curDir.value());
        noteJump->endRotation = cutQuaternion;
        Vector3 vector = cutQuaternion.get_eulerAngles();
        vector = vector + noteJump->randomRotations->values[noteJump->randomRotationIdx] * 20;
        Quaternion midrotation = Quaternion::Euler(vector);
        noteJump->middleRotation = midrotation;
    }

    Transform *transform = self->get_transform();

    Quaternion localRotation = Quaternion::get_identity();
    if (customData.HasMember("_rotation") || customData.HasMember("_localRotation")) {
        if (customData.HasMember("_localRotation")) {
            float x = customData["_localRotation"][0].GetFloat();
            float y = customData["_localRotation"][1].GetFloat();
            float z = customData["_localRotation"][2].GetFloat();
            localRotation = Quaternion::Euler(x, y, z);
        }

        Quaternion worldRotationQuatnerion;
        if (customData.HasMember("_rotation")) {
            if (customData["_rotation"].IsArray()) {
                float x = customData["_rotation"][0].GetFloat();
                float y = customData["_rotation"][1].GetFloat();
                float z = customData["_rotation"][2].GetFloat();
                worldRotationQuatnerion = Quaternion::Euler(x, y, z);
            } else {
                worldRotationQuatnerion = Quaternion::Euler(0, customData["_rotation"].GetFloat(), 0);
            }

            Quaternion inverseWorldRotation = Quaternion::Euler(-worldRotationQuatnerion.get_eulerAngles());
            noteJump->worldRotation = worldRotationQuatnerion;
            noteJump->inverseWorldRotation = inverseWorldRotation;
            floorMovement->worldRotation = worldRotationQuatnerion;
            floorMovement->inverseWorldRotation = inverseWorldRotation;

            worldRotationQuatnerion = worldRotationQuatnerion * localRotation;
            transform->set_localRotation(worldRotationQuatnerion);
        } else {
            transform->set_localRotation(localRotation);
        }
    }

    transform->set_localScale(Vector3::get_one()); // This is a fix for animation due to notes being recycled

    ad->moveStartPos = startPos;
    ad->moveEndPos = midPos;
    ad->jumpEndPos = endPos;
    ad->worldRotation = self->get_worldRotation();
    ad->localRotation = localRotation;

    self->Update();
}

MAKE_HOOK_OFFSETLESS(NoteController_Update, void, NoteController *self) {
    auto customNoteData = (CustomJSONData::CustomNoteData *) self->noteData;

    if (!customNoteData->customData->value) {
        NoteController_Update(self);
        return;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;

    // TODO: Cache deserialized animation data
    // if (!customData.HasMember("_animation")) {
    //     NoteController_Update(self);
    //     return;
    // }
    rapidjson::Value &animation = customData["_animation"];

    BeatmapObjectAssociatedData *ad = getAD(customNoteData->customData);

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;

    float songTime = noteJump->audioTimeSyncController->get_songTime();
    float elapsedTime = songTime - (noteJump->beatTime - (noteJump->jumpDuration * 0.5));
    float normalTime = elapsedTime / noteJump->jumpDuration;

    AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(animation, ad->track, normalTime);

    if (offset.positionOffset.has_value()) {
    //   {"_time":1,"_type":"AnimateTrack","_data":{"_track":"secondLeftStart","_position":[[-30,0,0,1]],"_dissolve":[[0,0]],"_dissolveArrow":[[0,0]]}},
    //   {"_time":1,"_type":"AnimateTrack","_data":{"_track":"secondRightStart","_position":[[30,0,0,1]],"_dissolve":[[0,0]],"_dissolveArrow":[[0,0]]}},
        //     {"_time":17.5,"_lineIndex":2,"_lineLayer":0,"_type":1,"_cutDirection":5,"_customData":{"_track":"secondRightStart","_noteJumpStartBeatOffset":2,"_disableSpawnEffect":true,"_flip":[0,0]}}

        floorMovement->startPos = ad->moveStartPos + *offset.positionOffset;
        floorMovement->endPos = ad->moveEndPos + *offset.positionOffset;
        noteJump->startPos = ad->moveEndPos + *offset.positionOffset;
        noteJump->endPos = ad->jumpEndPos + *offset.positionOffset;
    }

    if (offset.scaleOffset.has_value()) {
        self->get_transform()->set_localScale(*offset.scaleOffset);
    }

    if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
        Quaternion worldRotation = ad->worldRotation;
        Quaternion localRotation = ad->localRotation;

        Quaternion worldRotationQuaternion = worldRotation;
        if (offset.rotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
            Quaternion inverseWorldRotation = Quaternion::Euler(-worldRotationQuaternion.get_eulerAngles());
            noteJump->worldRotation = worldRotationQuaternion;
            noteJump->inverseWorldRotation = inverseWorldRotation;
            floorMovement->worldRotation = worldRotationQuaternion;
            floorMovement->inverseWorldRotation = inverseWorldRotation;
        }

        worldRotationQuaternion = worldRotationQuaternion * localRotation;

        if (offset.localRotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.localRotationOffset;
        }

        self->get_transform()->set_localRotation(worldRotationQuaternion);
    }

    NoteController_Update(self);
}

void NoodleExtensions::InstallNoteControllerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, NoteController_Init, il2cpp_utils::FindMethodUnsafe("", "NoteController", "Init", 10));
    INSTALL_HOOK_OFFSETLESS(logger, NoteController_Update, il2cpp_utils::FindMethodUnsafe("", "NoteController", "ManualUpdate", 0));
}