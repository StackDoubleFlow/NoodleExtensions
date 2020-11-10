#include "CustomJSONData/CustomBeatmapData.h"

DEFINE_CLASS(CustomJSONData::CustomBeatmapData);

void CustomJSONData::CustomBeatmapData::ctor() {

}

void CustomJSONData::CustomBeatmapData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(CustomJSONData::CustomBeatmapEventData);

void CustomJSONData::CustomBeatmapEventData::ctor() {

}

void CustomJSONData::CustomBeatmapEventData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(CustomJSONData::CustomObstacleData);

void CustomJSONData::CustomObstacleData::ctor() {

}

void CustomJSONData::CustomObstacleData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(CustomJSONData::CustomNoteData);

void CustomJSONData::CustomNoteData::ctor(float time, int lineIndex, NoteLineLayer noteLineLayer, NoteLineLayer startNoteLineLayer, ColorType colorType, NoteCutDirection cutDirection, float timeToNextColorNote, float timeToPrevColorNote, int flipLineIndex, float flipYSide, float duration) {
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

// void CustomJSONData::CustomNoteData::Finalize() {
//     delete this->customData;
// }

DEFINE_CLASS(CustomJSONData::CustomEventData);

void CustomJSONData::CustomEventData::ctor(Il2CppString* type, float time) {
    this->type = type;
    this->time = time;
}

void CustomJSONData::CustomEventData::Finalize() {
    delete this->data;
}