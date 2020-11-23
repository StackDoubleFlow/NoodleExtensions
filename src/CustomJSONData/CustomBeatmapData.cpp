#include "CustomJSONData/CustomBeatmapData.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/stringbuffer.h"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/prettywriter.h"

using namespace GlobalNamespace;

DEFINE_CLASS(CustomJSONData::CustomBeatmapData);

void CustomJSONData::CustomBeatmapData::ctor() {

}

void CustomJSONData::CustomBeatmapData::Finalize() {
    delete this->customData;
}

DEFINE_CLASS(CustomJSONData::CustomBeatmapEventData);

void CustomJSONData::CustomBeatmapEventData::ctor(float time, BeatmapEventType type, int value) {
    this->time = time;
    this->type = type;
    this->value = value;
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
    // if (this->customData) {
    //     NELogger::GetLogger().info("CustomJSONData::CustomNoteData::GetCopy %p %p %p", copy, copy->customData, this->customData);
    // }
    return copy;
}

void CustomJSONData::CustomObstacleData::Finalize() {
    // delete this->customData;
    NELogger::GetLogger().debug("CustomObstacleData::Finalize");
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
    this->customData = nullptr;
}

BeatmapObjectData *CustomJSONData::CustomNoteData::GetCopy() {
    auto copy = CRASH_UNLESS(il2cpp_utils::New<CustomJSONData::CustomNoteData*>(this->time, this->lineIndex, this->noteLineLayer, this->startNoteLineLayer, 
        this->colorType, this->cutDirection, this->timeToNextColorNote, this->timeToPrevColorNote, this->flipLineIndex, this->flipYSide, this->duration));
    copy->customData = this->customData;
    if (this->customData) {
        NELogger::GetLogger().info("CustomJSONData::CustomNoteData::GetCopy %p %p %p", copy, copy->customData, this->customData);
    }
    return copy;
}

void CustomJSONData::CustomNoteData::Finalize() {
    // delete this->customData;
    NELogger::GetLogger().debug("CustomObstacleData::Finalize");
}

DEFINE_CLASS(CustomJSONData::CustomEventData);

void CustomJSONData::CustomEventData::ctor(Il2CppString* type, float time) {
    this->type = type;
    this->time = time;
}

void CustomJSONData::CustomEventData::Finalize() {
    delete this->data;
}