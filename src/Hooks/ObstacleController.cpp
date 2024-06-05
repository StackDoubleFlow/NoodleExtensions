#include "NELogger.h"
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
#include "NEUtils.hpp"
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

static SafePtr<System::Collections::Generic::List_1<UnityW<ObstacleController>>> activeObstacles;
SafePtr<System::Collections::Generic::List_1<UnityW<ObstacleController>>>& getActiveObstacles() {
  if (!activeObstacles) activeObstacles = System::Collections::Generic::List_1<UnityW<ObstacleController>>::New_ctor();

  return activeObstacles;
}

bool mapLoaded = false;

void OnObstacleChangeColor(GlobalNamespace::ObstacleControllerBase* oc, Sombrero::FastColor const& color) {
  NECaches::getObstacleCache(oc).color = color;
}

void NECaches::ClearObstacleCaches() {
  NECaches::obstacleCache.clear();
  mapLoaded = false;
}

float obstacleTimeAdjust(ObstacleController* oc, float original, std::span<Track*> tracks) {
  if (original <= oc->move1Duration) return original;

  auto time = NoodleExtensions::getTimeProp(tracks);
  if (!time) return original;

  return (time.value() * (oc->_finishMovementTime - oc->move1Duration)) + oc->move1Duration;
}

Quaternion GetWorldRotation(CustomJSONData::CustomObstacleData* obstacleData, float def,
                            BeatmapObjectAssociatedData& ad) {
  auto worldRotation = NEVector::Quaternion::Euler({ 0, def, 0 });

  if (!obstacleData->customData) {
    return worldRotation;
  }

  auto const& worldRotationQuaternion = ad.objectData.rotation;
  if (worldRotationQuaternion.has_value()) {
    worldRotation = worldRotationQuaternion.value();
  }

  ad.worldRotation = worldRotation;

  return worldRotation;
}

constexpr float GetCustomWidth(float def, BeatmapObjectAssociatedData const& ad) {
  if (!ad.objectData.scale) return def;
  return ad.objectData.scale.value()[0].value_or(def);
}

constexpr float GetCustomLength(float def, BeatmapObjectAssociatedData const& ad) {
  if (!ad.objectData.scale) return def;
  auto opt = ad.objectData.scale.value()[2];

  if (!opt) return def;

  return *opt * NECaches::get_noteLinesDistanceFast();
}

static void ObstacleInitTranspileLite(ObstacleController* self, CustomJSONData::CustomObstacleData* obstacleData,
                                      BeatmapObjectAssociatedData& ad, float worldRotation, Vector3 startPos,
                                      Vector3 midPos, Vector3 endPos, float move1Duration, float move2Duration,
                                      float singleLineWidth, float height) {
  static auto Quaternion_Inverse =
      il2cpp_utils::il2cpp_type_check::FPtrWrapper<static_cast<UnityEngine::Quaternion (*)(UnityEngine::Quaternion)>(
          &UnityEngine::Quaternion::Inverse)>::get();
  static auto Quaternion_Euler =
      il2cpp_utils::il2cpp_type_check::FPtrWrapper<static_cast<UnityEngine::Quaternion (*)(float, float, float)>(
          &UnityEngine::Quaternion::Euler)>::get();

  Transform* transform = self->get_transform();

  // Now to do the transpiler stuff at the end
  self->_worldRotation = GetWorldRotation(obstacleData, worldRotation, ad);
  self->_inverseWorldRotation = Quaternion_Inverse(self->_worldRotation);

  // set transform
  transform->localRotation = self->_worldRotation;

  self->_width = GetCustomWidth((float)obstacleData->width * singleLineWidth, ad);

  // fixup position
  auto b = NEVector::Vector3((self->width - singleLineWidth) * 0.5f, 0, 0);
  self->_startPos = NEVector::Vector3(startPos) + b;
  self->_midPos = NEVector::Vector3(midPos) + b;
  self->_endPos = NEVector::Vector3(endPos) + b;



  float num = (NEVector::Vector3(self->_endPos) - self->_midPos).Magnitude() / self->move2Duration;
  self->_length = GetCustomLength(num * obstacleData->duration, ad);

  // fix bounds
  self->_stretchableObstacle->SetSizeAndColor(self->_width * 0.98f, self->_height, self->_length,
                                              self->_stretchableObstacle->_obstacleFrame->color);
  self->_bounds = self->_stretchableObstacle->bounds;
}

MAKE_HOOK_MATCH(ObstacleController_Init, &ObstacleController::Init, void, ObstacleController* self,
                ObstacleData* normalObstacleData, float worldRotation, Vector3 startPos, Vector3 midPos, Vector3 endPos,
                float move1Duration, float move2Duration, float singleLineWidth, float height) {
  if (!Hooks::isNoodleHookEnabled())
    return ObstacleController_Init(self, normalObstacleData, worldRotation, startPos, midPos, endPos, move1Duration,
                                   move2Duration, singleLineWidth, height);
  ObstacleController_Init(self, normalObstacleData, worldRotation, startPos, midPos, endPos, move1Duration,
                          move2Duration, singleLineWidth, height);

  static auto CustomKlass = classof(CustomJSONData::CustomObstacleData*);

  if (normalObstacleData->klass != CustomKlass) return;

  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(normalObstacleData);

  Transform* transform = self->get_transform();
  transform->set_localScale(NEVector::Vector3::one());

  if (!obstacleData->customData) {
    return;
  }

  auto& obstacleCache = NECaches::getObstacleCache(self);

  if (getNEConfig().materialBehaviour.GetValue() == (int)MaterialBehaviour::SMART_COLOR) {
    // lazy initialize since Chroma clears the callbacks on map load and ordering that is not easy
    if (!mapLoaded) {
      static auto callbackOpt = Chroma::ObstacleAPI::getObstacleChangedColorCallbackSafe();
      mapLoaded = true;
      if (callbackOpt) {
        Chroma::ObstacleAPI::ObstacleCallback& callback = *callbackOpt;
        // remove if it already exists
        callback -= &OnObstacleChangeColor;
        callback += &OnObstacleChangeColor;
      }
    }
  }

  BeatmapObjectAssociatedData& ad = getAD(obstacleData->customData);

  if (!ad.parsed) {
    NELogger::Logger.warn("Obstacle at time {} was not parsed", obstacleData->time);
    return;
  }

  ArrayW<ConditionalMaterialSwitcher*>& materialSwitchers = obstacleCache.conditionalMaterialSwitchers;
  if (!materialSwitchers) {
    materialSwitchers = self->GetComponentsInChildren<ConditionalMaterialSwitcher*>();
  }

  // Reset only if NE dissolve is enabled
  if (getNEConfig().enableObstacleDissolve.GetValue()) {
    for (auto* materialSwitcher : materialSwitchers) {
      materialSwitcher->_renderer->set_sharedMaterial(materialSwitcher->_material0);
    }
  }
  obstacleCache.dissolveEnabled = false;

  auto const setBounds = [&ad, &self]() constexpr {
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

  static auto Quaternion_Inverse =
      il2cpp_utils::il2cpp_type_check::FPtrWrapper<static_cast<UnityEngine::Quaternion (*)(UnityEngine::Quaternion)>(
          &UnityEngine::Quaternion::Inverse)>::get();
  static auto Quaternion_Euler =
      il2cpp_utils::il2cpp_type_check::FPtrWrapper<static_cast<UnityEngine::Quaternion (*)(float, float, float)>(
          &UnityEngine::Quaternion::Euler)>::get();

  ObstacleInitTranspileLite(self, obstacleData, ad, worldRotation, startPos, midPos, endPos, move1Duration,
                            move2Duration, singleLineWidth, height);

  setBounds();

  // Init noodlefyi
  auto localRotation = NEVector::Quaternion::identity();

  if (ad.objectData.localRotation) {
    localRotation = *ad.objectData.localRotation;
    transform->localRotation = NEVector::Quaternion(self->_worldRotation) * localRotation;
  };


  // auto const& scale = ad.objectData.scale;

  // if (scale) {
  //   if (scale->at(0)) {
  //     self->_width = *scale->at(0) * singleLineWidth;
  //   }

  //   if (scale->at(2)) {
  //     self->_length = *scale->at(2) * /*NoteLinesDistace*/ 0.6f;
  //   }
  // }


  ad.moveStartPos = self->_startPos;
  ad.moveEndPos = self->_midPos;
  ad.jumpEndPos = self->_endPos;
  ad.localRotation = localRotation;

  Vector3 noteOffset = self->_endPos;
  noteOffset.z = 0;
  ad.noteOffset = noteOffset;

  auto const& tracks = TracksAD::getAD(obstacleData->customData).tracks;
  if (!tracks.empty()) {
    auto go = self->get_gameObject();
    for (auto& track : tracks) {
      track->AddGameObject(go);
    }
  }

  self->Update();
}

static void ObstacleController_ManualUpdateTranspile(ObstacleController* self, float const elapsedTime) {
  // TRANSPILE HERE
  float num = elapsedTime;
  // TRANSPILE HERE
  NEVector::Vector3 posForTime = self->GetPosForTime(num);
  self->get_transform()->set_localPosition(NEVector::Quaternion(self->_worldRotation) * posForTime);
  auto action = self->didUpdateProgress;
  if (action) {
    action->Invoke(self, num);
  }
  if (!self->_passedThreeQuartersOfMove2Reported && num > self->move1Duration + self->move2Duration * 0.75f) {
    self->_passedThreeQuartersOfMove2Reported = true;
    auto action2 = self->passedThreeQuartersOfMove2Event;
    if (action2) {
      action2->Invoke(self);
    }
  }
  if (!self->_passedAvoidedMarkReported && num > self->_passedAvoidedMarkTime) {
    self->_passedAvoidedMarkReported = true;
    auto action3 = self->passedAvoidedMarkEvent;
    if (action3) {
      action3->Invoke(self);
    }
  }
  if (num > self->_finishMovementTime) {
    auto action4 = self->finishedMovementEvent;
    if (!action4) {
      return;
    }
    action4->Invoke(self);
  }
}

MAKE_HOOK_MATCH(ObstacleController_ManualUpdate, &ObstacleController::ManualUpdate, void, ObstacleController* self) {
  if (!Hooks::isNoodleHookEnabled()) return ObstacleController_ManualUpdate(self);

  static auto CustomKlass = classof(CustomJSONData::CustomObstacleData*);

  if (self->obstacleData->klass != CustomKlass) return ObstacleController_ManualUpdate(self);

  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(self->obstacleData);

  BeatmapObjectAssociatedData& ad = getAD(obstacleData->customData);

  if (ad.doUnhide) {
    self->Hide(false);
    ad.doUnhide = false;
  }
  if (!obstacleData->customData->value) {
    ObstacleController_ManualUpdate(self);
    return;
  }

  auto const& tracks = TracksAD::getAD(obstacleData->customData).tracks;

  if (tracks.empty() && !ad.parsed) {
    ObstacleController_ManualUpdate(self);
    return;
  }

  float const songTime = TimeSourceHelper::getSongTime(self->_audioTimeSyncController);
  float const elapsedTime = songTime - self->_startTimeOffset;
  float const obstacleOriginalTime =
      (elapsedTime - self->move1Duration) / (self->move2Duration + self->_obstacleDuration);

  float normalTime;
  auto animatedTime = NoodleExtensions::getTimeProp(tracks);

  if (animatedTime) {
    normalTime = *animatedTime;
  } else {
    normalTime = obstacleOriginalTime;
  }

  AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(ad.animationData, tracks, normalTime);

  if (offset.positionOffset.has_value()) {
    self->_startPos = ad.moveStartPos + *offset.positionOffset;
    self->_midPos = ad.moveEndPos + *offset.positionOffset;
    self->_endPos = ad.jumpEndPos + *offset.positionOffset;
  }

  Transform* transform = self->get_transform();

  if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
    NEVector::Quaternion const& worldRotation = ad.worldRotation;
    NEVector::Quaternion const& localRotation = ad.localRotation;

    NEVector::Quaternion worldRotationQuaternion = worldRotation;
    if (offset.rotationOffset.has_value()) {
      worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
      self->_worldRotation = worldRotationQuaternion;

      self->_inverseWorldRotation = NEVector::Quaternion::Inverse(worldRotationQuaternion);
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
    } else {
      self->bounds.set_size(ad.boundsSize);
    }
  }

  if (offset.scaleOffset.has_value()) {
    transform->set_localScale(*offset.scaleOffset);
  }

  auto& obstacleCache = NECaches::getObstacleCache(self);

  if (obstacleCache.cachedData != self->obstacleData) {
    obstacleCache.cachedData = self->obstacleData;
    // Obstacles are pooled. Clear obstacle when initialized if it's not colored or update to its new color (probably
    // redundantly)
    auto color = Chroma::ObstacleAPI::getObstacleControllerColorSafe(self);
    if (color) {
      obstacleCache.color = *color;
    } else {
      obstacleCache.color = std::nullopt;
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

    bool wasEnabled = obstacleCache.dissolveEnabled;
    obstacleCache.dissolveEnabled = obstacleDissolveConfig;

    if (obstacleCache.dissolveEnabled) {
      if (getNEConfig().materialBehaviour.GetValue() == (int)MaterialBehaviour::BASIC) {
        obstacleCache.dissolveEnabled |= dissolve > 0.0f;
      } else {
        bool transparent = true;

        if (getNEConfig().materialBehaviour.GetValue() == (int)MaterialBehaviour::SMART_COLOR) {
          auto const& colorIt = obstacleCache.color;

          // multiply rgb by alpha?
          if (colorIt &&
              (colorIt->a > 1.0f || NEVector::Vector3(colorIt->r, colorIt->g, colorIt->b).sqrMagnitude() > 3)) {
            transparent = false;
          }
        }

        if (transparent) obstacleCache.dissolveEnabled = dissolve > 0.0f;
      }
    }

    if (wasEnabled != obstacleCache.dissolveEnabled) {
      ArrayW<ConditionalMaterialSwitcher*> materialSwitchers = obstacleCache.conditionalMaterialSwitchers;
      for (auto* materialSwitcher : materialSwitchers) {
        materialSwitcher->_renderer->set_sharedMaterial(obstacleCache.dissolveEnabled ? materialSwitcher->_material1
                                                                                      : materialSwitcher->_material0);
      }
    }

    CutoutAnimateEffect*& cutoutAnimationEffect = obstacleCache.cutoutAnimateEffect;
    if (!cutoutAnimationEffect) {
      obstacleCache.obstacleDissolve =
          obstacleCache.obstacleDissolve ?: self->get_gameObject()->GetComponent<ObstacleDissolve*>();
      cutoutAnimationEffect = obstacleCache.obstacleDissolve->_cutoutAnimateEffect;
    }

    cutoutAnimationEffect->SetCutout(dissolve);
  }
  // do transpile only if needed

  auto animatedTimeAdjusted = obstacleTimeAdjust(self, elapsedTime, tracks);
  if (animatedTimeAdjusted != elapsedTime) {
    return ObstacleController_ManualUpdateTranspile(self, animatedTimeAdjusted);
  } else {
    return ObstacleController_ManualUpdate(self);
  }
}

MAKE_HOOK_MATCH(ObstacleController_GetPosForTime, &ObstacleController::GetPosForTime, Vector3, ObstacleController* self,
                float time) {
  if (!Hooks::isNoodleHookEnabled()) return ObstacleController_GetPosForTime(self, time);

  //    static auto CustomObstacleDataKlass = classof(CustomJSONData::CustomObstacleData *);
  //    CRASH_UNLESS(self);
  //    CRASH_UNLESS(self->obstacleData);
  //    NELogger::Logger.debug("ObstacleController::GetPosForTime %p", self->obstacleData);
  //    CRASH_UNLESS(self->obstacleData->klass);
  //    CRASH_UNLESS(CustomObstacleDataKlass);
  //
  //    if (!self || !self->obstacleData || self->obstacleData->klass != CustomObstacleDataKlass) {
  //        return ObstacleController_GetPosForTime(self, time);
  //    }
  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(self->obstacleData);

  static auto CustomKlass = classof(CustomJSONData::CustomObstacleData*);

  if (self->obstacleData->klass != CustomKlass || !obstacleData->customData->value) {
    return ObstacleController_GetPosForTime(self, time);
  }
  BeatmapObjectAssociatedData const& ad = getAD(obstacleData->customData);

  auto const& tracks = TracksAD::getAD(obstacleData->customData).tracks;

  float jumpTime = (time - self->move1Duration) / (self->move2Duration + self->_obstacleDuration);
  jumpTime = std::clamp(jumpTime, 0.0f, 1.0f);
  std::optional<NEVector::Vector3> position =
      AnimationHelper::GetDefinitePositionOffset(ad.animationData, tracks, jumpTime);

  if (!position.has_value()) return ObstacleController_GetPosForTime(self, time);

  NEVector::Vector3 const& noteOffset = ad.noteOffset;
  NEVector::Vector3 definitePosition = *position + noteOffset;
  if (time < self->move1Duration) {
    NEVector::Vector3 result =
        NEVector::Vector3::LerpUnclamped(self->_startPos, self->_midPos, time / self->move1Duration);
    return result + (definitePosition - static_cast<NEVector::Vector3>(self->_midPos));
  } else {
    return definitePosition;
  }
}

MAKE_HOOK_MATCH(ParametricBoxFakeGlowController_OnEnable, &ParametricBoxFakeGlowController::OnEnable, void,
                ParametricBoxFakeGlowController* self) {
  if (Hooks::isNoodleHookEnabled()) return;

  ParametricBoxFakeGlowController_OnEnable(self);
}

// Kaitlyn's fake glow overbounds fix
// https://github.com/ItsKaitlyn03/AnyTweaks-old/blob/a723e76506cd7cb8ab6f890b3d6a342f3618aaeb/src/hooks/ParametricBoxFakeGlowController.cpp#L23-L36
MAKE_HOOK_MATCH(ParametricBoxFakeGlowController_Refresh, &GlobalNamespace::ParametricBoxFakeGlowController::Refresh,
                void, GlobalNamespace::ParametricBoxFakeGlowController* self) {
  if (!Hooks::isNoodleHookEnabled()) return ParametricBoxFakeGlowController_Refresh(self);

  float value = std::min({ self->width, self->height, self->length });

  self->edgeSizeMultiplier = std::min(self->edgeSizeMultiplier, std::min(0.5f, value * 13.5f));

  ParametricBoxFakeGlowController_Refresh(self);
}

// #include "beatsaber-hook/shared/utils/instruction-parsing.hpp"
// MAKE_HOOK(Object_New, nullptr, Il2CppObject *, Il2CppClass *klass) {
//     if (test && klass && klass->name && klass->namespaze) {
//         NELogger::Logger.info("Allocating a {}.{}", klass->namespaze, klass->name);
//         PrintBacktrace(10);
//     }
//     return Object_New(klass);
// }

void InstallObstacleControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, ObstacleController_Init);
  INSTALL_HOOK_ORIG(NELogger::Logger, ObstacleController_ManualUpdate);
  INSTALL_HOOK(NELogger::Logger, ObstacleController_GetPosForTime);
  // Temporary fake glow disable hook
  INSTALL_HOOK(NELogger::Logger, ParametricBoxFakeGlowController_Refresh);
  INSTALL_HOOK(NELogger::Logger, ParametricBoxFakeGlowController_OnEnable);

  // Instruction on((const int32_t*) HookTracker::GetOrig(il2cpp_functions::object_new));
  // Instruction j2Ob_N_thunk(CRASH_UNLESS(on.findNthCall(1)->label));
  // logger.info("thunk addr %p %x", j2Ob_N_thunk.addr, *j2Ob_N_thunk.addr);
  // auto *j2Ob_N = CRASH_UNLESS(j2Ob_N_thunk.findNthDirectBranchWithoutLink(1));
  // INSTALL_HOOK_DIRECT(logger, Object_New, (void*) *j2Ob_N->label);
}

NEInstallHooks(InstallObstacleControllerHooks);