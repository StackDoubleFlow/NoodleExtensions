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

void CustomJSONData::CustomObstacleData::ctor(float time, int lineIndex, ObstacleType obstacleType, float duration, int width) {
    this->time = time;
    this->lineIndex = lineIndex;
    this->obstacleType = obstacleType;
    this->duration = duration;
    this->width = width;
}

BeatmapObjectData *CustomJSONData::CustomObstacleData::GetCopy() {
    auto copy = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomObstacleData*>(this->time, this->lineIndex, this->obstacleType, this->duration, this->width));
    copy->customData = this->customData;
    NELogger::GetLogger().info("CustomJSONData::CustomObstacleData::GetCopy %p %p", copy->customData, this->customData);
    return copy;
}

void CustomJSONData::CustomObstacleData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(CustomJSONData::CustomNoteData);

void CustomJSONData::CustomNoteData::ctor(float time, int lineIndex, NoteLineLayer noteLineLayer, NoteLineLayer startNoteLineLayer, ColorType colorType, NoteCutDirection cutDirection, float timeToNextColorNote, float timeToPrevColorNote, int flipLineIndex, float flipYSide, float duration) {
    this->time = time;
    this->lineIndex = lineIndex;
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