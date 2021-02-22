#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "GlobalNamespace/SimpleColorSO.hpp"
#include "GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "GlobalNamespace/ParametricBoxFrameController.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;

UnityEngine::Quaternion GetWorldRotation(float def, CustomJSONData::CustomObstacleData *obstacleData) {
    UnityEngine::Quaternion worldRotation = UnityEngine::Quaternion::Euler(0, def, 0);
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        if (customData.HasMember("_rotation")) {
            if (customData["_rotation"].IsArray()) {
                float x = customData["_rotation"][0].GetFloat();
                float y = customData["_rotation"][1].GetFloat();
                float z = customData["_rotation"][2].GetFloat();
                worldRotation = UnityEngine::Quaternion::Euler(x, y, z);
            } else {
                worldRotation = UnityEngine::Quaternion::Euler(0, customData["_rotation"].GetFloat(), 0);
            }
        }
    }
    return worldRotation;
}

float GetCustomWidth(float def, CustomJSONData::CustomObstacleData *obstacleData) {
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        std::optional<rapidjson::Value*> scale = customData.HasMember("_scale") ? std::optional{&customData["_scale"]} : std::nullopt;
        std::optional<float> width = scale.has_value() ? std::optional{(*scale.value())[0].GetFloat()} : std::nullopt;
        if (width.has_value()) {
            return width.value();
        }
    }
    return def;
}

float GetCustomLength(float def, CustomJSONData::CustomObstacleData *obstacleData) {
    if (obstacleData->customData->value) {
        rapidjson::Value &customData = *obstacleData->customData->value;
        std::optional<rapidjson::Value*> scale = customData.HasMember("_scale") ? std::optional{&customData["_scale"]} : std::nullopt;
        if (scale.has_value() && scale.value()->Size() > 2) {
            return (*scale.value())[2].GetFloat() * /*NoteLinesDistace*/ 0.6;
        }
    }
    return def;
}

MAKE_HOOK_OFFSETLESS(ObstacleController_Init, void, ObstacleController *self, CustomJSONData::CustomObstacleData *obstacleData, float worldRotation, UnityEngine::Vector3 startPos, UnityEngine::Vector3 midPos, UnityEngine::Vector3 endPos, float move1Duration, float move2Duration, float singleLineWidth, float height) {
    ObstacleController_Init(self, obstacleData, worldRotation, startPos, midPos, endPos, move1Duration, move2Duration, singleLineWidth, height);

    if (!obstacleData->customData->value) {
        return;
    }

    UnityEngine::Quaternion rotation = GetWorldRotation(worldRotation, obstacleData);
    self->worldRotation = rotation;
    self->inverseWorldRotation = UnityEngine::Quaternion::Euler(-rotation.get_eulerAngles());

    float width = GetCustomWidth(obstacleData->width, obstacleData) * 0.6;// * singleLineWidth;
    UnityEngine::Vector3 b = UnityEngine::Vector3 { (width - singleLineWidth) * 0.5f, 0, 0 };
    self->startPos = startPos + b;
    self->midPos = midPos + b;
    self->endPos = endPos + b;

    float defaultLength = (self->endPos - self->midPos).get_magnitude() / move2Duration * obstacleData->duration;
    float length = GetCustomLength(defaultLength, obstacleData);

    rapidjson::Value &customData = *obstacleData->customData->value;

    self->stretchableObstacle->SetSizeAndColor(width * 0.98, height, length, self->color->color);
    self->bounds = self->stretchableObstacle->bounds;

    std::optional<rapidjson::Value*> localrot = customData.HasMember("_localRotation") ? std::optional{&customData["_localRotation"]} : std::nullopt;

    UnityEngine::Transform *transform = self->get_transform();

    UnityEngine::Quaternion localRotation = UnityEngine::Quaternion::get_identity();
    if (localrot.has_value()) {
        localRotation = UnityEngine::Quaternion::Euler((*localrot.value())[0].GetFloat(), (*localrot.value())[1].GetFloat(), (*localrot.value())[2].GetFloat());
        transform->set_localRotation(self->worldRotation * localRotation);
    }

    transform->set_localScale(UnityEngine::Vector3::get_one());

    self->Update();
}

void NoodleExtensions::InstallObstacleControllerHooks(Logger& logger) {
    INSTALL_HOOK_OFFSETLESS(logger, ObstacleController_Init, il2cpp_utils::FindMethodUnsafe("", "ObstacleController", "Init", 9));
}