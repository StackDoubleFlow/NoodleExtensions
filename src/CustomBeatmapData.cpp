#include "CustomBeatmapData.h"

DEFINE_CLASS(Il2CppNamespace::CustomBeatmapData);

void Il2CppNamespace::CustomBeatmapData::ctor() {

}

void Il2CppNamespace::CustomBeatmapData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(Il2CppNamespace::CustomBeatmapEventData);

void Il2CppNamespace::CustomBeatmapEventData::ctor() {

}

void Il2CppNamespace::CustomBeatmapEventData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(Il2CppNamespace::CustomObstacleData);

void Il2CppNamespace::CustomObstacleData::ctor() {

}

void Il2CppNamespace::CustomObstacleData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(Il2CppNamespace::CustomNoteData);

void Il2CppNamespace::CustomNoteData::ctor(float time, int lineIndex, NoteLineLayer noteLineLayer, NoteLineLayer startNoteLineLayer, ColorType colorType, NoteCutDirection cutDirection, float timeToNextColorNote, float timeToPrevColorNote, int flipLineIndex, float flipYSide, float duration) {
    this->colorType = colorType;
    this->cutDirection = cutDirection;
    this->timeToNextColorNote = timeToNextColorNote;
    this->timeToPrevColorNote = timeToPrevColorNote;
    this->noteLineLayer = noteLineLayer;
    this->startNoteLineLayer = startNoteLineLayer;
    this->flipLineIndex = flipLineIndex;
    this->flipYSide = flipYSide;
    this->duration = duration;
}

Il2CppNamespace::CustomNoteData* Il2CppNamespace::CustomNoteData::CreateBasicNoteData(float time, int lineIndex, NoteLineLayer noteLineLayer, ColorType colorType, NoteCutDirection cutDirection) {
    return CRASH_UNLESS(il2cpp_utils::New<Il2CppNamespace::CustomNoteData*>(time, lineIndex, noteLineLayer, noteLineLayer, colorType, cutDirection, 0, 0, lineIndex, 0, 0));
}

Il2CppNamespace::CustomNoteData* Il2CppNamespace::CustomNoteData::CreateBombNoteData(float time, int lineIndex, NoteLineLayer noteLineLayer) {
    return CRASH_UNLESS(il2cpp_utils::New<Il2CppNamespace::CustomNoteData*>(time, lineIndex, noteLineLayer, noteLineLayer, ColorType::None, NoteCutDirection::None, 0, 0, lineIndex, 0, 0));
}

void Il2CppNamespace::CustomNoteData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(Il2CppNamespace::CustomEventData);

void Il2CppNamespace::CustomEventData::ctor(Il2CppString* type, float time) {
    this->type = type;
    this->time = time;
}

void Il2CppNamespace::CustomEventData::Finalize() {
    delete this->data;
}