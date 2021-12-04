
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"

#include "GlobalNamespace/NoteCutter.hpp"
#include "GlobalNamespace/NoteCutter_CuttableBySaberSortParams.hpp"
#include "GlobalNamespace/NoteCutter_CuttableBySaberSortParamsComparer.hpp"
#include "GlobalNamespace/Saber.hpp"
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

UnityEngine::Transform* playerTransform;
std::array<bool, 2> firstCall;

MAKE_HOOK_MATCH(Saber_get_saberBladeTopPos,
                &Saber::get_saberBladeTopPos,
                UnityEngine::Vector3,
                Saber* self) {
    if (!playerTransform || !firstCall[0])
        return Saber_get_saberBladeTopPos(self);

    auto result = Saber_get_saberBladeTopPos(self);

    firstCall[0] = false;

    return playerTransform->TransformPoint(result);
}

MAKE_HOOK_MATCH(Saber_get_saberBladeBottomPos,
                &Saber::get_saberBladeBottomPos,
                UnityEngine::Vector3,
                Saber* self) {
    if (!playerTransform || !firstCall[1])
        return Saber_get_saberBladeBottomPos(self);

    auto result = Saber_get_saberBladeBottomPos(self);

    firstCall[1] = false;

    return playerTransform->TransformPoint(result);
}

MAKE_HOOK_MATCH(NoteCutter_Cut,
                &NoteCutter::Cut,
                void,
                NoteCutter* self,
                Saber* saber) {
    firstCall[0] = true;
    firstCall[1] = true;

    playerTransform = saber->get_transform()->get_parent()->get_parent();

    NoteCutter_Cut(self, saber);
    playerTransform = nullptr;

//    auto prevTopPos = saber->saberBladeTopPos;
//    auto prevBottomPos = saber->saberBladeBottomPos;
//
//    playerTransform = saber->get_transform()->get_parent()->get_parent();
//
//    saber->saberBladeTopPos = playerTransform->TransformPoint(prevTopPos);
//    saber->saberBladeBottomPos = playerTransform->TransformPoint(prevBottomPos);
//
//    NoteCutter_Cut(self, saber);
//
//    saber->saberBladeTopPos = prevTopPos;
//    saber->saberBladeBottomPos = prevBottomPos;




//    Vector3 saberBladeTopPos = saber->saberBladeTopPos;
//    Vector3 saberBladeBottomPos = saber->saberBladeBottomPos;
//
//    auto playerTransform = saber->get_transform()->get_parent()->get_parent();
//
//    saberBladeTopPos = playerTransform->TransformPoint(saberBladeTopPos);
//    saberBladeBottomPos = playerTransform->TransformPoint(saberBladeBottomPos);
//
//    BladeMovementDataElement prevAddedData = saber->movementData->get_prevAddedData();
//    NEVector::Vector3 topPos = prevAddedData.topPos;
//    NEVector::Vector3 bottomPos = prevAddedData.bottomPos;
//    Vector3 vector;
//    Vector3 halfExtents;
//    Quaternion orientation;
//    if (GeometryTools::ThreePointsToBox(saberBladeTopPos, saberBladeBottomPos, (bottomPos + topPos) * 0.5f, vector, halfExtents, orientation))
//    {
//        int num = Physics::OverlapBoxNonAlloc(vector, halfExtents, self->colliders, orientation, LayerMasks::_get_noteLayerMask());
//        if (num == 0)
//        {
//            return;
//        }
//        if (num == 1)
//        {
//            self->colliders->get(0)->get_gameObject()->GetComponent<CuttableBySaber*>()->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            return;
//        }
//        for (int i = 0; i < num; i++)
//        {
//            CuttableBySaber* component = self->colliders->get(i)->get_gameObject()->GetComponent<CuttableBySaber*>();
//            Vector3 position = component->get_transform()->get_position();
//            NoteCutter::CuttableBySaberSortParams* cuttableBySaberSortParams = self->cuttableBySaberSortParams->get(i);
//            cuttableBySaberSortParams->cuttableBySaber = component;
//            cuttableBySaberSortParams->distance = (topPos - position).sqrMagnitude() - component->get_radius() * component->get_radius();
//            cuttableBySaberSortParams->pos = position;
//        }
//        if (num == 2)
//        {
//            if (self->comparer->Compare(self->cuttableBySaberSortParams->get(0), self->cuttableBySaberSortParams->get(1)) > 0)
//            {
//                self->cuttableBySaberSortParams->get(0)->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//                self->cuttableBySaberSortParams->get(1)->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//                return;
//            }
//            self->cuttableBySaberSortParams->get(1)->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            self->cuttableBySaberSortParams->get(0)->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            return;
//        }
//        else
//        {
//            System::Array::Sort(self->cuttableBySaberSortParams, 0, num,
//                                reinterpret_cast<System::Collections::IComparer *>(self->comparer));
//            for (int j = 0; j < num; j++)
//            {
//                self->cuttableBySaberSortParams->get(j)->cuttableBySaber->Cut(saber, vector, orientation, saberBladeTopPos - topPos);
//            }
//        }
//    }
}

void InstallNoteCutterHooks(Logger &logger) {
    INSTALL_HOOK(logger, NoteCutter_Cut);
    INSTALL_HOOK(logger, Saber_get_saberBladeBottomPos);
    INSTALL_HOOK(logger, Saber_get_saberBladeTopPos);
}

NEInstallHooks(InstallNoteCutterHooks);