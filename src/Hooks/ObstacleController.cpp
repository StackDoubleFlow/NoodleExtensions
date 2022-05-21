#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "System/Action_1.hpp"
#include "System/Action_2.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleDissolve.hpp"
#include "GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "GlobalNamespace/ParametricBoxFrameController.hpp"
#include "GlobalNamespace/SimpleColorSO.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEConfig.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/AssociatedData.h"
#include "tracks/shared/TimeSourceHelper.h"

#define ID "Noodle"
#include "chroma/shared/ObstacleAPI.hpp"
#undef ID

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

static SafePtr<List<ObstacleController*>> activeObstacles;
SafePtr<List<ObstacleController*>>& getActiveObstacles() {
    if (!activeObstacles)
        activeObstacles.emplace(List<ObstacleController *>::New_ctor());

    return activeObstacles;
}


std::unordered_map<ObstacleController *, ArrayW<ConditionalMaterialSwitcher *>> cachedObstacleMaterialSwitchers;
std::unordered_map<GlobalNamespace::ObstacleControllerBase *, Sombrero::FastColor> cachedObstacleColors;

bool mapLoaded = false;

void OnObstacleChangeColor(GlobalNamespace::ObstacleControllerBase * oc, Sombrero::FastColor const& color) {
    cachedObstacleColors[oc] = color;
}

void NECaches::ClearObstacleCaches() {
    cachedObstacleMaterialSwitchers.clear();
    cachedObstacleColors.clear();
    mapLoaded = false;
}

float obstacleTimeAdjust(float original, std::span<Track*> tracks, float move1Duration, float finishMovementTime) {
    if (original > move1Duration && !tracks.empty()) {
        Track *obstacleTrack = nullptr;

        if (tracks.size() > 1) {
            auto trackIt = std::find_if(tracks.begin(), tracks.end(), [](Track *track) {
                return track->properties.time.value.has_value();
            });

            if (trackIt != tracks.end()) {
                obstacleTrack = *trackIt;
            }
        } else {
            obstacleTrack = tracks.front();
        }

        if (obstacleTrack) {
            Property &timeProperty = obstacleTrack->properties.time;
            if (timeProperty.value) {
                float time = timeProperty.value->linear;
                return (time * (finishMovementTime - move1Duration)) + move1Duration;
            }
        }
    }

    return original;
}

MAKE_HOOK_MATCH(ObstacleController_Init, &ObstacleController::Init, void, ObstacleController *self,
                ObstacleData *normalObstacleData, float worldRotation, Vector3 startPos,
                Vector3 midPos, Vector3 endPos, float move1Duration, float move2Duration,
                float singleLineWidth, float height) {
    if (!Hooks::isNoodleHookEnabled())
        return ObstacleController_Init(self, normalObstacleData, worldRotation, startPos, midPos, endPos,
                                       move1Duration, move2Duration, singleLineWidth, height);
    ObstacleController_Init(self, normalObstacleData, worldRotation, startPos, midPos, endPos,
                            move1Duration, move2Duration, singleLineWidth, height);
    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(normalObstacleData);

    Transform *transform = self->get_transform();
    transform->set_localScale(NEVector::Vector3::one());


    if (!obstacleData->customData) {
        return;
    }

    if (getNEConfig().materialBehaviour.GetValue() == (int) MaterialBehaviour::SMART_COLOR) {
        // lazy initialize since Chroma clears the callbacks on map load and ordering that is not easy
        if (!mapLoaded) {
            static auto callbackOpt = Chroma::ObstacleAPI::getObstacleChangedColorCallbackSafe();
            mapLoaded = true;
            if (callbackOpt) {
                Chroma::ObstacleAPI::ObstacleCallback &callback = *callbackOpt;
                // remove if it already exists
                callback -= &OnObstacleChangeColor;
                callback += &OnObstacleChangeColor;
            }
        }

        // Obstacles are pooled. Clear obstacle when initialized if it's not colored or update to its new color (probably redundantly)
        auto color = Chroma::ObstacleAPI::getObstacleControllerColorSafe(self);
        if (color) {
            cachedObstacleColors[self] = *color;
        } else {
            cachedObstacleColors.erase(self);
        }
    }

    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    ArrayW<ConditionalMaterialSwitcher *> materialSwitchers;
    auto it = cachedObstacleMaterialSwitchers.find(self);
    if (it == cachedObstacleMaterialSwitchers.end()) {
        cachedObstacleMaterialSwitchers[self] = materialSwitchers = self->get_gameObject()->GetComponentsInChildren<ConditionalMaterialSwitcher *>();
    } else {
        materialSwitchers = it->second;
    }
    ad.materialSwitchers = materialSwitchers;

    // Reset only if NE dissolve is enabled
    if (getNEConfig().enableObstacleDissolve.GetValue()) {
        for (auto *materialSwitcher: materialSwitchers) {
            materialSwitcher->renderer->set_sharedMaterial(materialSwitcher->material0);
        }
    }
    ad.dissolveEnabled = false;

    std::function<void()> const setBounds = [&ad, &self](){
        auto const& cuttable = ad.objectData.uninteractable;
        if (cuttable && *cuttable) {
            self->bounds.set_size(NEVector::Vector3::zero());
        } else {
            getActiveObstacles()->Add(self);
        }

        ad.boundsSize = self->bounds.get_size();
    };


    if (!obstacleData->customData->value) {
        setBounds();
        return;
    }

    NEVector::Quaternion rotation;

    static auto Quaternion_Inverse = il2cpp_utils::il2cpp_type_check::FPtrWrapper<static_cast<UnityEngine::Quaternion (*)(UnityEngine::Quaternion)>(&UnityEngine::Quaternion::Inverse)>::get();
    static auto Quaternion_Euler = il2cpp_utils::il2cpp_type_check::FPtrWrapper<static_cast<UnityEngine::Quaternion (*)(float, float, float)>(&UnityEngine::Quaternion::Euler)>::get();

    if (ad.objectData.rotation)
        rotation = *ad.objectData.rotation;
    else
        rotation = Quaternion_Euler(0, worldRotation, 0);

    self->worldRotation = rotation;
    self->inverseWorldRotation = Quaternion_Inverse(rotation);

    auto const& scale = ad.objectData.scale;

    float width = (scale && scale->at(0) ? *scale->at(0) : obstacleData->width) * singleLineWidth;
    NEVector::Vector3 b = NEVector::Vector3((width - singleLineWidth) * 0.5f, 0, 0);
    self->startPos = NEVector::Vector3(startPos) + b;
    self->midPos = NEVector::Vector3(midPos) + b;
    self->endPos = NEVector::Vector3(endPos) + b;
    ad.moveStartPos = self->startPos;
    ad.moveEndPos = self->midPos;
    ad.jumpEndPos = self->endPos;

    float defaultLength =
            NEVector::Vector3(NEVector::Vector3(self->endPos) - NEVector::Vector3(self->midPos)).Magnitude() /
            move2Duration * obstacleData->duration;
    float length = (scale && scale->at(2) ? *scale->at(2) * /*NoteLinesDistace*/ 0.6 : defaultLength);

    self->stretchableObstacle->SetSizeAndColor(width * 0.98, height, length,
                                               self->stretchableObstacle->obstacleFrame->color);
    self->bounds = self->stretchableObstacle->bounds;

    setBounds();

    std::optional<NEVector::Quaternion> const& localrot = ad.objectData.localRotation;

    NEVector::Quaternion localRotation = NEVector::Quaternion::identity();
    if (localrot.has_value()) {
        localRotation = *localrot;
    }
    transform->set_localPosition(startPos);
    transform->set_localRotation(NEVector::Quaternion(self->worldRotation) * localRotation);
    ad.localRotation = localRotation;
    ad.worldRotation = rotation;

    auto const &tracks = TracksAD::getAD(obstacleData->customData).tracks;
    if (!tracks.empty()) {
        auto go = self->get_gameObject();
        for (auto &track: tracks) {
            track->AddGameObject(go);
        }
    }



    self->Update();
}

static void ObstacleController_ManualUpdateTranspile(ObstacleController *self, float const elapsedTime) {
    // TRANSPILE HERE
    float num = elapsedTime;
    // TRANSPILE HERE
    NEVector::Vector3 posForTime = self->GetPosForTime(num);
    self->get_transform()->set_localPosition(NEVector::Quaternion (self->worldRotation) * posForTime);
    auto action = self->didUpdateProgress;
    if (action)
    {
        action->Invoke(self, num);
    }
    if (!self->passedThreeQuartersOfMove2Reported && num > self->move1Duration + self->move2Duration * 0.75f)
    {
        self->passedThreeQuartersOfMove2Reported = true;
        auto action2 = self->passedThreeQuartersOfMove2Event;
        if (action2)
        {
            action2->Invoke(self);
        }
    }
    if (!self->passedAvoidedMarkReported && num > self->passedAvoidedMarkTime)
    {
        self->passedAvoidedMarkReported = true;
        auto action3 = self->passedAvoidedMarkEvent;
        if (action3)
        {
            action3->Invoke(self);
        }
    }
    if (num > self->finishMovementTime)
    {
        auto action4 = self->finishedMovementEvent;
        if (!action4)
        {
            return;
        }
        action4->Invoke(self);
    }
}

MAKE_HOOK_MATCH(ObstacleController_ManualUpdate, &ObstacleController::ManualUpdate, void,
                ObstacleController *self) {
    if (!Hooks::isNoodleHookEnabled())
        return ObstacleController_ManualUpdate(self);
    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(self->obstacleData);

    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    if (ad.doUnhide) {
        self->Hide(false);
    }
    if (!obstacleData->customData->value) {
        ObstacleController_ManualUpdate(self);
        return;
    }

    // TODO: Cache deserialized animation data
    // if (!customData.HasMember("_animation")) {
    //     ObstacleController_Update(self);
    //     return;
    // }



    auto const& tracks = TracksAD::getAD(obstacleData->customData).tracks;

    if (tracks.empty() && !ad.parsed)
    {
        ObstacleController_ManualUpdate(self);
        return;
    }

    float const songTime = TimeSourceHelper::getSongTime(self->audioTimeSyncController);
    float const elapsedTime = songTime - self->startTimeOffset;
    float const normalTime =
        (elapsedTime - self->move1Duration) / (self->move2Duration + self->obstacleDuration);

    AnimationHelper::ObjectOffset offset =
        AnimationHelper::GetObjectOffset(ad.animationData, tracks, normalTime);

    if (offset.positionOffset.has_value()) {
        self->startPos = ad.moveStartPos + *offset.positionOffset;
        self->midPos = ad.moveEndPos + *offset.positionOffset;
        self->endPos = ad.jumpEndPos + *offset.positionOffset;
    }

    Transform *transform = self->get_transform();

    if (offset.scaleOffset.has_value()) {
        transform->set_localScale(*offset.scaleOffset);
    }

    if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
        NEVector::Quaternion worldRotation = ad.worldRotation;
        NEVector::Quaternion localRotation = ad.localRotation;

        NEVector::Quaternion worldRotationQuaternion = worldRotation;
        if (offset.rotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
            NEVector::Quaternion inverseWorldRotation =
                NEVector::Quaternion::Inverse(worldRotationQuaternion);
            self->worldRotation = worldRotationQuaternion;
            self->inverseWorldRotation = inverseWorldRotation;
        }

        worldRotationQuaternion = worldRotationQuaternion * localRotation;

        if (offset.localRotationOffset.has_value()) {
            worldRotationQuaternion = worldRotationQuaternion * *offset.localRotationOffset;
        }

        transform->set_localRotation(worldRotationQuaternion);
    }

    if (offset.cuttable.has_value()) {
        if (*offset.cuttable >= 1.0f) {
            self->bounds.set_size(NEVector::Vector3::zero());
        }
        else {
            self->bounds.set_size(ad.boundsSize);
        }
    }

    bool obstacleDissolveConfig = getNEConfig().enableObstacleDissolve.GetValue();
    if (offset.dissolve.has_value()) {
        float dissolve;

        if (obstacleDissolveConfig) {
            dissolve = 1 - *offset.dissolve;
        } else {
            dissolve = *offset.dissolve >= 0 ? 0 : 1;
        }

        bool wasEnabled = ad.dissolveEnabled;
        ad.dissolveEnabled = obstacleDissolveConfig;

        if (ad.dissolveEnabled) {
            if (getNEConfig().materialBehaviour.GetValue() == (int) MaterialBehaviour::BASIC) {
                ad.dissolveEnabled |= dissolve > 0.0f;
            } else {
                bool transparent = true;

                if (getNEConfig().materialBehaviour.GetValue() == (int) MaterialBehaviour::SMART_COLOR) {
                    auto colorIt = cachedObstacleColors.find(self);

                    if (colorIt != cachedObstacleColors.end()) {
                        auto const &color = colorIt->second;

                        if (color.a <= 0.0f) {
                            transparent = false;
                        }
                    }
                }

                if (transparent)
                    ad.dissolveEnabled = dissolve > 0.0f;
            }
        }

        if (wasEnabled != ad.dissolveEnabled) {
            ArrayW<ConditionalMaterialSwitcher *> materialSwitchers = ad.materialSwitchers;
            for (auto *materialSwitcher: materialSwitchers) {
                materialSwitcher->renderer->set_sharedMaterial(
                        ad.dissolveEnabled ? materialSwitcher->material1 : materialSwitcher->material0);
            }
        }

        CutoutAnimateEffect *cutoutAnimationEffect = ad.cutoutAnimationEffect;
        if (!cutoutAnimationEffect) {
            ObstacleDissolve *obstacleDissolve =
                self->get_gameObject()->GetComponent<ObstacleDissolve *>();
            cutoutAnimationEffect = obstacleDissolve->cutoutAnimateEffect;
            ad.cutoutAnimationEffect = cutoutAnimationEffect;
        }

        cutoutAnimationEffect->SetCutout(dissolve);
    }

    // do transpile only if needed
    float const obstacleTime = obstacleTimeAdjust(elapsedTime, tracks, self->move1Duration, self->finishMovementTime);
    if (obstacleTime != elapsedTime) {
        ObstacleController_ManualUpdateTranspile(self, obstacleTime);
        return;
    }

    ObstacleController_ManualUpdate(self);
}

MAKE_HOOK_MATCH(ObstacleController_GetPosForTime, &ObstacleController::GetPosForTime, Vector3,
                ObstacleController *self, float time) {
    if (!Hooks::isNoodleHookEnabled())
        return ObstacleController_GetPosForTime(self, time);

//    static auto CustomObstacleDataKlass = classof(CustomJSONData::CustomObstacleData *);
//    CRASH_UNLESS(self);
//    CRASH_UNLESS(self->obstacleData);
//    NELogger::GetLogger().debug("ObstacleController::GetPosForTime %p", self->obstacleData);
//    CRASH_UNLESS(self->obstacleData->klass);
//    CRASH_UNLESS(CustomObstacleDataKlass);
//
//    if (!self || !self->obstacleData || self->obstacleData->klass != CustomObstacleDataKlass) {
//        return ObstacleController_GetPosForTime(self, time);
//    }
    auto *obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData *>(self->obstacleData);

    if (!obstacleData->customData->value) {
        return ObstacleController_GetPosForTime(self, time);
    }
    BeatmapObjectAssociatedData &ad = getAD(obstacleData->customData);

    auto const& tracks = TracksAD::getAD(obstacleData->customData).tracks;

    float jumpTime = (time - self->move1Duration) / (self->move2Duration + self->obstacleDuration);
    jumpTime = std::clamp(jumpTime, 0.0f, 1.0f);
    std::optional<NEVector::Vector3> position =
            AnimationHelper::GetDefinitePositionOffset(ad.animationData, tracks, jumpTime);

    if (!position.has_value())
        return ObstacleController_GetPosForTime(self, time);

    NEVector::Vector3 noteOffset = ad.noteOffset;
    NEVector::Vector3 definitePosition = *position + noteOffset;
    definitePosition.x += ad.xOffset;
    if (time < self->move1Duration) {
        NEVector::Vector3 result = NEVector::Vector3::LerpUnclamped(
                self->startPos, self->midPos, time / self->move1Duration);
        return result + (definitePosition - static_cast<NEVector::Vector3>(self->midPos));
    } else {
        return definitePosition;
    }
}

MAKE_HOOK_MATCH(ParametricBoxFakeGlowController_OnEnable,
                &ParametricBoxFakeGlowController::OnEnable, void,
                ParametricBoxFakeGlowController *self) {
    if (Hooks::isNoodleHookEnabled())
        return;

    ParametricBoxFakeGlowController_OnEnable(self);
}

// #include "beatsaber-hook/shared/utils/instruction-parsing.hpp"
// MAKE_HOOK(Object_New, nullptr, Il2CppObject *, Il2CppClass *klass) {
//     if (test && klass && klass->name && klass->namespaze) {
//         NELogger::GetLogger().info("Allocating a %s.%s", klass->namespaze, klass->name);
//         PrintBacktrace(10);
//     }
//     return Object_New(klass);
// }

void InstallObstacleControllerHooks(Logger &logger) {
    INSTALL_HOOK(logger, ObstacleController_Init);
    INSTALL_HOOK(logger, ObstacleController_ManualUpdate);
    INSTALL_HOOK(logger, ObstacleController_GetPosForTime);
    // Temporary fake glow disable hook
    INSTALL_HOOK(logger, ParametricBoxFakeGlowController_OnEnable);

    // Instruction on((const int32_t*) HookTracker::GetOrig(il2cpp_functions::object_new));
    // Instruction j2Ob_N_thunk(CRASH_UNLESS(on.findNthCall(1)->label));
    // logger.info("thunk addr %p %x", j2Ob_N_thunk.addr, *j2Ob_N_thunk.addr);
    // auto *j2Ob_N = CRASH_UNLESS(j2Ob_N_thunk.findNthDirectBranchWithoutLink(1));
    // INSTALL_HOOK_DIRECT(logger, Object_New, (void*) *j2Ob_N->label);
}

NEInstallHooks(InstallObstacleControllerHooks);