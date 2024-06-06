#pragma once

#include <vector>
#include "custom-json-data/shared/VList.h"

namespace GlobalNamespace {
class NoteController;
class NoteData;
class SliderData;
class ObstacleController;
} // namespace GlobalNamespace

namespace System::Collections::Generic {
template <typename T> class List_1;
}

namespace FakeNoteHelper {

bool GetFakeNote(GlobalNamespace::NoteData* noteData);
bool GetCuttable(GlobalNamespace::NoteData* noteData);
bool GetAttractableArc(GlobalNamespace::SliderData* arcData);

System::Collections::Generic::List_1<GlobalNamespace::ObstacleController*>*
ObstacleFakeCheck(VList<GlobalNamespace::ObstacleController*> intersectingObstacles);
} // namespace FakeNoteHelper