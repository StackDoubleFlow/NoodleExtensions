#pragma once

namespace GlobalNamespace {
    class NoteController;
    class NoteData;
}

namespace FakeNoteHelper {

bool GetFakeNote(GlobalNamespace::NoteController *noteController);
bool GetCuttable(GlobalNamespace::NoteData *noteData);

}