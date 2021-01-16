#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "UnityEngine/Transform.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "AssociatedData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;

MAKE_HOOK_OFFSETLESS(NoteController_Init, void, NoteController *self, CustomJSONData::CustomNoteData *noteData, float worldRotation, UnityEngine::Vector3 startPos, UnityEngine::Vector3 midPos, UnityEngine::Vector3 endPos, float move1Duration, float move2Duration, float jumpGravity, float endRotation) {
    NoteController_Init(self, noteData, worldRotation, startPos, midPos, endPos, move1Duration, move2Duration, jumpGravity, endRotation);
    
    if (!noteData->customData->value) {
        return;
    }
    rapidjson::Value &customData = *noteData->customData->value;

    NoteJump *noteJump = self->noteMovement->jump;
    NoteFloorMovement *floorMovement = self->noteMovement->floorMovement;
    
    std::optional<float> curDir = customData.HasMember("_cutDirection") ? std::optional{customData["_cutDirection"].GetFloat()} : std::nullopt;

    if (curDir.has_value()) {
        UnityEngine::Quaternion cutQuaternion = UnityEngine::Quaternion::Euler(0, 0, curDir.value());
        noteJump->endRotation = cutQuaternion;
        UnityEngine::Vector3 vector = cutQuaternion.get_eulerAngles();
        vector = vector + noteJump->randomRotations->values[noteJump->randomRotationIdx] * 20;
        UnityEngine::Quaternion midrotation = UnityEngine::Quaternion::Euler(vector);
        noteJump->middleRotation = midrotation;
    }

    UnityEngine::Transform *transform = self->get_transform();

    UnityEngine::Quaternion localRotation = UnityEngine::Quaternion::get_identity();
    if (customData.HasMember("_rotation") || customData.HasMember("_localRotation")) {
        if (customData.HasMember("_localRotation")) {
            float x = customData["_localRotation"][0].GetFloat();
            float y = customData["_localRotation"][1].GetFloat();
            float z = customData["_localRotation"][2].GetFloat();
            localRotation = UnityEngine::Quaternion::Euler(x, y, z);
        }

        UnityEngine::Quaternion worldRotationQuatnerion;
        if (customData.HasMember("_rotation")) {
            if (customData["_rotation"].IsArray()) {
                float x = customData["_rotation"][0].GetFloat();
                float y = customData["_rotation"][1].GetFloat();
                float z = customData["_rotation"][2].GetFloat();
                worldRotationQuatnerion = UnityEngine::Quaternion::Euler(x, y, z);
            } else {
                worldRotationQuatnerion = UnityEngine::Quaternion::Euler(0, customData["_rotation"].GetFloat(), 0);
            }

            UnityEngine::Quaternion inverseWorldRotation = UnityEngine::Quaternion::Euler(-worldRotationQuatnerion.get_eulerAngles());
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

    transform->set_localScale(UnityEngine::Vector3::get_one()); // This is a fix for animation due to notes being recycled

    BeatmapObjectAssociatedData *ad = getAD(noteData->customData);
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
        return;
    }
    rapidjson::Value &customData = *customNoteData->customData->value;

    // TODO: Cache deserialized animation data
    if (!customData.HasMember("_animation")) {
        return;
    }
    rapidjson::Value &animation = customData["_animation"];
}

void NoodleExtensions::InstallNoteControllerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, NoteController_Init, il2cpp_utils::FindMethodUnsafe("", "NoteController", "Init", 9));
    // INSTALL_HOOK_OFFSETLESS(NoteController_Update, il2cpp_utils::FindMethodUnsafe("", "NoteController", "Update", 0));
}