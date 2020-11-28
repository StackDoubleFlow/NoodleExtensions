#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "GlobalNamespace/SimpleColorSO.hpp"
#include "GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "CustomJSONData/CustomBeatmapData.h"
#include "NoodleExtensions/NEHooks.h"

using namespace GlobalNamespace;

UnityEngine::Quaternion GetWorldRotation(float def, CustomJSONData::CustomObstacleData *obstacleData) {
    UnityEngine::Quaternion worldRotation = UnityEngine::Quaternion::Euler(0, def, 0);
    if (obstacleData->customData) {
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
    if (obstacleData->customData) {
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
    if (obstacleData->customData) {
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

    UnityEngine::Quaternion rotation = GetWorldRotation(worldRotation, obstacleData);
    self->worldRotation = rotation;
    self->inverseWorldRotation = UnityEngine::Quaternion::Euler(-rotation.get_eulerAngles());

    float width = GetCustomWidth(obstacleData->get_width(), obstacleData) * 0.6;// * singleLineWidth;
    UnityEngine::Vector3 b = UnityEngine::Vector3 { (width - singleLineWidth) * 0.5f, 0, 0 };
    self->startPos = startPos + b;
    self->midPos = midPos + b;
    self->endPos = endPos + b;

    float defaultLength = (self->endPos - self->midPos).get_magnitude() / move2Duration * obstacleData->duration;
    float length = GetCustomLength(defaultLength, obstacleData);

    if (!obstacleData->customData) {
        return;
    }
    rapidjson::Value &customData = *obstacleData->customData->value;

    // Chroma thing
    UnityEngine::Color color = self->color->color;
    if (customData.HasMember("_color")) {
        float r = customData["_color"][0].GetFloat();
        float g = customData["_color"][1].GetFloat();
        float b = customData["_color"][2].GetFloat();
        float a = customData["_color"].GetArray().Size() > 3 ? customData["_color"][3].GetFloat() : 1;
        color = UnityEngine::Color(r, g, b, a);
    }

    self->stretchableObstacle->SetSizeAndColor(width * 0.98, height, length, color);
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

MAKE_HOOK_OFFSETLESS(ParametricBoxFakeGlowController_OnEnable, void, Il2CppObject *self) {}

void NoodleExtensions::InstallObstacleControllerHooks() {
    INSTALL_HOOK_OFFSETLESS(ObstacleController_Init, il2cpp_utils::FindMethodUnsafe("", "ObstacleController", "Init", 9));
    // Temporary fake glow disable hook
    INSTALL_HOOK_OFFSETLESS(ParametricBoxFakeGlowController_OnEnable, il2cpp_utils::FindMethodUnsafe("", "ParametricBoxFakeGlowController", "OnEnable", 0));
}