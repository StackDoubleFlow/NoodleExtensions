
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"

#include "GlobalNamespace/NoteCutter.hpp"
#include "GlobalNamespace/NoteCutter_CuttableBySaberSortParams.hpp"
#include "GlobalNamespace/NoteCutter_CuttableBySaberSortParamsComparer.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberTypeObject.hpp"
#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/BladeMovementDataElement.hpp"
#include "GlobalNamespace/GeometryTools.hpp"
#include "GlobalNamespace/LayerMasks.hpp"

#include "UnityEngine/Physics.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Collider.hpp"

#include "System/Array.hpp"

#include "NEHooks.h"
#include "NELogger.h"
#include "GlobalNamespace/CuttableBySaber.hpp"
#include "tracks/shared/Vector.h"

#include <optional>

using namespace GlobalNamespace;
using namespace UnityEngine;

static void ConvertToWorld(Saber* saber, NEVector::Vector3& topPos, NEVector::Vector3& bottomPos)
{
    Transform* playerTransform = saber->get_transform()->get_parent()->get_parent();

    // For some reason, SiraUtil's FPFCToggle unparents the left and right hand from VRGameCore
    // This only affects fpfc so w/e, just null check and go home
    if (playerTransform == nullptr)
    {
        return;
    }

    topPos = playerTransform->TransformPoint(topPos);
    bottomPos = playerTransform->TransformPoint(bottomPos);
}

static int getPrevAddedIndex(SaberMovementData* self) {
    int num = self->nextAddIndex - 2;
    if (num < 0)
    {
        num += self->data.size();
    }

    return num;
}

MAKE_HOOK_MATCH(NoteCutter_Cut,
                &NoteCutter::Cut,
                void,
                NoteCutter* self,
                Saber* saber) {
    if (!Hooks::isNoodleHookEnabled())
        return NoteCutter_Cut(self, saber);

//
//    NEVector::Vector3 saberBladeTopPos = saber->saberBladeTopPos;
//    NEVector::Vector3 saberBladeBottomPos = saber->saberBladeBottomPos;
//    BladeMovementDataElement prevAddedData = saber->movementData->get_prevAddedData();
//    NEVector::Vector3 topPos = prevAddedData.topPos;
//    NEVector::Vector3 bottomPos = prevAddedData.bottomPos;
//    NEVector::Vector3 p = saberBladeTopPos;
//    // TRANSPILE HERE
//    ConvertToWorld(saber, topPos, bottomPos);
//    //
//    NEVector::Vector3 vector;
//    NEVector::Vector3 halfExtents;
//    Quaternion orientation;
//    if (GeometryTools::ThreePointsToBox(p, saberBladeBottomPos, (bottomPos + topPos) * 0.5f, vector, halfExtents, orientation))
//    {
//        int num = Physics::OverlapBoxNonAlloc(vector, halfExtents, self->colliders, orientation, LayerMasks::_get_noteLayerMask());
//        if (num == 0)
//        {
//            return;
//        }
//        if (num == 1)
//        {
//            self->colliders[0]->get_gameObject()->GetComponent<CuttableBySaber*>()->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            return;
//        }
//        for (int i = 0; i < num; i++)
//        {
//            CuttableBySaber* component = self->colliders[i]->get_gameObject()->GetComponent<CuttableBySaber*>();
//            NEVector::Vector3 position = component->get_transform()->get_position();
//            NoteCutter::CuttableBySaberSortParams* cuttableBySaberSortParams = self->cuttableBySaberSortParams[i];
//            cuttableBySaberSortParams->cuttableBySaber = component;
//            cuttableBySaberSortParams->distance = (topPos - position).sqrMagnitude() - component->get_radius() * component->get_radius();
//            cuttableBySaberSortParams->pos = position;
//        }
//        if (num == 2)
//        {
//            if (self->comparer->Compare(self->cuttableBySaberSortParams[0], self->cuttableBySaberSortParams[1]) > 0)
//            {
//                self->cuttableBySaberSortParams[0]->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//                self->cuttableBySaberSortParams[1]->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//                return;
//            }
//            self->cuttableBySaberSortParams[1]->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            self->cuttableBySaberSortParams[0]->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            return;
//        }
//        else
//        {
//
//            System::Array::Sort((System::Array *) self->cuttableBySaberSortParams.operator Il2CppArray *(), 0, num, (System::Collections::IComparer*) self->comparer);
//            for (int j = 0; j < num; j++)
//            {
//                self->cuttableBySaberSortParams[j]->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            }
//        }
//    }


    auto playerTransform = saber->get_transform()->get_parent()->get_parent();

    auto prevIndex = getPrevAddedIndex(saber->movementData);
    // get as reference so no copy modification
    auto& prevData = saber->movementData->data[prevIndex];

    auto saberTopPos = prevData.topPos;
    auto saberBottomPos = prevData.bottomPos;


    prevData.topPos = playerTransform->TransformPoint(saberTopPos);
    prevData.bottomPos = playerTransform->TransformPoint(saberBottomPos);
    NoteCutter_Cut(self, saber);
    prevData.topPos = saberTopPos;
    prevData.bottomPos = saberBottomPos;
}

void InstallNoteCutterHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteCutter_Cut);
//    INSTALL_HOOK(logger, Saber_get_saberBladeBottomPos);
//    INSTALL_HOOK(logger, Saber_get_saberBladeTopPos);
}

NEInstallHooks(InstallNoteCutterHooks);