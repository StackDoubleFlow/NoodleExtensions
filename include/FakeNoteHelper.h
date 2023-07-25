#pragma once

#include <vector>

namespace GlobalNamespace {
class NoteController;
class NoteData;
class ObstacleController;
} // namespace GlobalNamespace

template <typename T> class VList;

namespace System::Collections::Generic {
template <typename T> class List_1;
}

namespace FakeNoteHelper {

bool GetFakeNote(GlobalNamespace::NoteData* noteData);
bool GetCuttable(GlobalNamespace::NoteData* noteData);

System::Collections::Generic::List_1<GlobalNamespace::ObstacleController*>*
ObstacleFakeCheck(VList<GlobalNamespace::ObstacleController*> intersectingObstacles);
} // namespace FakeNoteHelper